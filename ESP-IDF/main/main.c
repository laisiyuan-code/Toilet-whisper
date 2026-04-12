#include "app_main.h"

#include <stdio.h>
#include <string.h>

#include "esp_check.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spiffs.h"
#include "esp_sntp.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include "air_quality.h"
#include "ble_presence.h"
#include "config.h"
#include "data_logger.h"
#include "decision_engine.h"
#include "fan_control.h"
#include "i2c_manager.h"
#include "rainmaker_app.h"
#include "veml7700.h"

static const char *TAG = "app_main";

static app_state_t g_app;
static veml7700_t g_veml7700;
static air_quality_dev_t g_air;

static void wifi_event_handler(void *arg, esp_event_base_t base, int32_t id, void *event_data)
{
    app_state_t *state = (app_state_t *)arg;
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        app_state_lock(state);
        state->connectivity.wifi_connected = false;
        xEventGroupSetBits(state->events, APP_EVENT_WIFI_CHANGED);
        app_state_unlock(state);
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        app_state_lock(state);
        state->connectivity.wifi_connected = true;
        xEventGroupSetBits(state->events, APP_EVENT_WIFI_CHANGED);
        app_state_unlock(state);
    }
}

static void init_default_config(app_config_t *cfg)
{
    *cfg = (app_config_t){
        .ble_enable = DEFAULT_BLE_ENABLE,
        .light_enable = DEFAULT_LIGHT_ENABLE,
        .humidity_enable = DEFAULT_HUMIDITY_ENABLE,
        .air_enable = DEFAULT_AIR_ENABLE,
        .ble_rssi_threshold_dbm = DEFAULT_BLE_RSSI_THRESHOLD_DBM,
        .light_delta_threshold_lux = DEFAULT_LIGHT_DELTA_THRESHOLD_LUX,
        .humidity_delta_threshold_pct = DEFAULT_HUMIDITY_DELTA_THRESHOLD,
        .humidity_absolute_threshold_pct = DEFAULT_HUMIDITY_ABS_THRESHOLD,
        .tvoc_threshold_ppb = DEFAULT_TVOC_THRESHOLD_PPB,
        .eco2_threshold_ppm = DEFAULT_ECO2_THRESHOLD_PPM,
        .aqi_threshold = DEFAULT_AQI_THRESHOLD,
        .sensor_period_ms = DEFAULT_SENSOR_PERIOD_MS,
        .decision_period_ms = DEFAULT_DECISION_PERIOD_MS,
        .status_period_ms = DEFAULT_STATUS_PERIOD_MS,
        .log_period_ms = DEFAULT_LOG_PERIOD_MS,
        .telemetry_period_ms = DEFAULT_RAINMAKER_TELEMETRY_MS,
        .fan_hold_time_s = DEFAULT_FAN_HOLD_TIME_S,
        .light_only_run_time_s = DEFAULT_LIGHT_ONLY_RUN_TIME_S,
        .presence_hold_time_s = DEFAULT_PRESENCE_HOLD_TIME_S,
        .start_hold_time_s = DEFAULT_START_HOLD_TIME_S,
        .min_duty_percent = DEFAULT_MIN_DUTY_PERCENT,
        .low_duty_percent = DEFAULT_LOW_DUTY_PERCENT,
        .medium_duty_percent = DEFAULT_MED_DUTY_PERCENT,
        .high_duty_percent = DEFAULT_HIGH_DUTY_PERCENT,
        .max_duty_percent = DEFAULT_MAX_DUTY_PERCENT,
        .kickstart_enable = DEFAULT_KICKSTART_ENABLE,
        .kickstart_duration_ms = DEFAULT_KICKSTART_DURATION_MS,
        .ble_scan_interval_ms = DEFAULT_BLE_SCAN_INTERVAL_MS,
        .ble_scan_window_ms = DEFAULT_BLE_SCAN_WINDOW_MS,
        .ble_scan_burst_ms = DEFAULT_BLE_SCAN_BURST_MS,
        .ble_scan_idle_ms = DEFAULT_BLE_SCAN_IDLE_MS,
        .ble_presence_debounce_count = DEFAULT_BLE_PRESENCE_DEBOUNCE_COUNT,
    };
}

