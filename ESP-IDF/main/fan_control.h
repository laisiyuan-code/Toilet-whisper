#pragma once

#include "esp_err.h"

#include "app_main.h"

esp_err_t fan_control_init(app_state_t *state);
esp_err_t fan_control_apply(app_state_t *state, fan_mode_t mode, uint8_t target_duty_percent);
uint32_t fan_control_percent_to_duty(uint8_t duty_percent);
