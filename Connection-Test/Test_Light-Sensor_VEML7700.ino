#include <Wire.h>
#include <SparkFun_VEML7700_Arduino_Library.h>

#define SDA_PIN 6
#define SCL_PIN 7

VEML7700 lightSensor;

void setup() {
  Serial.begin(115200);
  delay(1500);
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!lightSensor.begin()) {
    Serial.println("VEML7700 not detected");
    while (1) delay(10);
  }

  Serial.println("VEML7700 OK");
}

void loop() {
  Serial.print("Lux: ");
  //400 ppm → fresh air
  //600–800 → normal indoor
  //1000+   → stuffy
  //1500+   → poor ventilation
  Serial.println(lightSensor.getLux(), 2);
  delay(1000);
}