void app_state_lock(app_state_t *state)
{
    xSemaphoreTake(state->mutex, portMAX_DELAY);
}

void app_state_unlock(app_state_t *state)
{
    xSemaphoreGive(state->mutex);
}

int64_t app_now_us(void)
{
    return esp_timer_get_time();
}

const char *fan_mode_to_str(fan_mode_t mode)
{
    switch (mode) {
    case FAN_MODE_OFF:
        return "OFF";
    case FAN_MODE_AUTO:
        return "AUTO";
    case FAN_MODE_LOW:
        return "LOW";
    case FAN_MODE_MEDIUM:
        return "MED";
    case FAN_MODE_HIGH:
        return "HIGH";
    default:
        return "UNKNOWN";
    }
}

const char *fan_state_to_str(fan_state_t state)
{
    switch (state) {
    case FAN_STATE_STOPPED:
        return "STOP";
    case FAN_STATE_STARTING:
        return "START";
    case FAN_STATE_RUNNING:
        return "RUN";
    default:
        return "UNKNOWN";
    }
}

fan_mode_t fan_mode_from_str(const char *value)
{
    if (!value) {
        return FAN_MODE_AUTO;
    }
    if (strcasecmp(value, "OFF") == 0) {
        return FAN_MODE_OFF;
    }
    if (strcasecmp(value, "LOW") == 0) {
        return FAN_MODE_LOW;
    }
    if (strcasecmp(value, "MED") == 0 || strcasecmp(value, "MEDIUM") == 0) {
        return FAN_MODE_MEDIUM;
    }
    if (strcasecmp(value, "HIGH") == 0) {
        return FAN_MODE_HIGH;
    }
    return FAN_MODE_AUTO;
}

static esp_err_t nvs_save_blob(const char *ns, const char *key, const void *data, size_t len)
{
    nvs_handle_t handle;
    ESP_RETURN_ON_ERROR(nvs_open(ns, NVS_READWRITE, &handle), TAG, "nvs open failed");
    esp_err_t err = nvs_set_blob(handle, key, data, len);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}

static esp_err_t nvs_load_blob(const char *ns, const char *key, void *data, size_t len)
{
    nvs_handle_t handle;
    ESP_RETURN_ON_ERROR(nvs_open(ns, NVS_READONLY, &handle), TAG, "nvs open failed");
    size_t required = len;
    esp_err_t err = nvs_get_blob(handle, key, data, &required);
    nvs_close(handle);
    return err;
}

esp_err_t app_load_config(app_state_t *state)
{
    app_config_t stored = {0};
    esp_err_t err = nvs_load_blob(NVS_NAMESPACE_CONFIG, "cfg", &stored, sizeof(stored));
    if (err == ESP_OK) {
        state->config = stored;
        return ESP_OK;
    }
    return err;
}

esp_err_t app_save_config(app_state_t *state)
{
    return nvs_save_blob(NVS_NAMESPACE_CONFIG, "cfg", &state->config, sizeof(state->config));
}

esp_err_t app_load_daily_summary(app_state_t *state)
{
    daily_summary_t stored = {0};
    esp_err_t err = nvs_load_blob(NVS_NAMESPACE_SUMMARY, "sum", &stored, sizeof(stored));
    if (err == ESP_OK) {
        state->summary = stored;
    }
    return err;
}

esp_err_t app_save_daily_summary(app_state_t *state)
{
    state->summary.fan_runtime_s = state->fan.runtime_seconds_today;
    state->summary.fan_activation_count = state->fan.activation_count_today;
    state->summary.ble_detections = state->ble.detections_today;
    return nvs_save_blob(NVS_NAMESPACE_SUMMARY, "sum", &state->summary, sizeof(state->summary));
}

void app_reset_daily_summary_if_needed(app_state_t *state, struct tm *timeinfo)
{
    if (!timeinfo) {
        return;
    }
    if (state->summary.day_of_year == 0) {
        state->summary.day_of_year = timeinfo->tm_yday;
        return;
    }
    if (state->summary.day_of_year != timeinfo->tm_yday) {
        memset(&state->summary, 0, sizeof(state->summary));
        state->summary.day_of_year = timeinfo->tm_yday;
        state->fan.runtime_seconds_today = 0;
        state->fan.activation_count_today = 0;
        state->ble.detections_today = 0;
    }
}

