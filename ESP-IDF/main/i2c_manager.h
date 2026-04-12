#pragma once

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

esp_err_t i2c_manager_init(void);
esp_err_t i2c_manager_write(uint8_t addr, const uint8_t *data, size_t len);
esp_err_t i2c_manager_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len);
esp_err_t i2c_manager_read(uint8_t addr, uint8_t *data, size_t len);
esp_err_t i2c_manager_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, size_t len);
