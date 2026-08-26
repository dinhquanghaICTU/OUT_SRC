#include "UV.h"
#include <driver/gpio.h>
#include <Arduino.h>
#include <math.h>

const int SAMPLES_PER_READ = 16; // so mau lay trung binh
const float ADC_VREF = 3.3f;
const int ADC_RESOLUTION = 4095; // 12-bit

void init_uv(void)
{
    analogReadResolution(12);
    analogSetPinAttenuation(SIG_PIN, ADC_11db);
}

// int getdata_uv()
// {
//     const int raw = analogRead(SIG_PIN);

//     const uint32_t millivolts = analogReadMilliVolts(SIG_PIN);

//     Serial.printf("raw=%d, voltage=%d mV\n", raw, millivolts);
//     return millivolts;
// }

float readUvVoltage()
{
    long sum = 0;
    for (int i = 0; i < SAMPLES_PER_READ; i++)
    {
        sum += analogRead(SIG_PIN);
        delay(2);
    }
    float avgRaw = (float)sum / SAMPLES_PER_READ;
    return (avgRaw / ADC_RESOLUTION) * ADC_VREF;
}

float voltageToUvIndex(float voltage)
{
    struct Point
    {
        float v;
        float idx;
    };
    static const Point table[] = {{0.00f, 0},
                                  {0.43f, 0},
                                  {0.49f, 1},
                                  {0.55f, 2},
                                  {0.62f, 3},
                                  {0.68f, 4},
                                  {0.74f, 5},
                                  {0.80f, 6},
                                  {0.86f, 7},
                                  {0.93f, 8},
                                  {0.99f, 9},
                                  {1.05f, 10}};
    const int n = sizeof(table) / sizeof(table[0]);

    if (voltage <= table[0].v)
        return table[0].idx;
    if (voltage >= table[n - 1].v)
        return table[n - 1].idx;

    for (int i = 0; i < n - 1; i++)
    {
        if (voltage >= table[i].v && voltage <= table[i + 1].v)
        {
            float ratio = (voltage - table[i].v) / (table[i + 1].v - table[i].v);
            return table[i].idx + ratio * (table[i + 1].idx - table[i].idx);
        }
    }
    return table[n - 1].idx;
}

// Mo phong chu ky buc xa mat troi khi chua gan cam bien vat ly
float generateSolarUvVoltage()
{
    const float DAY_PERIOD_MS = 2 * 60 * 1000.0f;

    float t = fmodf(millis(), DAY_PERIOD_MS) / DAY_PERIOD_MS;    
    float sunCurve = sinf((float)M_PI * t);
    if (sunCurve < 0) sunCurve = 0;
    const float V_MAX = 1.05f;
    float baseVoltage = sunCurve * V_MAX;
    float noise = ((float)random(-20, 21)) / 1000.0f; 
    float voltage = baseVoltage + noise;
    if (voltage < 0) voltage = 0;
    if (voltage > V_MAX) voltage = V_MAX;
    return voltage;
}

float generateSolarUvIndex()
{
    float v = generateSolarUvVoltage();
    return voltageToUvIndex(v);
}