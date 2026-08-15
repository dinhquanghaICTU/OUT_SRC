#include "pump_peripheral.h"

#include <Arduino.h>
#include "relay.h"

/*
 * YF-S201 water flow sensor
 * - OUT -> GPIO27 (digital interrupt)
 * - Formula phổ biến: F(Hz) = 7.5 * Q(L/min)
 *   => Q(L/min) = pulse_frequency / 7.5
 * - Tương đương khoảng 450 pulse / lít.
 */

#ifndef YFS201_CALIBRATION_FACTOR
#define YFS201_CALIBRATION_FACTOR 7.5F
#endif

#ifndef YFS201_PULSES_PER_LITER
#define YFS201_PULSES_PER_LITER 450.0F
#endif

#ifndef FLOW_UPDATE_INTERVAL_MS
#define FLOW_UPDATE_INTERVAL_MS 1000UL
#endif

static volatile unsigned long s_pulse_count = 0;
static unsigned long s_last_pulse_snapshot = 0;
static unsigned long s_last_update_ms = 0;
static float s_flow_l_min = 0.0F;
static float s_total_liters = 0.0F;

static void IRAM_ATTR flow_pulse_isr()
{
    ++s_pulse_count;
}

void pump_peripheral_init(void)
{
    pinMode(WATER_FLOW_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(WATER_FLOW_PIN), flow_pulse_isr, RISING);

    init_relay();
    pump_set_relay(false);

    noInterrupts();
    s_pulse_count = 0;
    interrupts();

    s_last_pulse_snapshot = 0;
    s_last_update_ms = millis();
    s_flow_l_min = 0.0F;
    s_total_liters = 0.0F;
}

void pump_peripheral_update(void)
{
    const unsigned long now = millis();
    const unsigned long elapsed_ms = now - s_last_update_ms;

    if (elapsed_ms < FLOW_UPDATE_INTERVAL_MS) {
        return;
    }

    noInterrupts();
    const unsigned long pulse_snapshot = s_pulse_count;
    interrupts();

    const unsigned long pulse_delta = pulse_snapshot - s_last_pulse_snapshot;
    s_last_pulse_snapshot = pulse_snapshot;
    s_last_update_ms = now;

    const float elapsed_seconds = (float)elapsed_ms / 1000.0F;
    const float frequency_hz = elapsed_seconds > 0.0F
                                   ? (float)pulse_delta / elapsed_seconds
                                   : 0.0F;

    s_flow_l_min = frequency_hz / YFS201_CALIBRATION_FACTOR;
    s_total_liters += (float)pulse_delta / YFS201_PULSES_PER_LITER;
}

float pump_get_flow_l_min(void)
{
    return s_flow_l_min;
}

float pump_get_total_liters(void)
{
    return s_total_liters;
}

unsigned long pump_get_pulse_count(void)
{
    noInterrupts();
    const unsigned long value = s_pulse_count;
    interrupts();
    return value;
}

bool pump_get_relay_state(void)
{
    return relay_get_state();
}

void pump_set_relay(bool on)
{
    relay_set(on);
}

void pump_toggle_relay(void)
{
    pump_set_relay(!pump_get_relay_state());
}
