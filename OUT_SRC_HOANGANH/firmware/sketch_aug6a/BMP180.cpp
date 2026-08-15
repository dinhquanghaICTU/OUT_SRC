#include "BMP180.h"
#include "config.h"

#include <Arduino.h>
#include <BMP180I2C.h>
#include <Wire.h>
#include <math.h>

namespace {
BMP180I2C sensor(BMP180_I2C_ADDRESS);
bool sensorReady = false;

bool waitForMeasurement(unsigned long timeoutMs)
{
    const unsigned long startedAt = millis();
    while (!sensor.hasValue()) {
        if (millis() - startedAt >= timeoutMs)
            return false;
        delay(2);
    }
    return true;
}
}

bool bmp180_begin(void)
{
    Wire.begin(BMP180_SDA_PIN, BMP180_SCL_PIN);
    Wire.setClock(100000);
    sensorReady = sensor.begin();
    if (!sensorReady) {
        Serial.printf("[BMP180] Khong tim thay sensor tai 0x%02X (SDA=%d, SCL=%d)\n",
                      BMP180_I2C_ADDRESS, BMP180_SDA_PIN, BMP180_SCL_PIN);
        return false;
    }
    sensor.resetToDefaults();
    sensor.setSamplingMode(BMP180MI::MODE_UHR);
    Serial.printf("[BMP180] San sang tai 0x%02X (SDA=%d, SCL=%d)\n",
                  BMP180_I2C_ADDRESS, BMP180_SDA_PIN, BMP180_SCL_PIN);
    return true;
}

bool bmp180_is_ready(void)
{
    return sensorReady;
}

bool bmp180_read(float *temperature_c, float *pressure_hpa)
{
    if (!sensorReady || temperature_c == nullptr || pressure_hpa == nullptr)
        return false;
    if (!sensor.measureTemperature() || !waitForMeasurement(200))
        return false;
    const float temperature = sensor.getTemperature();
    if (!sensor.measurePressure() || !waitForMeasurement(300))
        return false;
    const float pressurePa = sensor.getPressure();
    if (!isfinite(temperature) || !isfinite(pressurePa)
        || pressurePa < 30000.0f || pressurePa > 120000.0f)
        return false;
    *temperature_c = temperature;
    *pressure_hpa = pressurePa / 100.0f;
    return true;
}

void bmp180_simulate(float *temperature_c, float *pressure_hpa)
{
    if (temperature_c == nullptr || pressure_hpa == nullptr)
        return;
    const float seconds = millis() / 1000.0f;
    *temperature_c = 29.0f + 2.2f * sinf(seconds / 17.0f)
        + 0.25f * sinf(seconds / 3.0f);
    *pressure_hpa = 1008.0f + 4.5f * sinf(seconds / 29.0f)
        + 0.4f * sinf(seconds / 5.0f);
}
