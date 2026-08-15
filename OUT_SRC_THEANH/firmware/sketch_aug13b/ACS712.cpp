#include "ACS712.h"
#include <Arduino.h>
#include <math.h>

static float s_zeroVoltage = ACS712_ADC_REF_V / 2.0F;
static float s_currentA = 0.0F;
static float s_sensorVoltageV = 0.0F;
static bool s_ready = false;

static float read_adc_voltage()
{
    return analogReadMilliVolts(ACS712_PIN) / 1000.0F;
}

static float smooth_deadband(float previous, float current, float deadband)
{
    if (fabsf(current - previous) < deadband)
        return previous;
    return previous + (current - previous) * ACS712_FILTER_ALPHA;
}

void acs712_init()
{
    pinMode(ACS712_PIN, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(ACS712_PIN, ADC_11db);

    float total = 0.0F;
    for (int i = 0; i < ACS712_ZERO_SAMPLE_COUNT; ++i)
    {
        total += read_adc_voltage();
        delay(2);
    }

    s_zeroVoltage = total / ACS712_ZERO_SAMPLE_COUNT;
    s_currentA = 0.0F;
    s_sensorVoltageV = s_zeroVoltage;
    s_ready = true;
}

bool acs712_update()
{
    if (!s_ready)
        acs712_init();

    float squareSum = 0.0F;
    float voltageSum = 0.0F;

    for (int i = 0; i < ACS712_SAMPLE_COUNT; ++i)
    {
        const float voltage = read_adc_voltage();
        const float centered = voltage - s_zeroVoltage;
        squareSum += centered * centered;
        voltageSum += voltage;
        delayMicroseconds(ACS712_SAMPLE_DELAY_US);
    }

    const float rmsVoltage = sqrtf(squareSum / ACS712_SAMPLE_COUNT);
    const float currentA = rmsVoltage / ACS712_SENSITIVITY_V_PER_A;
    const float avgVoltage = voltageSum / ACS712_SAMPLE_COUNT;

    s_sensorVoltageV = smooth_deadband(s_sensorVoltageV, avgVoltage, ACS712_VOLTAGE_DEADBAND_V);
    s_currentA = smooth_deadband(s_currentA, currentA, ACS712_CURRENT_DEADBAND_A);

    return true;
}

float acs712_get_current_a()
{
    return s_currentA;
}

float acs712_get_sensor_voltage_v()
{
    return s_sensorVoltageV;
}

float acs712_get_zero_voltage_v()
{
    return s_zeroVoltage;
}
