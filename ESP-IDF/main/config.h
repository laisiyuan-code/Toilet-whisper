#pragma once

#include "driver/gpio.h"

#define APP_NAME "intelligent_ventilation_iot"

#define I2C_PORT_NUM            0
#define I2C_SDA_GPIO            GPIO_NUM_6
#define I2C_SCL_GPIO            GPIO_NUM_7
#define I2C_CLK_SPEED_HZ        400000
#define I2C_TIMEOUT_MS          100

#define FAN_PWM_GPIO            GPIO_NUM_3
#define FAN_PWM_TIMER           LEDC_TIMER_0
#define FAN_PWM_MODE            LEDC_LOW_SPEED_MODE
#define FAN_PWM_CHANNEL         LEDC_CHANNEL_0
#define FAN_PWM_FREQUENCY_HZ    25000
#define FAN_PWM_RESOLUTION      LEDC_TIMER_10_BIT

#define DEFAULT_SENSOR_PERIOD_MS            5000
#define DEFAULT_STATUS_PERIOD_MS            10000
#define DEFAULT_DECISION_PERIOD_MS          1000
#define DEFAULT_LOG_PERIOD_MS               60000
#define DEFAULT_BASELINE_WINDOW             24

#define DEFAULT_BLE_ENABLE                  true
#define DEFAULT_LIGHT_ENABLE                true
#define DEFAULT_HUMIDITY_ENABLE             true
#define DEFAULT_AIR_ENABLE                  true

#define DEFAULT_BLE_RSSI_THRESHOLD_DBM      -72
#define DEFAULT_LIGHT_DELTA_THRESHOLD_LUX   35.0f
#define DEFAULT_HUMIDITY_DELTA_THRESHOLD    6.0f
#define DEFAULT_HUMIDITY_ABS_THRESHOLD      72.0f
#define DEFAULT_TVOC_THRESHOLD_PPB          400
#define DEFAULT_ECO2_THRESHOLD_PPM          1200
#define DEFAULT_AQI_THRESHOLD               3

#define DEFAULT_FAN_HOLD_TIME_S             180
#define DEFAULT_LIGHT_ONLY_RUN_TIME_S       90
#define DEFAULT_PRESENCE_HOLD_TIME_S        90
#define DEFAULT_START_HOLD_TIME_S           20

#define DEFAULT_MIN_DUTY_PERCENT            45
#define DEFAULT_LOW_DUTY_PERCENT            55
#define DEFAULT_MED_DUTY_PERCENT            72
#define DEFAULT_HIGH_DUTY_PERCENT           100
#define DEFAULT_MAX_DUTY_PERCENT            100
#define DEFAULT_KICKSTART_ENABLE            true
#define DEFAULT_KICKSTART_DURATION_MS       1500

#define DEFAULT_BLE_SCAN_INTERVAL_MS        160
#define DEFAULT_BLE_SCAN_WINDOW_MS          80
#define DEFAULT_BLE_SCAN_BURST_MS           4000
#define DEFAULT_BLE_SCAN_IDLE_MS            25000
#define DEFAULT_BLE_PRESENCE_DEBOUNCE_COUNT 2

#define DEFAULT_RAINMAKER_TELEMETRY_MS      10000

#define LOGGER_MOUNT_POINT                  "/spiffs"
#define LOGGER_CSV_PATH                     LOGGER_MOUNT_POINT "/telemetry.csv"
#define LOGGER_EVENT_PATH                   LOGGER_MOUNT_POINT "/events.log"

#define NVS_NAMESPACE_CONFIG                "vent_cfg"
#define NVS_NAMESPACE_SUMMARY               "vent_daily"

#define BLE_PHONE_1_NAME "Phone_1"
#define BLE_PHONE_1_ADDR "AA:BB:CC:11:22:33"
#define BLE_PHONE_2_NAME "Phone_2"
#define BLE_PHONE_2_ADDR "11:22:33:44:55:66"
