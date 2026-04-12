# intelligent_ventilation_iot

ESP-IDF firmware for an ESP32-C3 DevKitM-1 based intelligent bathroom ventilation controller. The firmware supports fully autonomous offline operation and optionally layers ESP RainMaker cloud/app control and telemetry on top when Wi-Fi is available.

## Features

- Autonomous fan control using BLE presence, light change, humidity rise, and air-quality boost logic
- Manual and automatic operating modes: `OFF`, `AUTO`, `LOW`, `MEDIUM`, `HIGH`
- ENS160 + AHT21 environmental sensing and VEML7700 ambient light sensing over I2C
- PWM-controlled fan output with minimum/maximum duty limits and kick-start logic
- Periodic BLE scan bursts with debounce and hold timers to preserve Wi-Fi responsiveness
- CSV telemetry logging plus event logging to SPIFFS
- Daily summary persistence in NVS
- ESP RainMaker integration with OTA and Time services when RainMaker is available in the SDK

## Hardware Mapping

- `GPIO6` -> I2C SDA
- `GPIO7` -> I2C SCL
- `GPIO3` -> Fan PWM (LEDC)

## Build

```bash
idf.py set-target esp32c3
idf.py build
idf.py flash monitor
```

## Notes About RainMaker

`main/rainmaker_app.c` uses `__has_include()` guards. If the ESP RainMaker component is available in your ESP-IDF environment, the project builds with cloud support. If it is not available, the same project still builds and runs fully offline with a stubbed RainMaker layer.

## Runtime Behavior

- Offline autonomy is always available
- RainMaker app callbacks only update configuration and requested mode
- The decision engine remains the single authority that converts state into fan actions
- BLE scanning runs in periodic bursts instead of continuously so Wi-Fi and RainMaker remain the priority radios

## Files

```text
intelligent_ventilation_iot/
|-- CMakeLists.txt
|-- partitions.csv
|-- README.md
|-- sdkconfig.defaults
`-- main/
    |-- CMakeLists.txt
    |-- app_main.h
    |-- config.h
    |-- main.c
    |-- i2c_manager.c
    |-- i2c_manager.h
    |-- veml7700.c
    |-- veml7700.h
    |-- air_quality.c
    |-- air_quality.h
    |-- ble_presence.c
    |-- ble_presence.h
    |-- fan_control.c
    |-- fan_control.h
    |-- decision_engine.c
    |-- decision_engine.h
    |-- rainmaker_app.c
    |-- rainmaker_app.h
    |-- data_logger.c
    `-- data_logger.h
```

## Calibration Notes

### BLE RSSI threshold

Start around `-75 dBm` to `-65 dBm`. In bathrooms with walls or doors between the device and the phone, a less strict threshold may work better. Raise the threshold if false positives occur outside the room.

### Lux delta

Use the difference between the rolling baseline and the current value. Start with a threshold around `20-80 lux` depending on how bright the room is when lights are on and off.

### Humidity delta

For shower detection, start with a rise threshold around `4-8 %RH` above baseline and optionally use an absolute humidity threshold such as `70 %RH` for extra certainty.

### Fan duty tuning

Because the fan is a 24 V model operated from 12 V, start with:

- minimum duty: `40-50 %`
- medium duty: `65-75 %`
- high duty: `85-100 %`
- kick-start: enabled, `1200-2000 ms`

Tune until the fan starts reliably every time without stalling at low duty.

### BLE scan parameters

Use short scan bursts such as:

- scan interval: `160 ms`
- scan window: `80 ms`
- burst duration: `4 s`
- idle between bursts: `20-30 s`
- presence hold time: `60-180 s`

If Wi-Fi responsiveness suffers, reduce burst duration or increase the idle gap between bursts.
