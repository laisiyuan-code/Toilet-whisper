#include <Wire.h>
#include <Adafruit_AHTX0.h>

#define SDA_PIN 6
#define SCL_PIN 7

Adafruit_AHTX0 aht;

void setup() {
  Serial.begin(115200);
  delay(1500);
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!aht.begin(&Wire, 0, 0x38)) {
    Serial.println("AHT21 not detected");
    while (1) delay(10);
  }

  Serial.println("AHT21 OK");
}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  Serial.print("Temp C: ");
  //24–27°C → air-conditioned
  //27–30°C → normal indoor
  //30–35°C → hot / poorly ventilated
  Serial.println(temp.temperature, 2);

  Serial.print("Humidity %: ");
  //24–27°C → air-conditioned
  //27–30°C → normal indoor
  //30–35°C → hot / poorly ventilated
  Serial.println(humidity.relative_humidity, 2);

  delay(1000);
}
