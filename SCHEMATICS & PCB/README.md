# PCB and Schematic Summary

This board is a compact `ESP32-C3` fan controller designed around three main sections:

- `12V` fan power input
- low-voltage control and battery/power management
- sensor and control connections to the `ESP32-C3 DevKitM-1`

## Main Idea

The schematic separates high-power fan wiring from low-voltage logic wiring. That keeps the control side cleaner and makes the board easier to route and debug.

## Main Blocks

### 1. Fan Power

- `VIN` and `VIN_GND` bring `12V` onto the board
- the fan is powered from the `12V` side
- the fan control stage receives a `TRIG/PWM` signal from the ESP32

### 2. Power Conversion

- an `LM2596` buck converter steps the input voltage down for the controller side
- a `TP4056` charger module and `3.7V` battery section provide the battery/power-management part of the design

### 3. ESP32-C3 Control

The `ESP32-C3 DevKitM-1` is the center of the board and handles:

- fan control
- I2C sensor communication
- firmware logic and RainMaker connectivity

## Important Pin Mapping

- `GPIO3` -> fan `TRIG/PWM`
- `GPIO6` -> `SDA`
- `GPIO7` -> `SCL`

These match the firmware implementation in:

- [main/app_priv.h](/C:/esp/v5.4.4/fan_controller/main/app_priv.h:16)
- [main/app_driver.c](/C:/esp/v5.4.4/fan_controller/main/app_driver.c:25)
- [main/app_main.c](/C:/esp/v5.4.4/fan_controller/main/app_main.c:352)

## Sensor Connections

The board uses one shared I2C bus for the sensors:

- `VEML7700` light sensor
- `ENS160` air-quality sensor
- `AHT21` temperature/humidity sensor

This keeps the wiring simple and reduces the number of GPIOs used.

## PCB Layout Strategy

The PCB is arranged by function:

- fan and `12V` routing on one side
- `ESP32-C3` in the center
- sensor connections on the other side
- power modules near the bottom

This helps keep:

- high-current traces short and wide
- sensor traces cleaner
- the board easier to understand visually

## What a New User Should Know

- do not drive the fan directly from an ESP32 pin
- make sure the buck converter output is correct before powering the ESP32
- keep grounds shared between the control and fan sections
- confirm your sensor modules use the expected I2C addresses

## In One Sentence

This PCB was designed by separating `12V` fan power, low-voltage control, and shared I2C sensing around a central `ESP32-C3` so the hardware layout matches the firmware cleanly.
