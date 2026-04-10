#include <Wire.h>

#define I2C_SDA 6
#define I2C_SCL 7

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println();
  Serial.println("I2C scanner starting...");
}

void loop() {
  byte count = 0;

  Serial.println("Scanning...");
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Found device at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      count++;
    }
  }

  if (count == 0) {
    Serial.println("No I2C devices found.");
  } else {
    Serial.print("Total devices found: ");
    Serial.println(count);
  }

  Serial.println("----------------------");
  delay(3000);
}


//Serial monitor output for success

//0x10 for VEML7700
//0x38 for AHT2x
//0x52 or 0x53 for ENS160
