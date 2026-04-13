# ESP RainMaker Feature Specification
## Intelligent Ventilation IoT Device

## 1. Purpose

This document defines the ESP RainMaker feature set for the intelligent ventilation device based on the ESP32-C3 DevKitM-1.

The device shall:
- operate on 2.4 GHz Wi-Fi
- connect to ESP RainMaker for remote monitoring and control
- use BLE locally for registered-device presence detection
- continue autonomous ventilation logic even if cloud/app access is unavailable

---

## 2. System Overview

The device controls a bathroom ventilation fan using:
- BLE presence detection of registered nearby devices
- sudden bathroom light change detection
- humidity rise detection
- optional air-quality-based boosting from ENS160 readings

The device must support both:
- autonomous local decision making
- user control from the ESP RainMaker app

---

## 3. Wireless Requirements

### 3.1 Wi-Fi
- The device shall use **2.4 GHz Wi-Fi only**
- The firmware shall not assume or require 5 GHz Wi-Fi
- The device shall reconnect automatically after Wi-Fi dropouts
- RainMaker cloud features shall depend on Wi-Fi connectivity, but local autonomous fan control shall continue without Wi-Fi

### 3.2 BLE + Wi-Fi Coexistence
- The ESP32-C3 shall use BLE scanning for presence detection while also maintaining Wi-Fi for RainMaker
- BLE scanning shall be periodic and non-blocking
- Continuous aggressive scanning shall be avoided
- BLE scan interval, scan window, and scan burst period shall be configurable
- Presence detection shall use debounce and hold timers to reduce scan load and prevent fan chatter
- Wi-Fi stability, RainMaker responsiveness, and local control responsiveness take priority over aggressive BLE scanning

---

## 4. RainMaker Device Model

### 4.1 Primary Device
Use a primary **Fan** device in RainMaker.

### 4.2 Services
The firmware should include:
- ESP RainMaker core node
- OTA service
- Time service
- optional Schedule service if integration remains clean and maintainable

---

## 5. User Controls in RainMaker

### 5.1 Core Controls
The app shall expose:
- Power
- Mode

### 5.2 Mode Options
The supported fan modes shall be:
- OFF
- AUTO
- LOW
- MEDIUM
- HIGH

### 5.3 Trigger Enable Controls
The app shall allow enabling or disabling:
- BLE trigger enable
- light trigger enable
- humidity trigger enable
- optional air-quality trigger enable

### 5.4 Threshold Controls
The app shall allow configuration of:
- BLE RSSI threshold
- light delta threshold
- humidity delta threshold
- optional absolute humidity threshold
- fan hold-on time
- minimum fan duty
- maximum fan duty
- kick-start enable
- kick-start duration

---

## 6. Telemetry in RainMaker

The app shall show read-only telemetry for:
- lux
- temperature
- humidity
- TVOC
- eCO2
- AQI
- BLE present state
- strongest detected RSSI
- active trigger states
- fan state
- fan duty
- manual override state
- Wi-Fi connected state
- RainMaker connected state

---

## 7. App-Control Behavior

### 7.1 OFF Mode
- Fan remains off
- Optional safety override for extreme air-quality events may be supported if explicitly enabled

### 7.2 LOW / MEDIUM / HIGH Modes
- Fan runs at fixed configured duty
- Automatic trigger logic does not change fan speed while manual fixed mode is active

### 7.3 AUTO Mode
- Firmware decides fan operation using BLE, light, humidity, and optional air quality logic
- Hysteresis and hold timers shall be used to avoid rapid toggling

---

## 8. Autonomous Logic Expectations

The following intended logic shall guide implementation:
- BLE presence plus recent light change indicates likely occupancy
- humidity rise indicates ventilation demand
- poor air quality indicates ventilation demand
- light change alone may trigger a short ventilation cycle if configured
- any logic thresholds must be easy to tune in a central configuration layer

---

## 9. Logging Requirements

The firmware shall log periodic time-series data including:
- timestamp
- uptime seconds
- lux
- temperature
- humidity
- TVOC
- eCO2
- AQI
- BLE present
- strongest RSSI
- trigger states
- fan mode
- fan state
- fan duty
- Wi-Fi state
- RainMaker state

The firmware shall also log events including:
- BLE device detected/lost
- light trigger asserted/cleared
- humidity trigger asserted/cleared
- air-quality trigger asserted/cleared
- fan turned on/off
- mode changed from app
- threshold changed from app
- Wi-Fi connected/disconnected
- RainMaker connected/disconnected
- sensor fault

---

## 10. Daily Summary Metrics

The firmware should maintain daily summary values such as:
- total fan runtime
- number of fan activations
- max humidity
- average humidity
- peak TVOC
- peak eCO2
- number of BLE detections
- highest activity period

---

## 11. Persistence Requirements

The following values should persist across reboots using NVS:
- current mode
- threshold values
- trigger enable states
- fan tuning settings
- optional daily summary counters where practical

---

## 12. Fail-Safe Behavior

- Local autonomous control must continue even if RainMaker is disconnected
- Sensor read failures shall not crash the system
- If one sensor fails, remaining triggers shall continue operating
- Firmware should emit warnings and degraded-state telemetry where possible

---

## 13. Recommended Parameter Grouping in Firmware

Suggested RainMaker parameter groups:
- control parameters
- trigger enable parameters
- threshold parameters
- telemetry parameters
- status parameters
- daily summary parameters

---

## 14. Calibration Notes

The following values will require real-world bathroom testing:
- BLE RSSI threshold
- BLE scan interval/window/burst period
- lux delta threshold
- humidity delta threshold
- fan hold-on time
- minimum reliable PWM duty
- kick-start duration
- AUTO mode decision rules

---

## 15. Future Expansion

The firmware architecture should remain expandable for:
- MQTT
- local web dashboard
- richer analytics export
- OTA improvements
- scheduling enhancements
- SD card or larger local storage