void app_update_daily_summary_from_sensor(app_state_t *state)
{
    if (state->sensors.humidity_pct > state->summary.max_humidity_pct) {
        state->summary.max_humidity_pct = state->sensors.humidity_pct;
    }
    state->summary.humidity_accumulator += state->sensors.humidity_pct;
    state->summary.humidity_samples++;
    if (state->sensors.tvoc_ppb > state->summary.peak_tvoc_ppb) {
        state->summary.peak_tvoc_ppb = state->sensors.tvoc_ppb;
    }
    if (state->sensors.eco2_ppm > state->summary.peak_eco2_ppm) {
        state->summary.peak_eco2_ppm = state->sensors.eco2_ppm;
    }
}

static esp_err_t init_spiffs(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = LOGGER_MOUNT_POINT,
        .partition_label = NULL,
        .max_files = 6,
        .format_if_mount_failed = true,
    };
    return esp_vfs_spiffs_register(&conf);
}

static void init_time_sync(void)
{
    esp_sntp_config_t cfg = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&cfg);
}

static esp_err_t init_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return err;
}

static void sensor_task(void *arg)
{
    app_state_t *state = (app_state_t *)arg;
    float lux = 0.0f;
    const float alpha = 0.15f;

    while (true) {
        esp_err_t lux_err = veml7700_read_lux(&g_veml7700, &lux);
        esp_err_t air_err = air_quality_read(&g_air);

        app_state_lock(state);
        if (lux_err == ESP_OK) {
            state->sensors.veml7700_ok = true;
            state->sensors.lux = lux;
            if (state->sensors.lux_baseline <= 0.1f) {
                state->sensors.lux_baseline = lux;
            } else {
                state->sensors.lux_baseline = ((1.0f - alpha) * state->sensors.lux_baseline) + (alpha * lux);
            }
        } else {
            state->sensors.veml7700_ok = false;
        }

        if (air_err == ESP_OK || g_air.aht21_ok || g_air.ens160_ok) {
            state->sensors.aht21_ok = g_air.aht21_ok;
            state->sensors.ens160_ok = g_air.ens160_ok;
            state->sensors.temp_c = g_air.temp_c;
            state->sensors.humidity_pct = g_air.humidity_pct;
            if (state->sensors.humidity_baseline <= 0.1f) {
                state->sensors.humidity_baseline = g_air.humidity_pct;
            } else {
                state->sensors.humidity_baseline = ((1.0f - alpha) * state->sensors.humidity_baseline) + (alpha * g_air.humidity_pct);
            }
            state->sensors.tvoc_ppb = g_air.tvoc_ppb;
            state->sensors.eco2_ppm = g_air.eco2_ppm;
            state->sensors.aqi = g_air.aqi;
        }

        state->sensors.last_read_ok = (lux_err == ESP_OK) || (air_err == ESP_OK);
        state->sensors.last_update_us = app_now_us();
        app_update_daily_summary_from_sensor(state);
        xEventGroupSetBits(state->events, APP_EVENT_SENSORS_UPDATED);
        app_state_unlock(state);

        if (lux_err != ESP_OK && air_err != ESP_OK) {
            data_logger_log_event(state, "sensor_fault", "All sensor reads failed in this cycle");
        }
        vTaskDelay(pdMS_TO_TICKS(state->config.sensor_period_ms));
    }
}

static void decision_task(void *arg)
{
    app_state_t *state = (app_state_t *)arg;
    while (true) {
        decision_engine_step(state);
        vTaskDelay(pdMS_TO_TICKS(state->config.decision_period_ms));
    }
}

static void logger_task(void *arg)
{
    app_state_t *state = (app_state_t *)arg;
    while (true) {
        data_logger_log_snapshot(state);
        app_state_lock(state);
        time_t now = time(NULL);
        struct tm timeinfo = {0};
        localtime_r(&now, &timeinfo);
        app_reset_daily_summary_if_needed(state, &timeinfo);
        app_save_daily_summary(state);
        app_state_unlock(state);
        vTaskDelay(pdMS_TO_TICKS(state->config.log_period_ms));
    }
}

