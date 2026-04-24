# Toilet Whisper

A smart bathroom fan controller built around an `ESP32-C3`, sensor-triggered automation, and `ESP RainMaker`.

## Why I Built This

Most bathroom fans are either fully manual or run on a basic timer. I wanted something a little smarter: a fan that could react to real bathroom use, stay quiet when not needed, and still be easy to control from a phone.

This project combines sensor monitoring, app control, and a custom enclosure into a single DIY build that other makers can understand, copy, and improve.

## What It Does

- Reads bathroom conditions using sensors
- Lets you manually control the fan from the RainMaker app
- Supports automatic fan control using configurable thresholds
- Stores settings so they survive reboots
- Supports BLE provisioning and OTA updates

## Project Status

This repository contains the full hardware, CAD, test sketches, and firmware work for the project.

Current repo status:
- hardware design is documented
- test sketches are included for bring-up
- ESP-IDF firmware structure is in place
- some auto-sensing paths in the RainMaker firmware may still be under active development

## Build Overview

If you want to recreate this project, this is the easiest order to follow:

1. Review the schematics and parts
2. Run the sensor and PWM checks in `Connection-Test/`
3. Print or inspect the enclosure parts in `CAD-Files/`
4. Assemble the hardware
5. Build and flash the RainMaker firmware
6. Provision the device in the RainMaker app
7. Tune your thresholds and test the final behavior

## Main Features

- `Manual Mode`: Turn the fan on and off and change speed from the app
- `Auto Mode`: Trigger the fan when sensor readings cross a threshold
- `Live Readings`: View sensor values in the app
- `Persistent Settings`: Keep thresholds and mode settings after reboot
- `BLE Provisioning`: Set up Wi-Fi from the mobile app
- `OTA Updates`: Update firmware without replugging the device

## Core Hardware

| Part | Purpose |
|---|---|
| ESP32-C3 DevKitM-1 | Main controller |
| VEML7700 | Ambient light sensor |
| AHT21 | Temperature and humidity sensor |
| ENS160 | Air quality sensor used during testing and hardware bring-up |
| MOSFET fan driver | PWM switching stage |
| Axial fan | Exhaust airflow |
| LM2596 module | Power conversion |
| TP4056 USB-C module | Battery charging |
| 3.7V LiPo battery | Portable power source |

## Pin Mapping

| Function | GPIO |
|---|---|
| SDA | `GPIO6` |
| SCL | `GPIO7` |
| Fan PWM | `GPIO3` |
| Boot Button | `GPIO9` |

## RainMaker Controls

The finished device appears as a fan controller in the ESP RainMaker app.

Available controls and readings:
- power toggle
- speed slider
- auto mode toggle
- lux threshold
- humidity threshold
- temperature threshold
- live temperature reading
- live humidity reading
- live lux reading

## Repo Guide

| Folder | What it contains |
|---|---|
| `Connection-Test/` | Arduino sketches for wiring and sensor validation |
| `Arduino-Connection-Test/` | Legacy documentation folder that points to the test sketches |
| `CAD-Files/` | Fusion 360 archive and printable STL exports |
| `SCHEMATICS & PCB/` | PCB design files and hardware summary |
| `ESP-IDF (RAINMAKER)/` | Main firmware project notes and flashing guide |

## Build Path for New Makers

### Step 1: Validate the electronics
Start with the files in `Connection-Test/`. These are the quickest way to confirm that your sensors, wiring, and fan control stage are behaving correctly before you touch the final firmware.

### Step 2: Review the hardware design
Use `SCHEMATICS & PCB/` to understand the power path, fan control path, and I2C sensor wiring.

### Step 3: Prepare the enclosure
Use `CAD-Files/` if you want to print or modify the case.

### Step 4: Flash the final firmware
Use the instructions in `ESP-IDF (RAINMAKER)/` to build, flash, and provision the device.

## Safety Notes

- Do not drive the fan directly from an ESP32 GPIO
- Double-check voltage rails before connecting the ESP32
- Make sure grounds are shared between the control side and fan side
- Verify your sensor I2C addresses before debugging firmware
- Be careful when working with batteries, charging modules, and power converters

## License

Apache 2.0
