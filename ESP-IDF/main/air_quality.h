#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

typedef struct {
    bool initialized;
    bool aht21_ok;
    bool ens160_ok;
    float temp_c;
    float humidity_pct;
    uint16_t tvoc_ppb;
    uint16_t eco2_ppm;
    uint8_t aqi;
} air_quality_dev_t;

esp_err_t air_quality_init(air_quality_dev_t *dev);
esp_err_t air_quality_read(air_quality_dev_t *dev);