static void rainmaker_task(void *arg)
{
    app_state_t *state = (app_state_t *)arg;
    while (true) {
        rainmaker_app_report_state(state);
        vTaskDelay(pdMS_TO_TICKS(state->config.telemetry_period_ms));
    }
}

static void status_task(void *arg)
{
    app_state_t *state = (app_state_t *)arg;
    while (true) {
        char timebuf[32] = "boot";
        app_state_lock(state);
        time_t now = time(NULL);
        struct tm timeinfo = {0};
        if (localtime_r(&now, &timeinfo)) {
            strftime(timebuf, sizeof(timebuf), "%H:%M:%S", &timeinfo);
        }
        ESP_LOGI(TAG,
                 "[%s] mode=%s lux=%.1f temp=%.1f rh=%.1f tvoc=%u eco2=%u aqi=%u ble=%d rssi=%d trig={%s%s%s%s} fan=%s duty=%u wifi=%d rmk=%d",
                 timebuf,
                 fan_mode_to_str(state->fan.requested_mode),
                 state->sensors.lux,
                 state->sensors.temp_c,
                 state->sensors.humidity_pct,
                 state->sensors.tvoc_ppb,
                 state->sensors.eco2_ppm,
                 state->sensors.aqi,
                 state->ble.stable_present,
                 state->ble.strongest_rssi,
                 state->triggers.ble_trigger ? "ble," : "",
                 state->triggers.light_trigger ? "light," : "",
                 state->triggers.humidity_trigger ? "humidity," : "",
                 state->triggers.air_trigger ? "air" : "",
                 fan_state_to_str(state->fan.state),
                 state->fan.duty_percent,
                 state->connectivity.wifi_connected,
                 state->connectivity.rainmaker_connected);
        app_state_unlock(state);
        vTaskDelay(pdMS_TO_TICKS(state->config.status_period_ms));
    }
}

void app_main(void)
{
    memset(&g_app, 0, sizeof(g_app));
    g_app.mutex = xSemaphoreCreateMutex();
    g_app.events = xEventGroupCreate();
    g_app.boot_time = time(NULL);
    init_default_config(&g_app.config);
    g_app.fan.requested_mode = FAN_MODE_AUTO;
    g_app.fan.applied_mode = FAN_MODE_OFF;
    g_app.fan.state = FAN_STATE_STOPPED;

    ESP_ERROR_CHECK(init_nvs());
    esp_err_t cfg_err = app_load_config(&g_app);
    if (cfg_err != ESP_OK) {
        ESP_LOGW(TAG, "Using default config: %s", esp_err_to_name(cfg_err));
    }
    app_load_daily_summary(&g_app);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, &g_app));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, &g_app));

    ESP_ERROR_CHECK(init_spiffs());
    init_time_sync();
    ESP_ERROR_CHECK(i2c_manager_init());

    if (veml7700_init(&g_veml7700) != ESP_OK) {
        ESP_LOGW(TAG, "VEML7700 not available");
    }
    if (air_quality_init(&g_air) != ESP_OK) {
        ESP_LOGW(TAG, "Air-quality stack not fully available");
    }

    ESP_ERROR_CHECK(data_logger_init(&g_app));
    ESP_ERROR_CHECK(fan_control_init(&g_app));
    ble_presence_init(&g_app);
    ble_presence_start(&g_app);
    rainmaker_app_init(&g_app);

    xTaskCreate(sensor_task, "sensor_task", 4096, &g_app, 5, NULL);
    xTaskCreate(decision_task, "decision_task", 4096, &g_app, 6, NULL);
    xTaskCreate(logger_task, "logger_task", 4096, &g_app, 3, NULL);
    xTaskCreate(status_task, "status_task", 4096, &g_app, 2, NULL);
    xTaskCreate(rainmaker_task, "rainmaker_task", 6144, &g_app, 3, NULL);

    ESP_LOGI(TAG, "%s started", APP_NAME);
}
