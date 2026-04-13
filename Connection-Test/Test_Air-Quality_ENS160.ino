#include <Wire.h>
#include <ScioSense_ENS160.h>

#define SDA_PIN 6
#define SCL_PIN 7

// Try 0x52 first. If your scanner showed 0x53, change it.
ScioSense_ENS160 ens160(0x52);

void setup() {
  Serial.begin(115200);
  delay(1500);
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!ens160.begin()) {
    Serial.println("ENS160 not detected");
    while (1) delay(10);
  }

  ens160.setMode(ENS160_OPMODE_STD);
  Serial.println("ENS160 OK");
}

void loop() {
  if (ens160.available()) {
    ens160.measure();

    Serial.print("Air-Quality-Index: "); 
    //1:Excellent to 5:Unhealthy
    Serial.println(ens160.getAQI());

    Serial.print("Total-Volatile-Organic-Compounds: "); 
    //0–50 ppb    → very clean
    //50–200 ppb  → normal indoor
    //200–500 ppb → noticeable odor
    //500+ ppb    → strong / unpleasant
    Serial.println(ens160.getTVOC());

    Serial.print("Equivalent CO₂: ");
    //400 ppm    → fresh air
    //600–800    → normal indoor
    //1000+      → stuffy
    //1500+      → poor ventilation    
    Serial.println(ens160.geteCO2());
  } else {
    Serial.println("ENS160 data not ready");
  }

  delay(1000);
}
