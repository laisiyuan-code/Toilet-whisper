# Toilet-Whisper

Smart Home IOT ventilator for your toilet needs

An ESP32-C3-based intelligent bathroom ventilation controller built on **ESP RainMaker**. Monitors ambient light, temperature, and humidity via I2C sensors and autonomously controls an exhaust fan through PWM.

## Features

- **Manual Mode** -- Control fan speed (0-100%) via the RainMaker app slider
- **Auto Mode** -- Fan turns on/off automatically based on sensor thresholds:
  - **Lux** -- Light level above threshold (bathroom light is on)
  - **Humidity** -- Humidity above threshold (shower/bath detected)
  - **Temperature** -- Temperature above threshold
- **Live Sensor Dashboard** -- Real-time Temperature, Humidity, and Lux readings in the app
- **Persistent Settings** -- Thresholds and auto mode survive reboots (NVS)
- **OTA Updates** -- Over-the-air firmware updates via RainMaker cloud
- **BLE Provisioning** -- Easy Wi-Fi setup via the RainMaker mobile app

## Hardware

### Components

| Component | Role |
|---|---|
| ESP32-C3 DevKitM-1 | Main controller (Wi-Fi, I2C, PWM) |
| VEML7700 | Ambient light sensor (I2C) |
| AHT21 | Temperature & humidity sensor (I2C) |
| 4028 Axial Fan (24V) | Exhaust fan |
| MOD-MOS-008 MOSFET | PWM fan driver |
| LM2596 Buck/Boost | 3.7V to 12V step-up |
| TP4056 USB-C Module | Battery charging |
| 1200 mAh 3.7V LiPo | Power source |

### Pin Mapping

| Function | GPIO | Notes |
|---|---|---|
| I2C SDA | GPIO 6 | Shared bus for VEML7700 + AHT21 |
| I2C SCL | GPIO 7 | 400 kHz |
| Fan PWM | GPIO 3 | 25 kHz, 10-bit resolution |
| Boot Button | GPIO 9 | Toggle fan / Wi-Fi reset (3s) / Factory reset (10s) |

### I2C Devices

| Sensor | Address | Measurements |
|---|---|---|
| VEML7700 | `0x10` | Ambient light (lux) |
| AHT21 | `0x38` | Temperature (C), Relative humidity (%) |

### Schematic

![Schematic](docs/schematic.jpg)

### PCB Layout

![PCB](docs/pcb.jpg)

## RainMaker App

The device appears as a **Fan** device in the ESP RainMaker app with the following controls:

| Parameter | Type | Description |
|---|---|---|
| Power | Toggle | Fan on/off |
| Speed | Slider (0-100) | Fan speed percentage |
| Temperature | Read-only | Current temperature (C) |
| Humidity | Read-only | Current relative humidity (%) |
| Lux | Read-only | Current ambient light (lux) |
| Auto Mode | Toggle | Enable/disable automatic fan control |
| Lux Threshold | Slider (5-200) | Lux level to trigger fan |
| Humidity Threshold | Slider (2-200) | Humidity level to trigger fan |
| Temp Threshold | Slider (1-40) | Temperature level to trigger fan |

### App Screenshots

<p align="center">
  <img src="docs/app_controls.jpg" width="300" alt="Fan controls and sensor readings"/>
  &nbsp;&nbsp;
  <img src="docs/app_thresholds.jpg" width="300" alt="Auto mode thresholds"/>
</p>

### Demo Videos

**Lux trigger** -- Fan turns on when light exceeds threshold:

https://github.com/laisiyuan-code/Toilet-whisper/raw/master/ESP-IDF/esp-rainmaker/examples/sy_dup/docs/lux_turn_on.MOV

**Temperature trigger** -- Fan turns on when temperature exceeds threshold:

https://github.com/laisiyuan-code/Toilet-whisper/raw/master/ESP-IDF/esp-rainmaker/examples/sy_dup/docs/temperature_turn_on.MOV

## Auto Mode Logic

When **Auto Mode** is enabled, the fan is fully sensor-driven:

```
IF lux > lux_threshold  OR  humidity > humidity_threshold  OR  temp > temp_threshold
    -> Fan ON (at current speed setting, default 70%)
ELSE
    -> Fan OFF
```

- Any **one** sensor exceeding its threshold keeps the fan running
- Fan turns off immediately when **all** values drop below their thresholds
- Thresholds are persisted in NVS and survive reboots
- Button press or app toggle still works as manual override

When **Auto Mode** is disabled, the fan is controlled manually via the Power toggle and Speed slider.

## Build & Flash

### Prerequisites

- [ESP-IDF v5.3](https://docs.espressif.com/projects/esp-idf/en/v5.3/)
- [ESP RainMaker](https://rainmaker.espressif.com/)

### Steps

```bash
# Set target
idf.py set-target esp32c3

# Build
idf.py build

# Flash and monitor
idf.py -p COMx flash monitor
```

### First-Time Setup

1. Flash the firmware
2. Open the **ESP RainMaker** app ([iOS](https://apps.apple.com/app/esp-rainmaker/id1497491540) / [Android](https://play.google.com/store/apps/details?id=com.espressif.rainmaker))
3. Scan the QR code shown in the serial monitor
4. Follow the provisioning flow to connect to Wi-Fi

### Reset

| Action | How |
|---|---|
| Toggle fan | Single press boot button |
| Wi-Fi reset | Hold boot button 3 seconds |
| Factory reset | Hold boot button 10 seconds |

## Project Structure

```
main/
  app_priv.h          # Hardware config, constants, API declarations
  app_driver.c        # I2C sensors, PWM fan, auto-trigger logic
  app_main.c          # RainMaker device setup, cloud callbacks
  CMakeLists.txt      # Build config
  Kconfig.projbuild   # Menuconfig options
```

## License

Apache 2.0
