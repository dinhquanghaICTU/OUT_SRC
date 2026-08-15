#include "PIR.h"
#include <driver/gpio.h>
#include <Arduino.h>

static int s_lastRawState = LOW;
static int s_stableState = LOW;
static uint32_t s_lastRawChangeMs = 0;

void pir_init(){
    pinMode(PIR_PIN, INPUT);
    s_lastRawState = digitalRead(PIR_PIN);
    s_stableState = s_lastRawState;
    s_lastRawChangeMs = millis();
}

int pir_read_raw()
{
    return digitalRead(PIR_PIN);
}

bool pir_update()
{
    const int rawState = pir_read_raw();
    const uint32_t nowMs = millis();

    if (rawState != s_lastRawState)
    {
        s_lastRawState = rawState;
        s_lastRawChangeMs = nowMs;
    }

    if ((nowMs - s_lastRawChangeMs) >= PIR_DEBOUNCE_MS &&
        s_stableState != s_lastRawState)
    {
        s_stableState = s_lastRawState;
    }

    return pir_is_detected();
}

bool pir_is_detected()
{
    return s_stableState == PIR_ACTIVE_LEVEL;
}
