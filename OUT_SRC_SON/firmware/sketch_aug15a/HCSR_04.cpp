#include "HCSR_04.h"

#include <Arduino.h>

#ifndef HCSR04_TIMEOUT_US
#define HCSR04_TIMEOUT_US 30000UL
#endif

#ifndef HCSR04_SAMPLE_COUNT
#define HCSR04_SAMPLE_COUNT 5
#endif

#ifndef HCSR04_SAMPLE_DELAY_MS
#define HCSR04_SAMPLE_DELAY_MS 10
#endif

#ifndef HCSR04_INVALID_DISTANCE_CM
#define HCSR04_INVALID_DISTANCE_CM -1.0F
#endif

#if defined(HCSR04_TRIG_PIN) && defined(HCSR04_ECHO_PIN)
static constexpr uint8_t HCSR_TRIG = HCSR04_TRIG_PIN;
static constexpr uint8_t HCSR_ECHO = HCSR04_ECHO_PIN;
#elif defined(HCSR_04_TRIG_PIN) && defined(HCSR_04_ECHO_PIN)
static constexpr uint8_t HCSR_TRIG = HCSR_04_TRIG_PIN;
static constexpr uint8_t HCSR_ECHO = HCSR_04_ECHO_PIN;
#elif defined(TRIG_PIN) && defined(ECHO_PIN)
static constexpr uint8_t HCSR_TRIG = TRIG_PIN;
static constexpr uint8_t HCSR_ECHO = ECHO_PIN;
#else
// Fallback: nếu mày nối chân khác thì define trong HCSR_04.h hoặc config.h.
static constexpr uint8_t HCSR_TRIG = 18;
static constexpr uint8_t HCSR_ECHO = 5;
#endif

static float s_last_distance_cm = HCSR04_INVALID_DISTANCE_CM;

static float read_once_cm()
{
    digitalWrite(HCSR_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(HCSR_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(HCSR_TRIG, LOW);

    const unsigned long duration = pulseIn(HCSR_ECHO, HIGH, HCSR04_TIMEOUT_US);
    if (duration == 0) {
        return HCSR04_INVALID_DISTANCE_CM;
    }

    return (float)duration * 0.0343F / 2.0F;
}

void hcsr04_init()
{
    pinMode(HCSR_TRIG, OUTPUT);
    pinMode(HCSR_ECHO, INPUT);
    digitalWrite(HCSR_TRIG, LOW);

    s_last_distance_cm = HCSR04_INVALID_DISTANCE_CM;
}

float hcsr04_read_distance_cm()
{
    float total = 0.0F;
    int validCount = 0;

    for (int i = 0; i < HCSR04_SAMPLE_COUNT; ++i) {
        const float distance = read_once_cm();
        if (distance > 0.0F && distance < 500.0F) {
            total += distance;
            ++validCount;
        }
        delay(HCSR04_SAMPLE_DELAY_MS);
    }

    if (validCount == 0) {
        return s_last_distance_cm;
    }

    s_last_distance_cm = total / (float)validCount;
    return s_last_distance_cm;
}

float hcsr04_get_last_distance_cm()
{
    return s_last_distance_cm;
}
