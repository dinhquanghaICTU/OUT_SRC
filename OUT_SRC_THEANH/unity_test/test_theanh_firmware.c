#include "unity.h"
#include "mock_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define PRODUCT_ID "Theanh-190782"
#define FIRMWARE_VERSION "1.0.0"

#define ACS712_SENSITIVITY_V_PER_A 0.185f
#define ZMPT101B_CALIBRATION 628.57f
#define RELAY_PIN 5

// ACS712 RMS Current calculation
float acs712_calc_current(float sensor_rms_v) {
    return sensor_rms_v / ACS712_SENSITIVITY_V_PER_A;
}

// ZMPT101B RMS Voltage and Power calculation
float zmpt101b_calc_mains_voltage(float sensor_rms_v) {
    return sensor_rms_v * ZMPT101B_CALIBRATION;
}

float power_calc_watts(float current_a, float voltage_v) {
    return current_a * voltage_v;
}

// Smooth deadband filter
float smooth_deadband(float previous, float current, float deadband, float alpha) {
    if (fabsf(current - previous) < deadband) return previous;
    return previous + (current - previous) * alpha;
}

// MQTT Telemetry Builder
int mqtt_build_telemetry_theanh(char *buf, size_t max_len, uint32_t seq, uint32_t boot_id,
                                float current_a, float voltage_v, bool relay_on) {
    float power_w = power_calc_watts(current_a, voltage_v);
    return snprintf(buf, max_len,
        "{\"schema_version\":1,"
        "\"device_id\":\"%s\","
        "\"message_id\":\"%s-%08x-%u\","
        "\"sequence\":%u,"
        "\"uptime_ms\":%llu,"
        "\"firmware_version\":\"%s\","
        "\"metrics\":{"
        "\"current_a\":%.2f,"
        "\"current\":%.2f,"
        "\"voltage_v\":%.2f,"
        "\"voltage\":%.2f,"
        "\"power_w\":%.2f,"
        "\"power\":%.2f,"
        "\"relay_on\":%s,"
        "\"relay\":%s}}",
        PRODUCT_ID, PRODUCT_ID, boot_id, seq, seq,
        (unsigned long long)mock_hal_get_time_ms(), FIRMWARE_VERSION,
        current_a, current_a,
        voltage_v, voltage_v,
        power_w, power_w,
        relay_on ? "true" : "false", relay_on ? "true" : "false");
}

void setUp(void) {
    mock_hal_reset();
}

void tearDown(void) {}

void test_theanh_acs712_current_math(void) {
    // 0.370 V RMS / 0.185 V/A -> 2.00 A
    float i1 = acs712_calc_current(0.370f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.00f, i1);

    // 0.0925 V RMS -> 0.50 A
    float i2 = acs712_calc_current(0.0925f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.50f, i2);
}

void test_theanh_zmpt101b_voltage_and_power_math(void) {
    // 0.350 V RMS * 628.57 -> 220.0 V
    float v = zmpt101b_calc_mains_voltage(0.350f);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 220.0f, v);

    // 2.5 A * 220 V -> 550 W
    float p = power_calc_watts(2.5f, 220.0f);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 550.0f, p);
}

void test_theanh_smooth_deadband_filter(void) {
    float prev = 220.0f;
    // Small noise 0.05V within deadband 0.1V -> Keeps previous
    float filtered_noise = smooth_deadband(prev, 220.05f, 0.1f, 0.2f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 220.0f, filtered_noise);

    // Large step change 230V (> 0.1V) -> Filters smoothly
    float filtered_step = smooth_deadband(prev, 230.0f, 0.1f, 0.5f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 225.0f, filtered_step);
}

void test_theanh_mqtt_telemetry_schema(void) {
    char payload[512];
    mock_hal_set_time_ms(50000);
    int len = mqtt_build_telemetry_theanh(payload, sizeof(payload), 77, 0x445566, 3.2f, 220.0f, true);
    TEST_ASSERT_TRUE(len > 0);

    TEST_ASSERT_TRUE(json_has_key(payload, "current_a"));
    TEST_ASSERT_TRUE(json_has_key(payload, "voltage_v"));
    TEST_ASSERT_TRUE(json_has_key(payload, "power_w"));
    TEST_ASSERT_TRUE(json_has_key(payload, "relay_on"));

    char dev_id[64];
    json_extract_string(payload, "device_id", dev_id, sizeof(dev_id));
    TEST_ASSERT_EQUAL_STRING(PRODUCT_ID, dev_id);

    double power_val;
    json_extract_number(payload, "power_w", &power_val);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 704.0f, (float)power_val);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_theanh_acs712_current_math);
    RUN_TEST(test_theanh_zmpt101b_voltage_and_power_math);
    RUN_TEST(test_theanh_smooth_deadband_filter);
    RUN_TEST(test_theanh_mqtt_telemetry_schema);
    return UNITY_END();
}
