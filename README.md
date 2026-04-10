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

## 🧠 Engineering Approach

A **modular validation strategy** was used:

1. Verify I2C communication (foundation layer)
2. Test each sensor independently
3. Validate actuator separately

### Benefits
- Isolates faults effectively
- Reduces debugging complexity during integration
- Ensures each subsystem is functional before combining

---

## ⚠️ Observations

- I2C scanner works reliably across all devices
- Individual sensor tests pass successfully
- Combined sensor test previously failed despite individual success

### Possible Causes (for future debugging)
- I2C address conflicts
- Library incompatibility
- Initialization timing issues
- Blocking delays in code

---

## 🚀 Next Step

Proceed to **incremental system integration**:
1. Combine ENS160 + AHT21
2. Add VEML7700
3. Integrate fan control logic

---
