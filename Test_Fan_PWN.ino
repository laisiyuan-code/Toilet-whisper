#include <Arduino.h>

#define FAN_PWM_PIN 3

const uint32_t PWM_FREQ_HZ = 25000;
const uint8_t PWM_RESOLUTION = 8;

void setFan(uint8_t duty, const char* label) {
  ledcWrite(FAN_PWM_PIN, duty);
  Serial.print(label);
  Serial.print(" | duty=");
  Serial.println(duty);
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  if (!ledcAttach(FAN_PWM_PIN, PWM_FREQ_HZ, PWM_RESOLUTION)) {
    Serial.println("PWM attach failed");
    while (true) delay(1000);
  }

  setFan(0, "OFF");
}

void loop() {
  setFan(0,   "OFF");
  delay(3000);

  setFan(64,  "LOW");
  delay(5000);

  setFan(128, "MED");
  delay(5000);

  setFan(255, "HIGH");
  delay(5000);
}
