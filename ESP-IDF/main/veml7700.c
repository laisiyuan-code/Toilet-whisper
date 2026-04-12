#include "veml7700.h"

#include <stdint.h>

#include "esp_log.h"

#include "i2c_manager.h"

static const char *TAG = "veml7700";

#define VEML7700_I2C_ADDR        0x10
#define VEML7700_REG_ALS_CONF    0x00
#define VEML7700_REG_ALS_DATA    0x04

static esp_err_t write16(uint8_t reg, uint16_t value)
{
    uint8_t payload[3] = {reg, (uint8_t)(value & 0xFF), (uint8_t)(value >> 8)};
    return i2c_manager_write(VEML7700_I2C_ADDR, payload, sizeof(payload));
}

static esp_err_t read16(uint8_t reg, uint16_t *value)
{
    uint8_t data[2] = {0};
    ESP_RETURN_ON_ERROR(i2c_manager_read_reg(VEML7700_I2C_ADDR, reg, data, sizeof(data)), TAG, "read16 failed");
    *value = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
    return ESP_OK;
}

esp_err_t veml7700_init(veml7700_t *dev)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    /* ALS gain x1/8, integration time 100 ms, ALS enabled. */
    ESP_RETURN_ON_ERROR(write16(VEML7700_REG_ALS_CONF, 0x1000), TAG, "config write failed");
    dev->initialized = true;
    dev->latest_lux = 0.0f;
    ESP_LOGI(TAG, "VEML7700 initialized");
    return ESP_OK;
}

esp_err_t veml7700_read_lux(veml7700_t *dev, float *lux)
{
    if (!dev || !dev->initialized || !lux) {
        return ESP_ERR_INVALID_STATE;
    }

    uint16_t raw = 0;
    ESP_RETURN_ON_ERROR(read16(VEML7700_REG_ALS_DATA, &raw), TAG, "ALS read failed");

    /* For gain x1/8, IT 100 ms the typical resolution is 0.0576 lux/step. */
    dev->latest_lux = raw * 0.0576f;
    *lux = dev->latest_lux;
    return ESP_OK;
}
