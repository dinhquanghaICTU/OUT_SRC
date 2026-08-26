#include "unity.h"
#include "mock_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define PRODUCT_ID "190772"
#define FIRMWARE_VERSION "1.0.0"

#define HX710B_MOVING_AVG_WINDOW 10
#define HX710B_CAL_OFFSET 0.0f
#define HX710B_CAL_SCALE 0.001f
#define RING_PIN 19
#define LED_PIN 2

// Moving average filter for knock / vibration sensor
typedef struct {
    long buffer[HX710B_MOVING_AVG_WINDOW];
    int index;
    int count;
} knock_filter_t;

void knock_filter_init(knock_filter_t *kf) {
    memset(kf->buffer, 0, sizeof(kf->buffer));
    kf->index = 0;
    kf->count = 0;
}

double knock_filter_push(knock_filter_t *kf, long sample) {
    kf->buffer[kf->index] = sample;
    kf->index = (kf->index + 1) % HX710B_MOVING_AVG_WINDOW;
    if (kf->count < HX710B_MOVING_AVG_WINDOW) kf->count++;

    long long sum = 0;
    for (int i = 0; i < kf->count; ++i) sum += kf->buffer[i];
    return (double)sum / kf->count;
}

// UV Lookup Table
float voltageToUvIndex(float voltage) {
    typedef struct { float v; float idx; } Point;
    static const Point table[] = {
        {0.00f, 0}, {0.43f, 0}, {0.49f, 1}, {0.55f, 2},
        {0.62f, 3}, {0.68f, 4}, {0.74f, 5}, {0.80f, 6},
        {0.86f, 7}, {0.93f, 8}, {0.99f, 9}, {1.05f, 10}
    };
    const int n = sizeof(table) / sizeof(table[0]);
    if (voltage <= table[0].v) return table[0].idx;
    if (voltage >= table[n - 1].v) return table[n - 1].idx;
    for (int i = 0; i < n - 1; i++) {
        if (voltage >= table[i].v && voltage <= table[i + 1].v) {
            float ratio = (voltage - table[i].v) / (table[i + 1].v - table[i].v);
            return table[i].idx + ratio * (table[i + 1].idx - table[i].idx);
        }
    }
    return table[n - 1].idx;
}

// Solar diurnal UV estimation generator (from firmware)
float generateSolarUvVoltage(uint64_t ms) {
    const float DAY_PERIOD_MS = 2 * 60 * 1000.0f;
    float t = fmodf((float)ms, DAY_PERIOD_MS) / DAY_PERIOD_MS;
    float sunCurve = sinf((float)M_PI * t);
    if (sunCurve < 0) sunCurve = 0;
    const float V_MAX = 1.05f;
    float voltage = sunCurve * V_MAX;
    if (voltage < 0) voltage = 0;
    if (voltage > V_MAX) voltage = V_MAX;
    return voltage;
}

// Ring Controller
static bool s_ring_state = false;
void ring_set(bool on) {
    s_ring_state = on;
    mock_gpio_write(RING_PIN, on ? 1 : 0);
}
bool ring_get_state(void) { return s_ring_state; }

// MQTT Telemetry Builder
int mqtt_build_telemetry_trungkien(char *buf, size_t max_len, uint32_t seq, uint32_t boot_id,
                                   bool knock, float uv_idx, float uv_v, bool ring_on) {
    return snprintf(buf, max_len,
        "{\"schema_version\":1,"
        "\"device_id\":\"%s\","
        "\"message_id\":\"%s-%08x-%u\","
        "\"sequence\":%u,"
        "\"uptime_ms\":%llu,"
        "\"firmware_version\":\"%s\","
        "\"metrics\":{"
        "\"knock_detected\":%s,"
        "\"knock\":%s,"
        "\"uv_index\":%.2f,"
        "\"uv\":%.2f,"
        "\"uv_voltage_v\":%.3f,"
        "\"ring_on\":%s,"
        "\"ring\":%s}}",
        PRODUCT_ID, PRODUCT_ID, boot_id, seq, seq,
        (unsigned long long)mock_hal_get_time_ms(), FIRMWARE_VERSION,
        knock ? "true" : "false", knock ? "true" : "false",
        uv_idx, uv_idx, uv_v,
        ring_on ? "true" : "false", ring_on ? "true" : "false");
}

void setUp(void) {
    mock_hal_reset();
    s_ring_state = false;
}

void tearDown(void) {}

void test_trungkien_knock_moving_average_filter(void) {
    knock_filter_t kf;
    knock_filter_init(&kf);

    // Feed initial stable values
    for (int i = 0; i < 5; i++) {
        knock_filter_push(&kf, 1000);
    }
    double avg = knock_filter_push(&kf, 1000);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1000.0f, (float)avg);

    // Impulse knock pulse
    avg = knock_filter_push(&kf, 2000);
    TEST_ASSERT_TRUE(avg > 1000.0f);
}

void test_trungkien_uv_lookup_interpolation(void) {
    // Exact table points
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, voltageToUvIndex(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, voltageToUvIndex(0.43f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, voltageToUvIndex(0.49f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, voltageToUvIndex(0.74f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, voltageToUvIndex(1.05f));

    // Intermediate interpolation
    // Between 0.49V (1) and 0.55V (2), 0.52V is midway -> index 1.5
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 1.5f, voltageToUvIndex(0.52f));
}

void test_trungkien_solar_uv_simulation(void) {
    // At t=0 ms -> midday angle 0 -> 0V
    float v0 = generateSolarUvVoltage(0);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, v0);

    // At t=60000 ms (midday peak) -> sin(pi/2) = 1 -> 1.05V -> UV index 10
    float v_peak = generateSolarUvVoltage(60000);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.05f, v_peak);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 10.0f, voltageToUvIndex(v_peak));
}

void test_trungkien_ring_buzzer_logic(void) {
    ring_set(true);
    TEST_ASSERT_TRUE(ring_get_state());
    TEST_ASSERT_EQUAL_INT(1, mock_gpio_get_output_level(RING_PIN));

    ring_set(false);
    TEST_ASSERT_FALSE(ring_get_state());
    TEST_ASSERT_EQUAL_INT(0, mock_gpio_get_output_level(RING_PIN));
}

void test_trungkien_mqtt_telemetry_payload(void) {
    char payload[512];
    mock_hal_set_time_ms(30000);
    int len = mqtt_build_telemetry_trungkien(payload, sizeof(payload), 15, 0x998877, true, 7.5f, 0.89f, false);
    TEST_ASSERT_TRUE(len > 0);

    TEST_ASSERT_TRUE(json_has_key(payload, "schema_version"));
    TEST_ASSERT_TRUE(json_has_key(payload, "device_id"));
    TEST_ASSERT_TRUE(json_has_key(payload, "knock_detected"));
    TEST_ASSERT_TRUE(json_has_key(payload, "uv_index"));
    TEST_ASSERT_TRUE(json_has_key(payload, "ring_on"));

    char dev_id[64];
    json_extract_string(payload, "device_id", dev_id, sizeof(dev_id));
    TEST_ASSERT_EQUAL_STRING(PRODUCT_ID, dev_id);

    double uv_val;
    json_extract_number(payload, "uv_index", &uv_val);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 7.5f, (float)uv_val);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_trungkien_knock_moving_average_filter);
    RUN_TEST(test_trungkien_uv_lookup_interpolation);
    RUN_TEST(test_trungkien_solar_uv_simulation);
    RUN_TEST(test_trungkien_ring_buzzer_logic);
    RUN_TEST(test_trungkien_mqtt_telemetry_payload);
    return UNITY_END();
}
