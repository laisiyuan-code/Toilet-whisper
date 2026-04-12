#include "air_quality.h"

#include <math.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "i2c_manager.h"

static const char *TAG = "air_quality";

#define AHT21_I2C_ADDR               0x38
#define AHT21_INIT_CMD_MSB           0xBE
#define AHT21_INIT_CMD_LSB           0x08
#define AHT21_INIT_CMD_CRC           0x00
#define AHT21_MEASURE_CMD_MSB        0xAC
#define AHT21_MEASURE_CMD_LSB        0x33
#define AHT21_MEASURE_CMD_CRC        0x00

#define ENS160_I2C_ADDR              0x53
#define ENS160_REG_OPMODE            0x10
#define ENS160_REG_CONFIG            0x11
#define ENS160_REG_DATA_STATUS       0x20
#define ENS160_REG_DATA_AQI          0x21
#define ENS160_REG_DATA_TVOC         0x22
#define ENS160_REG_DATA_ECO2         0x24
#define ENS160_REG_TEMP_IN           0x13
#define ENS160_REG_RH_IN             0x15

static esp_err_t aht21_init(air_quality_dev_t *dev)
{
    uint8_t init_cmd[3] = {AHT21_INIT_CMD_MSB, AHT21_INIT_CMD_LSB, AHT21_INIT_CMD_CRC};
    esp_err_t err = i2c_manager_write(AHT21_I2C_ADDR, init_cmd, sizeof(init_cmd));
    if (err == ESP_OK) {
        vTaskDelay(pdMS_TO_TICKS(20));
        dev->aht21_ok = true;
    }
    return err;
}

static esp_err_t aht21_read(air_quality_dev_t *dev)
{
    uint8_t cmd[3] = {AHT21_MEASURE_CMD_MSB, AHT21_MEASURE_CMD_LSB, AHT21_MEASURE_CMD_CRC};
    uint8_t raw[6] = {0};

    ESP_RETURN_ON_ERROR(i2c_manager_write(AHT21_I2C_ADDR, cmd, sizeof(cmd)), TAG, "AHT21 trigger failed");
    vTaskDelay(pdMS_TO_TICKS(90));
    ESP_RETURN_ON_ERROR(i2c_manager_read(AHT21_I2C_ADDR, raw, sizeof(raw)), TAG, "AHT21 read failed");

    uint32_t humidity_raw = ((uint32_t)(raw[1] & 0x0F) << 16) | ((uint32_t)raw[2] << 8) | raw[3];
    uint32_t temp_raw = ((uint32_t)(raw[3] & 0xF0) << 12) | ((uint32_t)raw[4] << 8) | raw[5];

    dev->humidity_pct = ((float)humidity_raw * 100.0f) / 1048576.0f;
    dev->temp_c = (((float)temp_raw * 200.0f) / 1048576.0f) - 50.0f;
    dev->aht21_ok = true;
    return ESP_OK;
}

static esp_err_t ens160_write_u16(uint8_t reg, uint16_t value)
{
    uint8_t payload[3] = {reg, (uint8_t)(value & 0xFF), (uint8_t)(value >> 8)};
    return i2c_manager_write(ENS160_I2C_ADDR, payload, sizeof(payload));
}

static esp_err_t ens160_read(uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_manager_read_reg(ENS160_I2C_ADDR, reg, data, len);
}

static esp_err_t ens160_init(air_quality_dev_t *dev)
{
    uint8_t payload[2] = {ENS160_REG_OPMODE, 0x02};
    ESP_RETURN_ON_ERROR(i2c_manager_write(ENS160_I2C_ADDR, payload, sizeof(payload)), TAG, "ENS160 opmode failed");
    vTaskDelay(pdMS_TO_TICKS(20));
    dev->ens160_ok = true;
    return ESP_OK;
}

static esp_err_t ens160_apply_compensation(const air_quality_dev_t *dev)
{
    uint16_t temp_in = (uint16_t)lrintf((dev->temp_c + 273.15f) * 64.0f);
    uint16_t rh_in = (uint16_t)lrintf(dev->humidity_pct * 512.0f);
    ESP_RETURN_ON_ERROR(ens160_write_u16(ENS160_REG_TEMP_IN, temp_in), TAG, "ENS160 temp comp failed");
    ESP_RETURN_ON_ERROR(ens160_write_u16(ENS160_REG_RH_IN, rh_in), TAG, "ENS160 rh comp failed");
    return ESP_OK;
}

static esp_err_t ens160_read_data(air_quality_dev_t *dev)
{
    uint8_t status = 0;
    uint8_t aqi = 0;
    uint8_t tvoc_raw[2] = {0};
    uint8_t eco2_raw[2] = {0};

    ESP_RETURN_ON_ERROR(ens160_apply_compensation(dev), TAG, "ENS160 compensation failed");
    ESP_RETURN_ON_ERROR(ens160_read(ENS160_REG_DATA_STATUS, &status, 1), TAG, "ENS160 status failed");
    if ((status & 0x01U) == 0) {
        return ESP_ERR_NOT_FINISHED;
    }

    ESP_RETURN_ON_ERROR(ens160_read(ENS160_REG_DATA_AQI, &aqi, 1), TAG, "ENS160 AQI failed");
    ESP_RETURN_ON_ERROR(ens160_read(ENS160_REG_DATA_TVOC, tvoc_raw, sizeof(tvoc_raw)), TAG, "ENS160 TVOC failed");
    ESP_RETURN_ON_ERROR(ens160_read(ENS160_REG_DATA_ECO2, eco2_raw, sizeof(eco2_raw)), TAG, "ENS160 eCO2 failed");

    dev->aqi = aqi;
    dev->tvoc_ppb = (uint16_t)tvoc_raw[0] | ((uint16_t)tvoc_raw[1] << 8);
    dev->eco2_ppm = (uint16_t)eco2_raw[0] | ((uint16_t)eco2_raw[1] << 8);
    dev->ens160_ok = true;
    return ESP_OK;
}

esp_err_t air_quality_init(air_quality_dev_t *dev)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(dev, 0, sizeof(*dev));
    esp_err_t aht_err = aht21_init(dev);
    if (aht_err != ESP_OK) {
        ESP_LOGW(TAG, "AHT21 init failed: %s", esp_err_to_name(aht_err));
    }

    esp_err_t ens_err = ens160_init(dev);
    if (ens_err != ESP_OK) {
        ESP_LOGW(TAG, "ENS160 init failed: %s", esp_err_to_name(ens_err));
    }

    dev->initialized = (aht_err == ESP_OK) || (ens_err == ESP_OK);
    return dev->initialized ? ESP_OK : ESP_FAIL;
}

esp_err_t air_quality_read(air_quality_dev_t *dev)
{
    if (!dev || !dev->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t result = ESP_OK;
    esp_err_t aht_err = aht21_read(dev);
    if (aht_err != ESP_OK) {
        dev->aht21_ok = false;
        result = aht_err;
        ESP_LOGW(TAG, "AHT21 read failed: %s", esp_err_to_name(aht_err));
    }

    esp_err_t ens_err = ens160_read_data(dev);
    if (ens_err != ESP_OK) {
        dev->ens160_ok = false;
        result = (result == ESP_OK) ? ens_err : result;
        ESP_LOGW(TAG, "ENS160 read failed: %s", esp_err_to_name(ens_err));
    }

    return result;
}
