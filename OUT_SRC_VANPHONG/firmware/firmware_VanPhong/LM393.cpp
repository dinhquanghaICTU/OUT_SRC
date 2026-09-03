#include "LM393.h"
#include <Arduino.h>
#include <driver/gpio.h>

void LM393_init() {
  pinMode(LM393_PIN, INPUT);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

float get_soil_moisture_percent() {
  int raw = analogRead(LM393_PIN);

  float percent = map(raw, 4095, 0, 0, 100);
  percent = constrain(percent, 0.0, 100.0);
  return percent;
}
