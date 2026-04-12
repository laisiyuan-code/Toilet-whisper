#pragma once

#include "esp_err.h"

#include "app_main.h"

esp_err_t rainmaker_app_init(app_state_t *state);
esp_err_t rainmaker_app_report_state(app_state_t *state);
bool rainmaker_app_is_enabled(void);
