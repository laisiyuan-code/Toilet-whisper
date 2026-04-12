#include "fan_control.h"

#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/task.h"

#include "config.h"
#include "data_logger.h"

static const char *TAG = "fan_control";

uint32_t fan_control_percent_to_duty(uint8_t duty_percent)
{
    const uint32_t max_duty = (1U << FAN_PWM_RESOLUTION) - 1U;
    return (uint32_t)((max_duty * duty_percent) / 100U);
}

static esp_err_t set_pwm_percent(uint8_t duty_percent)
{
    ESP_RETURN_ON_ERROR(ledc_set_duty(FAN_PWM_MODE, FAN_PWM_CHANNEL, fan_control_percent_to_duty(duty_percent)),
                        TAG,
                        "set duty failed");
    return ledc_update_duty(FAN_PWM_MODE, FAN_PWM_CHANNEL);
}

esp_err_t fan_control_init(app_state_t *state)
{
    ledc_timer_config_t timer = {
        .speed_mode = FAN_PWM_MODE,
        .duty_resolution = FAN_PWM_RESOLUTION,
        .timer_num = FAN_PWM_TIMER,
        .freq_hz = FAN_PWM_FREQUENCY_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_channel_config_t channel = {
        .gpio_num = FAN_PWM_GPIO,
        .speed_mode = FAN_PWM_MODE,
        .channel = FAN_PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = FAN_PWM_TIMER,
        .duty = 0,
        .hpoint = 0,
        .flags.output_invert = 0,
    };

    ESP_RETURN_ON_ERROR(ledc_timer_config(&timer), TAG, "timer config failed");
    ESP_RETURN_ON_ERROR(ledc_channel_config(&channel), TAG, "channel config failed");

    app_state_lock(state);
    state->fan.duty_percent = 0;
    state->fan.target_duty_percent = 0;
    state->fan.state = FAN_STATE_STOPPED;
    state->fan.power = false;
    state->fan.applied_mode = FAN_MODE_OFF;
    state->fan.last_state_change_us = esp_timer_get_time();
    app_state_unlock(state);

    ESP_LOGI(TAG, "Fan control initialized on GPIO %d", FAN_PWM_GPIO);
    return ESP_OK;
}

static uint8_t clamp_target(const app_config_t *cfg, uint8_t duty)
{
    if (duty == 0) {
        return 0;
    }
    if (duty < cfg->min_duty_percent) {
        duty = cfg->min_duty_percent;
    }
    if (duty > cfg->max_duty_percent) {
        duty = cfg->max_duty_percent;
    }
    return duty;
}

esp_err_t fan_control_apply(app_state_t *state, fan_mode_t mode, uint8_t target_duty_percent)
{
    uint8_t clamped = 0;
    bool start_transition = false;

    app_state_lock(state);
    clamped = clamp_target(&state->config, target_duty_percent);
    start_transition = (state->fan.duty_percent == 0 && clamped > 0);
    app_state_unlock(state);

    if (clamped == 0 || mode == FAN_MODE_OFF) {
        ESP_RETURN_ON_ERROR(set_pwm_percent(0), TAG, "fan stop failed");
        app_state_lock(state);
        if (state->fan.runtime_active) {
            int64_t delta_us = esp_timer_get_time() - state->fan.runtime_started_us;
            state->fan.runtime_seconds_today += (uint32_t)(delta_us / 1000000ULL);
            state->fan.runtime_active = false;
        }
        state->fan.duty_percent = 0;
        state->fan.target_duty_percent = 0;
        state->fan.power = false;
        state->fan.state = FAN_STATE_STOPPED;
        state->fan.applied_mode = FAN_MODE_OFF;
        state->fan.last_state_change_us = esp_timer_get_time();
        xEventGroupSetBits(state->events, APP_EVENT_FAN_CHANGED);
        app_state_unlock(state);
        data_logger_log_event(state, "fan_state", "Fan stopped");
        return ESP_OK;
    }

    app_state_lock(state);
    bool kick = state->config.kickstart_enable;
    uint32_t kick_ms = state->config.kickstart_duration_ms;
    app_state_unlock(state);

    if (kick && start_transition) {
        ESP_RETURN_ON_ERROR(set_pwm_percent(100), TAG, "kick-start failed");
        app_state_lock(state);
        state->fan.state = FAN_STATE_STARTING;
        state->fan.last_state_change_us = esp_timer_get_time();
        app_state_unlock(state);
        vTaskDelay(pdMS_TO_TICKS(kick_ms));
    }

    ESP_RETURN_ON_ERROR(set_pwm_percent(clamped), TAG, "set target duty failed");

    app_state_lock(state);
    if (!state->fan.runtime_active) {
        state->fan.runtime_active = true;
        state->fan.runtime_started_us = esp_timer_get_time();
        state->fan.activation_count_today++;
    }
    state->fan.duty_percent = clamped;
    state->fan.target_duty_percent = clamped;
    state->fan.power = true;
    state->fan.state = FAN_STATE_RUNNING;
    state->fan.applied_mode = mode;
    state->fan.last_state_change_us = esp_timer_get_time();
    xEventGroupSetBits(state->events, APP_EVENT_FAN_CHANGED);
    app_state_unlock(state);

    data_logger_log_event(state, "fan_state", "Fan duty updated");
    return ESP_OK;
}
