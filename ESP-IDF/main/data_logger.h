#pragma once

#include "esp_err.h"

#include "app_main.h"

esp_err_t data_logger_init(app_state_t *state);
esp_err_t data_logger_log_snapshot(app_state_t *state);
esp_err_t data_logger_log_event(app_state_t *state, const char *event_tag, const char *message);
