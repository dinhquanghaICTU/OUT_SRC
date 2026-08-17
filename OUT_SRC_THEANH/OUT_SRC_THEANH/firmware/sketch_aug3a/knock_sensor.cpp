#include "knock_sensor.h"
#include <Arduino.h>
#include <limits.h>

static long moving_average_buffer[HX710B_MOVING_AVG_WINDOW] = {0};
static int moving_average_index = 0;
static int moving_average_count = 0;

void init_knock_sensor(void)
{
    pinMode(KNOCKSENSOR_PIN_DO, INPUT);
    pinMode(KNOCKSENSOR_PIN_SCK, OUTPUT);
    digitalWrite(KNOCKSENSOR_PIN_SCK, LOW);
}

static long read_hx710b_raw(void)
{
    const unsigned long wait_started_ms = millis();

    while (digitalRead(KNOCKSENSOR_PIN_DO) == HIGH)
    {
        if (millis() - wait_started_ms > 1000UL)
            return LONG_MIN;

        delayMicroseconds(10);
    }

    long value = 0;

    for (int bit = 0; bit < 24; ++bit)
    {
        digitalWrite(KNOCKSENSOR_PIN_SCK, HIGH);
        delayMicroseconds(1);
        value = (value << 1) | digitalRead(KNOCKSENSOR_PIN_DO);
        digitalWrite(KNOCKSENSOR_PIN_SCK, LOW);
        delayMicroseconds(1);
    }

    digitalWrite(KNOCKSENSOR_PIN_SCK, HIGH);
    delayMicroseconds(1);
    digitalWrite(KNOCKSENSOR_PIN_SCK, LOW);
    delayMicroseconds(1);

    if (value & 0x800000L)
        value |= 0xFF000000L;

    return value;
}

static double push_moving_average(long sample)
{
    moving_average_buffer[moving_average_index] = sample;
    moving_average_index =
        (moving_average_index + 1) % HX710B_MOVING_AVG_WINDOW;

    if (moving_average_count < HX710B_MOVING_AVG_WINDOW)
        ++moving_average_count;

    long long sum = 0;
    for (int index = 0; index < moving_average_count; ++index)
        sum += moving_average_buffer[index];

    return (double)sum / moving_average_count;
}

bool read_pressure_sensor(long *raw_value,
                          double *filtered_raw,
                          float *pressure_hpa)
{
    if (raw_value == NULL || filtered_raw == NULL || pressure_hpa == NULL)
        return false;

    const long raw = read_hx710b_raw();
    if (raw == LONG_MIN)
        return false;

    const double filtered = push_moving_average(raw);

    *raw_value = raw;
    *filtered_raw = filtered;
    *pressure_hpa =
        (float)(HX710B_CAL_OFFSET + filtered * HX710B_CAL_SCALE);
    return true;
}
