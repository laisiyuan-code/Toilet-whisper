#include "rainmaker_app.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"

#include "data_logger.h"

#if __has_include("esp_rmaker_core.h")
#define HAS_RAINMAKER 1
#include "esp_rmaker_common_events.h"
#include "esp_rmaker_core.h"
#include "esp_rmaker_ota.h"
#include "esp_rmaker_schedule.h"
#include "esp_rmaker_standard_devices.h"
#include "esp_rmaker_standard_params.h"
#include "esp_rmaker_time.h"
#else
#define HAS_RAINMAKER 0
#endif

static const char *TAG = "rainmaker";
static app_state_t *s_state;

#define RMK_TYPE_NUMBER "esp.param.number"
#define RMK_TYPE_STRING "esp.param.string"

#if HAS_RAINMAKER
static esp_rmaker_node_t *s_node;
static esp_rmaker_device_t *s_fan_device;
static esp_rmaker_param_t *s_mode_param;
static esp_rmaker_param_t *s_lux_param;
static esp_rmaker_param_t *s_temp_param;
static esp_rmaker_param_t *s_humidity_param;
static esp_rmaker_param_t *s_tvoc_param;
static esp_rmaker_param_t *s_eco2_param;
static esp_rmaker_param_t *s_aqi_param;
static esp_rmaker_param_t *s_ble_present_param;
static esp_rmaker_param_t *s_rssi_param;
static esp_rmaker_param_t *s_trigger_param;
static esp_rmaker_param_t *s_fan_state_param;
static esp_rmaker_param_t *s_fan_duty_param;
static esp_rmaker_param_t *s_wifi_param;
static esp_rmaker_param_t *s_rmk_param;
static esp_rmaker_param_t *s_runtime_param;

static esp_err_t write_cb(const esp_rmaker_device_t *device,
                          const esp_rmaker_param_t *param,
                          const esp_rmaker_param_val_t val,
                          void *priv_data,
                          esp_rmaker_write_ctx_t *ctx)
{
    (void)device;
    (void)priv_data;
    (void)ctx;

    app_state_lock(s_state);
    const char *name = esp_rmaker_param_get_name(param);
    if (strcmp(name, ESP_RMAKER_DEF_POWER_NAME) == 0) {
        s_state->fan.requested_mode = val.val.b ? FAN_MODE_AUTO : FAN_MODE_OFF;
    } else if (strcmp(name, "Mode") == 0) {
        s_state->fan.requested_mode = fan_mode_from_str(val.val.s);
    } else if (strcmp(name, "BLE Enable") == 0) {
        s_state->config.ble_enable = val.val.b;
    } else if (strcmp(name, "Light Enable") == 0) {
        s_state->config.light_enable = val.val.b;
    } else if (strcmp(name, "Humidity Enable") == 0) {
        s_state->config.humidity_enable = val.val.b;
    } else if (strcmp(name, "Air Enable") == 0) {
        s_state->config.air_enable = val.val.b;
    } else if (strcmp(name, "BLE RSSI Threshold") == 0) {
        s_state->config.ble_rssi_threshold_dbm = val.val.i;
    } else if (strcmp(name, "Light Delta Threshold") == 0) {
        s_state->config.light_delta_threshold_lux = val.val.f;
    } else if (strcmp(name, "Humidity Delta Threshold") == 0) {
        s_state->config.humidity_delta_threshold_pct = val.val.f;
    } else if (strcmp(name, "Humidity Absolute Threshold") == 0) {
        s_state->config.humidity_absolute_threshold_pct = val.val.f;
    } else if (strcmp(name, "Fan Hold Time") == 0) {
        s_state->config.fan_hold_time_s = val.val.i;
    } else if (strcmp(name, "Min Duty") == 0) {
        s_state->config.min_duty_percent = val.val.i;
    } else if (strcmp(name, "Max Duty") == 0) {
        s_state->config.max_duty_percent = val.val.i;
    } else if (strcmp(name, "Kick Start Enable") == 0) {
        s_state->config.kickstart_enable = val.val.b;
    } else if (strcmp(name, "Kick Start Duration") == 0) {
        s_state->config.kickstart_duration_ms = val.val.i;
    }
    xEventGroupSetBits(s_state->events, APP_EVENT_CONFIG_CHANGED);
    app_save_config(s_state);
    app_state_unlock(s_state);

    data_logger_log_event(s_state, "app_command", name);
    esp_rmaker_param_update_and_report((esp_rmaker_param_t *)param, val);
    return ESP_OK;
}

