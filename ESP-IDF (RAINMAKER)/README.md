# ESP-IDF Firmware Guide

This folder documents the final firmware build for the Toilet Whisper project.

The firmware is built for an `ESP32-C3` and integrates with `ESP RainMaker` for app control, provisioning, and cloud-connected updates.

## What This Firmware Does

At a high level, the firmware:
- creates a RainMaker device and app-facing controls
- stores settings such as thresholds and operating mode
- reads sensor-related inputs
- controls the fan through PWM
- supports provisioning and remote interaction through RainMaker

## Before You Start

This folder is best used after you have already validated the hardware in `Connection-Test/`.

If your sensors or fan stage are still unverified, do that first.

## Environment Notes

This project was developed against:
- `ESP-IDF v5.4.4`
- target `esp32c3`

If you keep machine-specific paths in your setup, document them clearly, but try to keep build steps portable for other users.

## Build Steps

Open an ESP-IDF terminal and go to your project folder.

```powershell
idf.py set-target esp32c3
idf.py build
