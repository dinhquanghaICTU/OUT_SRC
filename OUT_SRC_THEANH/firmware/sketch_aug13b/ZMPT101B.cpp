#include "ZMPT101B.h"
#include <Arduino.h>
#include <math.h>

static float s_zeroVoltage = ZMPT101B_ADC_REF_V / 2.0F;
static float s_voltageV = 0.0F;
static float s_sensorRmsV = 0.0F;
static bool s_ready = false;

static float read_adc_voltage()
{
    return analogReadMilliVolts(ZMPT101B_PIN) / 1000.0F;
}

static float smooth_deadband(float previous, float current, float deadband)
{
    if (fabsf(current - previous) < deadband)
        return previous;
    return previous + (current - previous) * ZMPT101B_FILTER_ALPHA;
}

void zmpt101b_init()
{
    pinMode(ZMPT101B_PIN, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(ZMPT101B_PIN, ADC_11db);

    float total = 0.0F;
    for (int i = 0; i < ZMPT101B_ZERO_SAMPLE_COUNT; ++i)
    {
        total += read_adc_voltage();
        delay(2);
    }

    s_zeroVoltage = total / ZMPT101B_ZERO_SAMPLE_COUNT;
    s_sensorRmsV = 0.0F;
    s_voltageV = 0.0F;
    s_ready = true;
}

bool zmpt101b_update()
{
    if (!s_ready)
        zmpt101b_init();

    float squareSum = 0.0F;

    for (int i = 0; i < ZMPT101B_SAMPLE_COUNT; ++i)
    {
        const float voltage = read_adc_voltage();
        const float centered = voltage - s_zeroVoltage;
        squareSum += centered * centered;
        delayMicroseconds(ZMPT101B_SAMPLE_DELAY_US);
    }

    const float sensorRmsV = sqrtf(squareSum / ZMPT101B_SAMPLE_COUNT);
    const float mainsVoltageV = sensorRmsV * ZMPT101B_CALIBRATION;

    s_sensorRmsV = smooth_deadband(s_sensorRmsV, sensorRmsV, ZMPT101B_SENSOR_DEADBAND_V);
    s_voltageV = smooth_deadband(s_voltageV, mainsVoltageV, ZMPT101B_VOLTAGE_DEADBAND_V);

    return true;
}

float zmpt101b_get_voltage_v()
{
    return s_voltageV;
}

float zmpt101b_get_sensor_rms_v()
{
    return s_sensorRmsV;
}

float zmpt101b_get_zero_voltage_v()
{
    return s_zeroVoltage;
}
