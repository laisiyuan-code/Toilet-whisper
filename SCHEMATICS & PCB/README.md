# Schematics and PCB

This folder contains the board-level design summary for the Toilet Whisper hardware.

It is meant to help builders understand how power, control, sensing, and fan driving are connected before they assemble or debug the system.

## What This Hardware Does

The board centers around an `ESP32-C3 DevKitM-1` and connects:
- fan power and fan switching
- low-voltage control electronics
- shared I2C sensors
- battery and charging hardware

## Main Hardware Blocks

### Fan Power
- `VIN` and `VIN_GND` bring power into the board
- the fan is powered from the higher-voltage side
- the fan control stage receives a PWM trigger from the ESP32

### Power Conversion
- an `LM2596` module handles voltage conversion
- a `TP4056` module handles battery charging
- the battery section supports portable power operation

### ESP32-C3 Control
The `ESP32-C3 DevKitM-1` handles:
- fan control
- sensor communication
- firmware logic
- RainMaker connectivity

## Important Pin Mapping

| Function | GPIO |
|---|---|
| Fan PWM | `GPIO3` |
| SDA | `GPIO6` |
| SCL | `GPIO7` |

These mappings should match the firmware constants in the ESP-IDF project.

## Sensor Connections

The board uses a shared I2C bus for:
- `VEML7700`
- `AHT21`
- `ENS160`

This keeps wiring simple and reduces GPIO usage.

## PCB Layout Intent

The board is arranged by function so it is easier to read and debug:
- fan and power routing on one side
- controller in the center
- sensor connections grouped together
- power modules placed where wiring is manageable

This layout helps keep:
- high-current traces shorter
- logic and sensing traces cleaner
- the design easier to understand visually

## Bring-Up Checklist

Before powering the full system:
- verify the voltage output of the power stage
- confirm grounds are shared correctly
- confirm the fan is connected through the driver stage
- confirm sensor modules are on the correct I2C lines
- confirm there are no shorts before installing the ESP32

## Safety Notes

- do not connect the fan directly to an ESP32 pin
- verify converter output before powering the controller
- be careful when working with batteries and charging modules
- disconnect power before rewiring

