#pragma once

#include "esp_err.h"

#include "app_main.h"

esp_err_t ble_presence_init(app_state_t *state);
esp_err_t ble_presence_start(app_state_t *state);
void ble_presence_stop(void);
