#include "decision_engine.h"

#include <stdio.h>

#include "esp_log.h"

#include "data_logger.h"
#include "fan_control.h"

static const char *TAG = "decision_engine";

static bool is_held(int64_t until_us, int64_t now_us)
{
    return until_us > 0 && until_us > now_us;
}

static uint8_t duty_for_mode(const app_config_t *cfg, fan_mode_t mode)
{
    switch (mode) {
    case FAN_MODE_LOW:
        return cfg->low_duty_percent;
    case FAN_MODE_MEDIUM:
        return cfg->medium_duty_percent;
    case FAN_MODE_HIGH:
        return cfg->high_duty_percent;
    default:
        return 0;
    }
}

esp_err_t decision_engine_step(app_state_t *state)
{
    int64_t now = app_now_us();
    fan_mode_t requested_mode;
    fan_mode_t target_mode = FAN_MODE_OFF;
    uint8_t target_duty = 0;
    bool changed = false;

    app_state_lock(state);

    float lux_delta = state->sensors.lux - state->sensors.lux_baseline;
    float humidity_delta = state->sensors.humidity_pct - state->sensors.humidity_baseline;

    bool ble_trigger = state->config.ble_enable &&
                       state->ble.stable_present &&
                       state->ble.strongest_rssi >= state->config.ble_rssi_threshold_dbm;
    bool light_trigger = state->config.light_enable &&
                         (lux_delta >= state->config.light_delta_threshold_lux);
    bool humidity_trigger = state->config.humidity_enable &&
                            ((humidity_delta >= state->config.humidity_delta_threshold_pct) ||
                             (state->sensors.humidity_pct >= state->config.humidity_absolute_threshold_pct));
    bool air_trigger = state->config.air_enable &&
                       ((state->sensors.tvoc_ppb >= state->config.tvoc_threshold_ppb) ||
                        (state->sensors.eco2_ppm >= state->config.eco2_threshold_ppm) ||
                        (state->sensors.aqi >= state->config.aqi_threshold));

    bool occupancy = ble_trigger && light_trigger;
    if (occupancy) {
        state->triggers.occupancy_hold_until_us = now + ((int64_t)state->config.presence_hold_time_s * 1000000LL);
    } else if (is_held(state->triggers.occupancy_hold_until_us, now)) {
        occupancy = true;
    }

    if (humidity_trigger || air_trigger || occupancy) {
        state->triggers.vent_hold_until_us = now + ((int64_t)state->config.fan_hold_time_s * 1000000LL);
    } else if (light_trigger && !ble_trigger) {
        state->triggers.light_only_until_us = now + ((int64_t)state->config.light_only_run_time_s * 1000000LL);
    }

    requested_mode = state->fan.requested_mode;
    state->triggers.ble_trigger = ble_trigger;
    state->triggers.light_trigger = light_trigger;
    state->triggers.humidity_trigger = humidity_trigger;
    state->triggers.air_trigger = air_trigger;
    state->triggers.occupancy = occupancy;
    state->triggers.manual_override = (requested_mode != FAN_MODE_AUTO);

    if (requested_mode == FAN_MODE_OFF) {
        target_mode = FAN_MODE_OFF;
    } else if (requested_mode == FAN_MODE_LOW || requested_mode == FAN_MODE_MEDIUM || requested_mode == FAN_MODE_HIGH) {
        target_mode = requested_mode;
        target_duty = duty_for_mode(&state->config, target_mode);
    } else {
        if (humidity_trigger || air_trigger) {
            target_mode = FAN_MODE_HIGH;
        } else if (occupancy || is_held(state->triggers.vent_hold_until_us, now)) {
            target_mode = FAN_MODE_MEDIUM;
        } else if (is_held(state->triggers.light_only_until_us, now)) {
            target_mode = FAN_MODE_LOW;
        } else {
            target_mode = FAN_MODE_OFF;
        }
        target_duty = duty_for_mode(&state->config, target_mode);
    }

    changed = (target_mode != state->fan.applied_mode) || (target_duty != state->fan.duty_percent);
    if (changed) {
        xEventGroupSetBits(state->events, APP_EVENT_TRIGGER_CHANGED);
    }
    app_state_unlock(state);

    if (changed) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Decision -> mode=%s duty=%u", fan_mode_to_str(target_mode), target_duty);
        ESP_LOGI(TAG, "%s", msg);
        data_logger_log_event(state, "decision", msg);
        return fan_control_apply(state, target_mode, target_duty);
    }

    return ESP_OK;
}
