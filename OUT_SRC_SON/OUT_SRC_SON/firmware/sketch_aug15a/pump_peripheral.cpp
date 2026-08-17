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

static bool s_auto_mode = false;
static float s_distance_start_cm = 35.0f; // Bật bơm khi >= 35cm (nước cạn)
static float s_distance_stop_cm = 10.0f;  // Tắt bơm khi <= 10cm (nước đầy)

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
    if (on) {
        relay_on();
    } else {
        relay_off();
    }
}

void pump_toggle_relay(void)
{
    if (relay_get_state()) {
        relay_off();
    } else {
        relay_on();
    }
}

void pump_peripheral_set_auto_config(bool auto_mode, float distance_start_cm, float distance_stop_cm)
{
    s_auto_mode = auto_mode;
    if (distance_start_cm > 0.0f) s_distance_start_cm = distance_start_cm;
    if (distance_stop_cm > 0.0f) s_distance_stop_cm = distance_stop_cm;
    Serial.printf("[AUTO_PUMP] Cap nhat cau hinh: Auto=%s, Bat khi >= %.1f cm, Tat khi <= %.1f cm\r\n",
                  s_auto_mode ? "BAT" : "TAT", s_distance_start_cm, s_distance_stop_cm);
}

bool pump_peripheral_get_auto_mode(void)
{
    return s_auto_mode;
}

float pump_peripheral_get_distance_start(void)
{
    return s_distance_start_cm;
}

float pump_peripheral_get_distance_stop(void)
{
    return s_distance_stop_cm;
}

void pump_peripheral_check_auto(float distance_cm)
{
    if (!s_auto_mode || distance_cm <= 0.0f || distance_cm > 450.0f)
        return;

    if (distance_cm >= s_distance_start_cm) {
        // Nước cạn -> Bật bơm
        if (!pump_get_relay_state()) {
            pump_set_relay(true);
            Serial.printf("[AUTO] Khoang cach %.1f cm >= %.1f cm -> Tu dong BAT BOM\r\n", distance_cm, s_distance_start_cm);
        }
    } else if (distance_cm <= s_distance_stop_cm) {
        // Nước đầy -> Tắt bơm
        if (pump_get_relay_state()) {
            pump_set_relay(false);
            Serial.printf("[AUTO] Khoang cach %.1f cm <= %.1f cm -> Tu dong TAT BOM\r\n", distance_cm, s_distance_stop_cm);
        }
    }
}
