#include "LM35.h"

#include <driver/gpio.h>
#include <Arduino.h>




// const float V_REF = 5.0;     
// const float R_BITS = 12;
// const float ADC_STEPS = (1 << int(R_BITS)) - 1; 



void init_lm35(){
    pinMode(LM35_PIN, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(LM35_PIN, ADC_0db);
}


float readLm35Voltage()
{
    uint32_t totalMillivolts = 0;

    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        totalMillivolts += analogReadMilliVolts(LM35_PIN);
        delay(5);
    }

    const float averageMillivolts =
        (float)totalMillivolts / SAMPLE_COUNT;

    return averageMillivolts / 1000.0F;
}

float readLm35Temperature()
{
    const float voltage = readLm35Voltage();

    return voltage * 100.0F;
}



