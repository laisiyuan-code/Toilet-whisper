#include "data_logger.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "esp_log.h"
#include "esp_timer.h"

#include "config.h"

static const char *TAG = "data_logger";
static bool s_ready;

static esp_err_t ensure_header(void)
{
    struct stat st = {0};
    if (stat(LOGGER_CSV_PATH, &st) == 0) {
        return ESP_OK;
    }

    FILE *fp = fopen(LOGGER_CSV_PATH, "w");
    if (!fp) {
        return ESP_FAIL;
    }

    fputs("timestamp,uptime_s,lux,temp_c,humidity_pct,tvoc_ppb,eco2_ppm,aqi,ble_present,device_name,rssi,"
          "light_trigger,humidity_trigger,air_trigger,fan_mode,fan_state,fan_duty,manual_override,wifi_connected,"
          "rainmaker_connected\n",
          fp);
    fclose(fp);
    return ESP_OK;
}

esp_err_t data_logger_init(app_state_t *state)
{
    (void)state;
    ESP_RETURN_ON_ERROR(ensure_header(), TAG, "failed to create CSV header");

    FILE *fp = fopen(LOGGER_EVENT_PATH, "a");
    if (!fp) {
        return ESP_FAIL;
    }
    fclose(fp);
    s_ready = true;
    return ESP_OK;
}

esp_err_t data_logger_log_snapshot(app_state_t *state)
{
    if (!s_ready) {
        return ESP_ERR_INVALID_STATE;
    }

    char timestamp[32] = "0";
    time_t now = time(NULL);
    struct tm timeinfo = {0};
    if (localtime_r(&now, &timeinfo)) {
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
    }

    FILE *fp = fopen(LOGGER_CSV_PATH, "a");
    if (!fp) {
        return ESP_FAIL;
    }

    app_state_lock(state);
    uint32_t uptime_s = (uint32_t)(esp_timer_get_time() / 1000000ULL);
    fprintf(fp,
            "%s,%u,%.2f,%.2f,%.2f,%u,%u,%u,%d,%s,%d,%d,%d,%d,%s,%s,%u,%d,%d,%d\n",
            timestamp,
            uptime_s,
            state->sensors.lux,
            state->sensors.temp_c,
            state->sensors.humidity_pct,
            state->sensors.tvoc_ppb,
            state->sensors.eco2_ppm,
            state->sensors.aqi,
            state->ble.stable_present,
            state->ble.strongest_name,
            state->ble.strongest_rssi,
            state->triggers.light_trigger,
            state->triggers.humidity_trigger,
            state->triggers.air_trigger,
            fan_mode_to_str(state->fan.requested_mode),
            fan_state_to_str(state->fan.state),
            state->fan.duty_percent,
            state->triggers.manual_override,
            state->connectivity.wifi_connected,
            state->connectivity.rainmaker_connected);
    app_state_unlock(state);

    fclose(fp);
    return ESP_OK;
}

esp_err_t data_logger_log_event(app_state_t *state, const char *event_tag, const char *message)
{
    (void)state;
    if (!s_ready) {
        return ESP_ERR_INVALID_STATE;
    }

    FILE *fp = fopen(LOGGER_EVENT_PATH, "a");
    if (!fp) {
        return ESP_FAIL;
    }

    char timestamp[32] = "0";
    time_t now = time(NULL);
    struct tm timeinfo = {0};
    if (localtime_r(&now, &timeinfo)) {
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
    }

    fprintf(fp, "[%s] %s: %s\n", timestamp, event_tag ? event_tag : "event", message ? message : "");
    fclose(fp);
    ESP_LOGI(TAG, "%s: %s", event_tag ? event_tag : "event", message ? message : "");
    return ESP_OK;
}
