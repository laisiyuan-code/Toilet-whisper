# Connection Test Guide

Use this folder to verify your hardware before moving on to the full ESP-IDF firmware.

This is the best starting point if you are building the project yourself.

## What This Folder Is For

These Arduino sketches help you confirm:
- your ESP32-C3 is detected correctly
- your I2C sensors are wired correctly
- your sensors return sensible data
- your fan PWM stage responds as expected

## Files in This Folder

| File | Purpose |
|---|---|
| `Test_I2C_Scanner.ino` | Confirms which I2C devices are visible |
| `Test_Humidity-Temp_AHTX0.ino` | Tests the AHT21 |
| `Test_Light-Sensor_VEML7700.ino` | Tests the VEML7700 |
| `Test_Air-Quality_ENS160.ino` | Tests the ENS160 |
| `Test_Fan_PWN.ino` | Tests PWM fan control |

## Recommended Order

Follow these steps in order:

1. Run the I2C scanner
2. Test the AHT21
3. Test the VEML7700
4. Test the ENS160
5. Test the fan PWM output last

If the scanner does not detect your devices, stop there and fix wiring first.

## Basic Wiring

| Function | GPIO |
|---|---|
| SDA | `GPIO6` |
| SCL | `GPIO7` |
| Fan PWM | `GPIO3` |

Important wiring rules:
- all I2C devices must share the same `SDA` and `SCL`
- all modules must share a common `GND`
- power each sensor with the correct voltage for that module
- do not connect the fan directly to the ESP32 pin

## Required Arduino Libraries

Install these from the Arduino IDE Library Manager:

| Sensor | Library |
|---|---|
| AHT21 | `Adafruit AHTX0` |
| VEML7700 | `SparkFun VEML7700 Arduino Library` |
| ENS160 | `ScioSense ENS160` |
| I2C | `Wire` |

## Step 1: Run the I2C Scanner

Open `Test_I2C_Scanner.ino` and upload it first.

Expected addresses:
- `VEML7700` at `0x10`
- `AHT21` at `0x38`
- `ENS160` at `0x52` or `0x53`

If no devices appear:
- check SDA and SCL are not swapped
- check the sensor power rails
- check common ground
- confirm the correct board and port are selected
- try another USB cable

## Step 2: Test the AHT21

Open `Test_Humidity-Temp_AHTX0.ino`.

Success looks like:
- repeated temperature output
- repeated humidity output
- values that change slowly with room conditions

If it fails:
- make sure `0x38` showed up in the scanner
- recheck the library install
- recheck sensor power and wiring

## Step 3: Test the VEML7700

Open `Test_Light-Sensor_VEML7700.ino`.

Success looks like:
- lux values updating in real time
- the reading drops when you cover the sensor
- the reading rises when you shine light on it

If it fails:
- make sure `0x10` showed up in the scanner
- confirm the correct library is installed

## Step 4: Test the ENS160

Open `Test_Air-Quality_ENS160.ino`.

Before uploading, match the constructor address to your scanner result:

```cpp
ScioSense_ENS160 ens160(0x52); // or 0x53
