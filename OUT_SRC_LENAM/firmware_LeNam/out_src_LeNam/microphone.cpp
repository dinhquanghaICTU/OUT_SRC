#include "microphone.h"
#include <driver/gpio.h>
#include <Arduino.h>



void init_microphone(){
    pinMode(MICROPHONE_PIN, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(MICROPHONE_PIN, ADC_11db);
}

float get_microphone(){

    const unsigned long startedMs = millis();

    int minimumRaw = 4095;
    int maximumRaw = 0;

    while (millis() - startedMs < SAMPLE_WINDOW_MS)
    {
        const int raw = analogRead(MICROPHONE_PIN);

        if (raw < minimumRaw)
            minimumRaw = raw;

        if (raw > maximumRaw)
            maximumRaw = raw;
    }

    const int peakToPeakRaw = maximumRaw - minimumRaw;
    const float amplitudeVoltage =
        peakToPeakRaw * 3.3F / 4095.0F;

    Serial.printf(
        "[MIC] min=%d max=%d amplitude=%d raw, %.3f Vpp\n",
        minimumRaw,
        maximumRaw,
        peakToPeakRaw,
        amplitudeVoltage
    );

    return amplitudeVoltage;
}