static esp_rmaker_param_t *telemetry_param(const char *name, const char *type, esp_rmaker_param_val_t value)
{
    return esp_rmaker_param_create(name, type, value, PROP_FLAG_READ);
}
#endif

bool rainmaker_app_is_enabled(void)
{
    return HAS_RAINMAKER;
}

esp_err_t rainmaker_app_init(app_state_t *state)
{
    s_state = state;
#if HAS_RAINMAKER
    esp_rmaker_config_t cfg = {
        .enable_time_sync = false,
    };
    s_node = esp_rmaker_node_init(&cfg, "Intelligent Ventilation", "ESP32-C3 Bathroom Fan");
    if (!s_node) {
        return ESP_FAIL;
    }

    s_fan_device = esp_rmaker_fan_device_create("Bathroom Fan", NULL, false);
    esp_rmaker_device_add_cb(s_fan_device, write_cb, NULL);

    esp_rmaker_device_add_param(
        s_fan_device,
        esp_rmaker_param_create("BLE Enable", ESP_RMAKER_PARAM_POWER_MODE, esp_rmaker_bool(state->config.ble_enable),
                                PROP_FLAG_READ | PROP_FLAG_WRITE));
    esp_rmaker_device_add_param(
        s_fan_device,
        esp_rmaker_param_create("Light Enable", ESP_RMAKER_PARAM_POWER_MODE, esp_rmaker_bool(state->config.light_enable),
                                PROP_FLAG_READ | PROP_FLAG_WRITE));
    esp_rmaker_device_add_param(
        s_fan_device,
        esp_rmaker_param_create("Humidity Enable", ESP_RMAKER_PARAM_POWER_MODE, esp_rmaker_bool(state->config.humidity_enable),
                                PROP_FLAG_READ | PROP_FLAG_WRITE));
    esp_rmaker_device_add_param(
        s_fan_device,
        esp_rmaker_param_create("Air Enable", ESP_RMAKER_PARAM_POWER_MODE, esp_rmaker_bool(state->config.air_enable),
                                PROP_FLAG_READ | PROP_FLAG_WRITE));
    esp_rmaker_device_add_param(
        s_fan_device,
        esp_rmaker_param_create("BLE RSSI Threshold", ESP_RMAKER_PARAM_RANGE, esp_rmaker_int(state->config.ble_rssi_threshold_dbm),
                                PROP_FLAG_READ | PROP_FLAG_WRITE));
    esp_rmaker_device_add_param(
        s_fan_device,
        esp_rmaker_param_create("Light Delta Threshold", ESP_RMAKER_PARAM_RANGE,
                                esp_rmaker_float(state->config.light_delta_threshold_lux), PROP_FLAG_READ | PROP_FLAG_WRITE));
    esp_rmaker_device_add_param(
        s_fan_device,
        esp_rmaker_param_create("Humidity Delta Threshold", ESP_RMAKER_PARAM_RANGE,
                                esp_rmaker_float(state->config.humidity_delta_threshold_pct), PROP_FLAG_READ | PROP_FLAG_WRITE));
    esp_rmaker_device_add_param(
        s_fan_device,
        esp_rmaker_param_create("Humidity Absolute Threshold", ESP_RMAKER_PARAM_RANGE,
                                esp_rmaker_float(state->config.humidity_absolute_threshold_pct), PROP_FLAG_READ | PROP_FLAG_WRITE));
    esp_rmaker_device_add_param(
        s_fan_device,
        esp_rmaker_param_create("Fan Hold Time", ESP_RMAKER_PARAM_RANGE, esp_rmaker_int(state->config.fan_hold_time_s),
                                PROP_FLAG_READ | PROP_FLAG_WRITE));
    esp_rmaker_device_add_param(
        s_fan_device,
        esp_rmaker_param_create("Min Duty", ESP_RMAKER_PARAM_SPEED, esp_rmaker_int(state->config.min_duty_percent),
                                PROP_FLAG_READ | PROP_FLAG_WRITE));
    esp_rmaker_device_add_param(
        s_fan_device,
        esp_rmaker_param_create("Max Duty", ESP_RMAKER_PARAM_SPEED, esp_rmaker_int(state->config.max_duty_percent),
                                PROP_FLAG_READ | PROP_FLAG_WRITE));
    esp_rmaker_device_add_param(
        s_fan_device,
        esp_rmaker_param_create("Kick Start Enable", ESP_RMAKER_PARAM_POWER_MODE, esp_rmaker_bool(state->config.kickstart_enable),
                                PROP_FLAG_READ | PROP_FLAG_WRITE));
    esp_rmaker_device_add_param(
        s_fan_device,
        esp_rmaker_param_create("Kick Start Duration", ESP_RMAKER_PARAM_RANGE, esp_rmaker_int(state->config.kickstart_duration_ms),
                                PROP_FLAG_READ | PROP_FLAG_WRITE));

    s_mode_param = esp_rmaker_param_create("Mode", ESP_RMAKER_PARAM_MODE, esp_rmaker_str(fan_mode_to_str(state->fan.requested_mode)),
                                           PROP_FLAG_READ | PROP_FLAG_WRITE);
    s_lux_param = telemetry_param("Lux", RMK_TYPE_NUMBER, esp_rmaker_float(0));
    s_temp_param = telemetry_param("Temperature", RMK_TYPE_NUMBER, esp_rmaker_float(0));
    s_humidity_param = telemetry_param("Humidity", RMK_TYPE_NUMBER, esp_rmaker_float(0));
    s_tvoc_param = telemetry_param("TVOC", RMK_TYPE_NUMBER, esp_rmaker_int(0));
    s_eco2_param = telemetry_param("eCO2", RMK_TYPE_NUMBER, esp_rmaker_int(0));
    s_aqi_param = telemetry_param("AQI", RMK_TYPE_NUMBER, esp_rmaker_int(0));
    s_ble_present_param = telemetry_param("BLE Presence", ESP_RMAKER_PARAM_POWER_MODE, esp_rmaker_bool(false));
    s_rssi_param = telemetry_param("RSSI", RMK_TYPE_NUMBER, esp_rmaker_int(-100));
    s_trigger_param = telemetry_param("Trigger State", RMK_TYPE_STRING, esp_rmaker_str("none"));
    s_fan_state_param = telemetry_param("Fan State", ESP_RMAKER_PARAM_MODE, esp_rmaker_str("STOP"));
    s_fan_duty_param = telemetry_param("Fan Duty", RMK_TYPE_NUMBER, esp_rmaker_int(0));
    s_wifi_param = telemetry_param("WiFi Status", ESP_RMAKER_PARAM_POWER_MODE, esp_rmaker_bool(false));
    s_rmk_param = telemetry_param("RainMaker Status", ESP_RMAKER_PARAM_POWER_MODE, esp_rmaker_bool(true));
    s_runtime_param = telemetry_param("Daily Runtime", RMK_TYPE_NUMBER, esp_rmaker_int(0));

    esp_rmaker_device_add_param(s_fan_device, s_mode_param);
    esp_rmaker_device_add_param(s_fan_device, s_lux_param);
    esp_rmaker_device_add_param(s_fan_device, s_temp_param);
    esp_rmaker_device_add_param(s_fan_device, s_humidity_param);
    esp_rmaker_device_add_param(s_fan_device, s_tvoc_param);
    esp_rmaker_device_add_param(s_fan_device, s_eco2_param);
    esp_rmaker_device_add_param(s_fan_device, s_aqi_param);
    esp_rmaker_device_add_param(s_fan_device, s_ble_present_param);
    esp_rmaker_device_add_param(s_fan_device, s_rssi_param);
    esp_rmaker_device_add_param(s_fan_device, s_trigger_param);
    esp_rmaker_device_add_param(s_fan_device, s_fan_state_param);
    esp_rmaker_device_add_param(s_fan_device, s_fan_duty_param);
    esp_rmaker_device_add_param(s_fan_device, s_wifi_param);
    esp_rmaker_device_add_param(s_fan_device, s_rmk_param);
    esp_rmaker_device_add_param(s_fan_device, s_runtime_param);
    esp_rmaker_node_add_device(s_node, s_fan_device);

    esp_rmaker_ota_enable_default();
    esp_rmaker_timezone_service_enable();
    esp_rmaker_schedule_enable();
    esp_rmaker_start();

    app_state_lock(state);
    state->connectivity.rainmaker_connected = true;
    app_state_unlock(state);
    ESP_LOGI(TAG, "RainMaker initialized");
    return ESP_OK;
#else
    app_state_lock(state);
    state->connectivity.rainmaker_connected = false;
    app_state_unlock(state);
    ESP_LOGW(TAG, "RainMaker component not present, running offline-only");
    return ESP_OK;
#endif
}

