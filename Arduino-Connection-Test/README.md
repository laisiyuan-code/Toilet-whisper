# Connection Test Suite (ESP32 + I2C Sensors + PWM Fan)

This repository contains test sketches to verify wiring and functionality for:

- AHT21 (Temperature & Humidity)
- VEML7700 (Light Sensor)
- ENS160 (Air Quality Sensor)
- PWM Fan Control

---

## ⚡ Quick Start (READ THIS FIRST)

Follow this exact order:

1. Install required libraries
2. Wire all components
3. Run **I2C Scanner**
4. Confirm all devices are detected
5. Test sensors individually
6. Test PWM fan last

> ❗ If the I2C scanner fails, **do not proceed**

---

## ⚙️ Setup

### 📦 Required Libraries

Install via **Arduino IDE → Library Manager**:

| Sensor | Library Name | Author |
|--------|-------------|--------|
| AHT21 | `Adafruit AHTX0` | Adafruit |
| VEML7700 | `SparkFun VEML7700 Arduino Library` | SparkFun |
| ENS160 | `ScioSense ENS160` | ScioSense |
| I2C | `Wire` | Built-in |

---

### 🧭 Installation Steps

1. Open Arduino IDE  
2. Go to **Sketch → Include Library → Manage Libraries**  
3. Search and install each library  

---

### ⚠️ Notes

- `Wire` is pre-installed  
- Use exact library names  
- Compile errors = missing library  

---

## 🔌 Wiring

| Function | GPIO |
|---------|------|
| SDA     | 6    |
| SCL     | 7    |
| Fan PWM | 3    |

All I2C devices must share:
- Common **GND**
- Correct **VCC**
- Same **SDA/SCL lines**

---

## 📡 Step 1 — I2C Scanner (MANDATORY)

**File:** `Test_I2C_Scanner.ino`

### ✅ Pass Condition

You should see:

| Device   | Address |
|----------|--------|
| VEML7700 | 0x10   |
| AHT21    | 0x38   |
| ENS160   | 0x52 / 0x53 |

---

### ❌ Fail Condition

No devices detected → check:

- SDA/SCL swapped
- No power to sensors
- Missing common ground
- Wrong board/port
- Faulty USB cable

---

## 🌡️ Step 2 — AHT21 Test

**File:** `Test_Humidity-Temp_AHTX0.ino`

### ✅ Pass Condition
- Prints temperature and humidity continuously

### ❌ Fail Condition
- "Sensor not detected" → check if `0x38` appeared in scanner

---

## 💡 Step 3 — VEML7700 Test

**File:** `Test_Light-Sensor_VEML7700.ino`

### ✅ Pass Condition
- Lux values update in real-time

### Quick Test
- Cover sensor → value drops  
- Shine light → value increases  

### ❌ Fail Condition
- Not detected → check for `0x10` in scanner

---

## 🌫️ Step 4 — ENS160 Test

**File:** `Test_Air-Quality_ENS160.ino`

### ⚠️ IMPORTANT (Address Setting)

Match the address to your scanner result:

```cpp
ScioSense_ENS160 ens160(0x52); // or 0x53
