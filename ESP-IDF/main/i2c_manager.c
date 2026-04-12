#include "i2c_manager.h"

#include <stdbool.h>

#include "driver/i2c.h"
#include "esp_log.h"

#include "config.h"

static const char *TAG = "i2c_manager";
static bool s_initialized;

esp_err_t i2c_manager_init(void)
{
    if (s_initialized) {
        return ESP_OK;
    }

    i2c_config_t cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_CLK_SPEED_HZ,
        .clk_flags = 0,
    };

    ESP_RETURN_ON_ERROR(i2c_param_config(I2C_PORT_NUM, &cfg), TAG, "i2c param config failed");
    ESP_RETURN_ON_ERROR(i2c_driver_install(I2C_PORT_NUM, cfg.mode, 0, 0, 0), TAG, "i2c install failed");
    s_initialized = true;
    ESP_LOGI(TAG, "I2C initialized on SDA=%d SCL=%d", I2C_SDA_GPIO, I2C_SCL_GPIO);
    return ESP_OK;
}

esp_err_t i2c_manager_write(uint8_t addr, const uint8_t *data, size_t len)
{
    return i2c_master_write_to_device(I2C_PORT_NUM, addr, data, len, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
}

esp_err_t i2c_manager_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len)
{
    if (!data && len > 0) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buffer[32];
    if (len + 1 > sizeof(buffer)) {
        return ESP_ERR_INVALID_SIZE;
    }

    buffer[0] = reg;
    for (size_t i = 0; i < len; ++i) {
        buffer[i + 1] = data[i];
    }
    return i2c_master_write_to_device(I2C_PORT_NUM, addr, buffer, len + 1, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
}

esp_err_t i2c_manager_read(uint8_t addr, uint8_t *data, size_t len)
{
    return i2c_master_read_from_device(I2C_PORT_NUM, addr, data, len, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
}

esp_err_t i2c_manager_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_PORT_NUM, addr, &reg, 1, data, len, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
}