esp_err_t rainmaker_app_report_state(app_state_t *state)
{
#if HAS_RAINMAKER
    char trigger_buf[64];
    app_state_lock(state);
    snprintf(trigger_buf,
             sizeof(trigger_buf),
             "%s%s%s%s",
             state->triggers.ble_trigger ? "ble," : "",
             state->triggers.light_trigger ? "light," : "",
             state->triggers.humidity_trigger ? "humidity," : "",
             state->triggers.air_trigger ? "air" : "");
    esp_rmaker_param_update_and_report(esp_rmaker_device_get_param_by_name(s_fan_device, ESP_RMAKER_DEF_POWER_NAME),
                                       esp_rmaker_bool(state->fan.requested_mode != FAN_MODE_OFF));
    esp_rmaker_param_update_and_report(s_mode_param, esp_rmaker_str(fan_mode_to_str(state->fan.requested_mode)));
    esp_rmaker_param_update_and_report(s_lux_param, esp_rmaker_float(state->sensors.lux));
    esp_rmaker_param_update_and_report(s_temp_param, esp_rmaker_float(state->sensors.temp_c));
    esp_rmaker_param_update_and_report(s_humidity_param, esp_rmaker_float(state->sensors.humidity_pct));
    esp_rmaker_param_update_and_report(s_tvoc_param, esp_rmaker_int(state->sensors.tvoc_ppb));
    esp_rmaker_param_update_and_report(s_eco2_param, esp_rmaker_int(state->sensors.eco2_ppm));
    esp_rmaker_param_update_and_report(s_aqi_param, esp_rmaker_int(state->sensors.aqi));
    esp_rmaker_param_update_and_report(s_ble_present_param, esp_rmaker_bool(state->ble.stable_present));
    esp_rmaker_param_update_and_report(s_rssi_param, esp_rmaker_int(state->ble.strongest_rssi));
    esp_rmaker_param_update_and_report(s_trigger_param, esp_rmaker_str(trigger_buf[0] ? trigger_buf : "none"));
    esp_rmaker_param_update_and_report(s_fan_state_param, esp_rmaker_str(fan_state_to_str(state->fan.state)));
    esp_rmaker_param_update_and_report(s_fan_duty_param, esp_rmaker_int(state->fan.duty_percent));
    esp_rmaker_param_update_and_report(s_wifi_param, esp_rmaker_bool(state->connectivity.wifi_connected));
    esp_rmaker_param_update_and_report(s_rmk_param, esp_rmaker_bool(state->connectivity.rainmaker_connected));
    esp_rmaker_param_update_and_report(s_runtime_param, esp_rmaker_int(state->fan.runtime_seconds_today));
    app_state_unlock(state);
    return ESP_OK;
#else
    (void)state;
    return ESP_OK;
#endif
}
