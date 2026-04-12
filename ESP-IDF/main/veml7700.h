#pragma once

#include <stdbool.h>

#include "esp_err.h"

typedef struct {
    bool initialized;
    float latest_lux;
} veml7700_t;

esp_err_t veml7700_init(veml7700_t *dev);
esp_err_t veml7700_read_lux(veml7700_t *dev, float *lux);
