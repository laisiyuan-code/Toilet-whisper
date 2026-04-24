# Code Explained: How to Build and Flash

This file focuses only on how to build and flash the firmware, with a short explanation of the project structure and what each main source file does.

## What This Project Is

This is an `ESP-IDF` firmware project for an `ESP32-C3` bathroom fan controller using `ESP RainMaker`.

At a high level, the firmware:

- creates a RainMaker node and app-facing controls
- reads sensor and trigger inputs
- decides whether the fan should run
- drives the fan with PWM

## Required Software Setup

This project is currently set up to match this environment:

- `ESP-IDF v5.4.4`
- target `esp32c3`
- project folder: `C:\esp\v5.4.4\fan_controller`
- local RainMaker checkout: `C:\esp\v5.4.4\esp-rainmaker`

The local RainMaker path matters because `main/idf_component.yml` uses local `override_path` entries instead of pulling everything remotely.

## Build the Firmware

Open an `ESP-IDF PowerShell` terminal, or load the environment manually:

```powershell
. C:\esp\v5.4.4\export.ps1
cd C:\esp\v5.4.4\fan_controller
```

Set the target and build:

```powershell
idf.py set-target esp32c3
idf.py build
```

If you want to force a clean rebuild:

```powershell
idf.py fullclean
idf.py set-target esp32c3
idf.py build
```

## Flash the Firmware

Plug in the ESP32-C3 board and find its COM port:

```powershell
Get-CimInstance Win32_SerialPort | Select-Object DeviceID, Name
```

Then flash it, replacing `COM3` with your real port:

```powershell
idf.py -p COM3 flash
```

To flash and immediately open the serial monitor:

```powershell
idf.py -p COM3 flash monitor
```

Useful monitor controls:

- `Ctrl+]` exits the monitor
- `idf.py monitor` opens the monitor again without reflashing

## Typical Build and Flash Flow

```powershell
. C:\esp\v5.4.4\export.ps1
cd C:\esp\v5.4.4\fan_controller
idf.py set-target esp32c3
idf.py build
idf.py -p COM3 flash monitor
```

## Brief Project Structure

### Root files

`CMakeLists.txt`

- top-level ESP-IDF project file
- defines the project name as `fan_controller`

`sdkconfig.defaults`

- default project configuration
- sets the target to `esp32c3`
- enables Wi-Fi, BLE, RainMaker-related options, and the custom partition table

`sdkconfig.defaults.esp32c3`

- extra ESP32-C3-specific defaults

`partitions.csv`

- partition layout used when building and flashing the firmware

`monitor_qr.py`

- small helper script for watching serial output for provisioning or QR-related messages

## `main/` folder

`main/app_main.c`

- main application logic
- starts NVS, networking, RainMaker, sensor handling, and the control task
- defines the RainMaker node, devices, and parameters
- contains the automatic trigger logic
- decides when the fan should run

Main features in this file:

- RainMaker node/device setup
- writable app parameters such as fan power, fan speed, thresholds, and auto mode
- trigger evaluation for presence, light, air quality, and climate
- I2C bus setup for sensors
- periodic control loop

`main/app_driver.c`

- low-level fan output driver
- configures LEDC PWM
- turns the fan on and off
- sets PWM duty based on fan speed percentage

Main features in this file:

- PWM timer setup
- PWM channel setup
- fan power control
- fan speed control

`main/app_priv.h`

- shared definitions used by the app
- stores constants such as GPIO assignments, speed limits, defaults, and related macros

`main/CMakeLists.txt`

- tells ESP-IDF which source files belong to the `main` component
- currently includes:
  - `app_main.c`
  - `app_driver.c`

`main/idf_component.yml`

- declares component dependencies
- requires `ESP-IDF >= 5.4.4`
- links the project to RainMaker and related components

`main/Kconfig.projbuild`

- project menu configuration entries used by `menuconfig`
- contains configurable board-related options

## How the Code Is Organized

The software is split into two main layers:

### 1. Application layer

Handled mostly by `main/app_main.c`.

This layer:

- manages RainMaker
- stores control state
- evaluates triggers
- decides the final fan output

### 2. Hardware driver layer

Handled by `main/app_driver.c`.

This layer:

- talks directly to the ESP32 PWM hardware
- translates a percentage speed into a real PWM duty cycle

## Important Note About Current Features

The project structure for BLE and sensor-based triggers is already in place, but some sensor-reading paths in `main/app_main.c` are still stubbed. So the build and flash steps work, and the project architecture is ready, but some automatic sensing features still need real device-driver code to become fully functional.

## Quick Troubleshooting

If `idf.py build` fails:

- make sure `ESP-IDF 5.4.4` is loaded
- make sure `C:\esp\v5.4.4\esp-rainmaker` exists
- make sure you ran the command from `C:\esp\v5.4.4\fan_controller`

If flashing fails:

- verify the COM port
- close any other serial monitor using the board
- unplug and reconnect the ESP32-C3, then try again

If the monitor opens but behavior looks incomplete:

- the firmware may be running correctly, but some sensor inputs are still placeholder stubs in the current code
