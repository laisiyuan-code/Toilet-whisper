# 🧪 Component Validation – I2C & Individual Sensor Testing

## 📌 Objective
To verify correct wiring, communication, and functionality of all sensors and actuators before system integration.

---

## 🔍 Step 1: I2C Bus Verification

### Test File
`Test_I2C_Scanner.ino`

### Purpose
To confirm:
- Proper wiring of SDA and SCL lines
- Detection of all I2C devices
- Correct device addresses

### Procedure
1. Upload I2C scanner code to ESP32-S3
2. Open Serial Monitor
3. Scan for connected I2C devices

### Result
- I2C scanner successfully detected connected devices
- Confirms:
  - SDA/SCL wiring is correct
  - Sensors are powered and responsive
  - I2C communication is functional

### Insight
This step validates the **entire I2C communication layer**, ensuring that any future issues are likely software-related rather than wiring-related.

---

## 🔧 Step 2: Individual Sensor Testing

After confirming I2C functionality, each sensor was tested independently to isolate issues and verify correct operation.

---

### 🌫️ ENS160 Air Quality Sensor (+ AHT21)

**Test File:**  
`Test_Air_Quality_ENS160.ino`

**Parameters Tested:**
- AQI (Air Quality Index)
- TVOC (Total Volatile Organic Compounds)
- eCO₂ (Equivalent CO₂)

**Result:**
- Sensor initialized successfully
- Data readings obtained from ENS160

---

### 🌡️ Temperature & Humidity Sensor (AHT21)

**Test File:**  
`Test_Humidity-Temp_AHTX0.ino`

**Parameters Tested:**
- Temperature
- Relative Humidity

**Result:**
- Stable readings observed
- Sensor communication verified

---

### 💡 Light Sensor (VEML7700)

**Test File:**  
`Test_Light-Sensor_VEML7700.ino`

**Parameters Tested:**
- Ambient Light (Lux)

**Result:**
- Sensor responded correctly
- Light intensity readings obtained

---

### 🌀 Fan PWM Control

**Test File:**  
`Test_Fan_PWN.ino`

**Purpose:**
- Validate PWM signal output from ESP32-S3
- Confirm fan response to varying duty cycles

**Result:**
- Fan responds to PWM input
- Speed variation observed with duty cycle changes

---

### Bill of Materials

| Category             | Component             | Description                           | Qty | Notes                  |
| -------------------- | --------------------- | ------------------------------------- | --- | ---------------------- |
| MCU                  | ESP32-C3 DevkitM-1    | Main controller (BLE, WiFi, I2C, PWM) | 1   |                        |
| Light Sensor         | VEML7700 Module       | Ambient light (lux measurement)       | 1   | I2C (0x10)             |
| Air Quality          | ENS160 + AHT21        | TVOC, eCO2, temp, humidity            | 1   | I2C (0x52 + 0x38)      |
| DC Fan               | 24V 4028 Axial Fan    | Exhaust airflow                       | 1   | High pressure, compact |
| MOSFET DRIVER MODULE | MOD-MO$-008           | Fan switching (PWM)                   | 1   | BA7U2D + 9A7U2D        |
| Power Supply         | 1600 mAH 3.7V LiPO    | Main power source                     | 1   |                        |
| DC-DC Step Up Boost  | LM2596 Buck Converter | 3.7V → 12V                            | 1   |                        |
| Charging module      | TP4056 TYPE-C         | BMS For managing power supply         | 1   |                        |
