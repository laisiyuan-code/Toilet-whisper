#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "esp_err.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FAN_MODE_OFF = 0,
    FAN_MODE_AUTO,
    FAN_MODE_LOW,
    FAN_MODE_MEDIUM,
    FAN_MODE_HIGH,
} fan_mode_t;

typedef enum {
    FAN_STATE_STOPPED = 0,
    FAN_STATE_STARTING,
    FAN_STATE_RUNNING,
} fan_state_t;

typedef struct {
    bool ble_enable;
    bool light_enable;
    bool humidity_enable;
    bool air_enable;

    int ble_rssi_threshold_dbm;
    float light_delta_threshold_lux;
    float humidity_delta_threshold_pct;
    float humidity_absolute_threshold_pct;
    uint16_t tvoc_threshold_ppb;
    uint16_t eco2_threshold_ppm;
    uint8_t aqi_threshold;

    uint32_t sensor_period_ms;
    uint32_t decision_period_ms;
    uint32_t status_period_ms;
    uint32_t log_period_ms;
    uint32_t telemetry_period_ms;

    uint32_t fan_hold_time_s;
    uint32_t light_only_run_time_s;
    uint32_t presence_hold_time_s;
    uint32_t start_hold_time_s;

    uint8_t min_duty_percent;
    uint8_t low_duty_percent;
    uint8_t medium_duty_percent;
    uint8_t high_duty_percent;
    uint8_t max_duty_percent;
    bool kickstart_enable;
    uint32_t kickstart_duration_ms;

    uint16_t ble_scan_interval_ms;
    uint16_t ble_scan_window_ms;
    uint16_t ble_scan_burst_ms;
    uint16_t ble_scan_idle_ms;
    uint8_t ble_presence_debounce_count;
} app_config_t;

typedef struct {
    bool veml7700_ok;
    bool aht21_ok;
    bool ens160_ok;
    bool last_read_ok;
    float lux;
    float lux_baseline;
    float temp_c;
    float humidity_pct;
    float humidity_baseline;
    uint16_t tvoc_ppb;
    uint16_t eco2_ppm;
    uint8_t aqi;
    int64_t last_update_us;
} sensor_snapshot_t;

typedef struct {
    bool present;
    bool stable_present;
    bool scanning;
    int strongest_rssi;
    char strongest_name[32];
    char strongest_addr[18];
    uint8_t debounce_counter;
    int64_t last_seen_us;
    int64_t last_lost_us;
    int64_t last_scan_start_us;
    uint32_t detections_today;
} ble_presence_state_t;

typedef struct {
    bool ble_trigger;
    bool light_trigger;
    bool humidity_trigger;
    bool air_trigger;
    bool occupancy;
    bool manual_override;
    int64_t occupancy_hold_until_us;
    int64_t vent_hold_until_us;
    int64_t light_only_until_us;
    int64_t start_hold_until_us;
} trigger_state_t;

typedef struct {
    fan_mode_t requested_mode;
    fan_mode_t applied_mode;
    fan_state_t state;
    bool power;
    uint8_t duty_percent;
    uint8_t target_duty_percent;
    bool runtime_active;
    int64_t last_state_change_us;
    int64_t runtime_started_us;
    uint32_t activation_count_today;
    uint32_t runtime_seconds_today;
} fan_runtime_t;

typedef struct {
    float max_humidity_pct;
    float humidity_accumulator;
    uint32_t humidity_samples;
    uint16_t peak_tvoc_ppb;
    uint16_t peak_eco2_ppm;
    uint32_t ble_detections;
    uint32_t fan_runtime_s;
    uint32_t fan_activation_count;
    int day_of_year;
    time_t last_persist_time;
} daily_summary_t;

typedef struct {
    bool wifi_connected;
    bool rainmaker_connected;
    bool time_synced;
} connectivity_state_t;

typedef struct {
    SemaphoreHandle_t mutex;
    EventGroupHandle_t events;
    app_config_t config;
    sensor_snapshot_t sensors;
    ble_presence_state_t ble;
    trigger_state_t triggers;
    fan_runtime_t fan;
    daily_summary_t summary;
    connectivity_state_t connectivity;
    time_t boot_time;
} app_state_t;

#define APP_EVENT_SENSORS_UPDATED    BIT0
#define APP_EVENT_PRESENCE_UPDATED   BIT1
#define APP_EVENT_CONFIG_CHANGED     BIT2
#define APP_EVENT_TRIGGER_CHANGED    BIT3
#define APP_EVENT_FAN_CHANGED        BIT4
#define APP_EVENT_WIFI_CHANGED       BIT5
#define APP_EVENT_RAINMAKER_CHANGED  BIT6

void app_state_lock(app_state_t *state);
void app_state_unlock(app_state_t *state);
int64_t app_now_us(void);
const char *fan_mode_to_str(fan_mode_t mode);
const char *fan_state_to_str(fan_state_t state);
fan_mode_t fan_mode_from_str(const char *value);
esp_err_t app_load_config(app_state_t *state);
esp_err_t app_save_config(app_state_t *state);
esp_err_t app_load_daily_summary(app_state_t *state);
esp_err_t app_save_daily_summary(app_state_t *state);
void app_reset_daily_summary_if_needed(app_state_t *state, struct tm *timeinfo);
void app_update_daily_summary_from_sensor(app_state_t *state);

#ifdef __cplusplus
}
#endif
