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

typedef struct {
    float voltage_min;
    float voltage_max;
    float current_max;
    float power_max;
    uint32_t sample_interval_ms;
} device_thresholds_t;

typedef struct {
    bool is_alert;
    bool over_voltage;
    bool under_voltage;
    bool over_current;
    bool over_power;
    const char *alert_msg;
} alert_status_t;

static device_thresholds_t test_thresholds = {
    .voltage_min = 180.0f,
    .voltage_max = 245.0f,
    .current_max = 15.0f,
    .power_max = 3000.0f,
    .sample_interval_ms = 2000
};

alert_status_t check_thresholds(float currentA, float voltageV, float powerW, const device_thresholds_t *th) {
    alert_status_t status = {
        .is_alert = false,
        .over_voltage = false,
        .under_voltage = false,
        .over_current = false,
        .over_power = false,
        .alert_msg = "normal"
    };

    if (voltageV > 10.0f) {
        if (voltageV < th->voltage_min) {
            status.under_voltage = true;
            status.is_alert = true;
            status.alert_msg = "under_voltage";
        } else if (voltageV > th->voltage_max) {
            status.over_voltage = true;
            status.is_alert = true;
            status.alert_msg = "over_voltage";
        }
    }

    if (currentA > th->current_max) {
        status.over_current = true;
        status.is_alert = true;
        status.alert_msg = "over_current";
    }

    if (powerW > th->power_max) {
        status.over_power = true;
        status.is_alert = true;
        status.alert_msg = "over_power";
    }

    return status;
}

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
                                float current_a, float voltage_v) {
    float power_w = power_calc_watts(current_a, voltage_v);
    alert_status_t alert = check_thresholds(current_a, voltage_v, power_w, &test_thresholds);

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
        "\"alert\":%s,"
        "\"alert_msg\":\"%s\"}}",
        PRODUCT_ID, PRODUCT_ID, boot_id, seq, seq,
        (unsigned long long)mock_hal_get_time_ms(), FIRMWARE_VERSION,
        current_a, current_a,
        voltage_v, voltage_v,
        power_w, power_w,
        alert.is_alert ? "true" : "false",
        alert.alert_msg);
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
    float filtered_noise = smooth_deadband(prev, 220.05f, 0.1f, 0.2f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 220.0f, filtered_noise);

    float filtered_step = smooth_deadband(prev, 230.0f, 0.1f, 0.5f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 225.0f, filtered_step);
}

void test_theanh_threshold_and_alert_logic(void) {
    // 1. Normal state: 220V, 2A, 440W -> No Alert
    alert_status_t normal = check_thresholds(2.0f, 220.0f, 440.0f, &test_thresholds);
    TEST_ASSERT_FALSE(normal.is_alert);
    TEST_ASSERT_EQUAL_STRING("normal", normal.alert_msg);

    // 2. Under Voltage: 165V (< 180V) -> Alert
    alert_status_t under_v = check_thresholds(2.0f, 165.0f, 330.0f, &test_thresholds);
    TEST_ASSERT_TRUE(under_v.is_alert);
    TEST_ASSERT_TRUE(under_v.under_voltage);
    TEST_ASSERT_EQUAL_STRING("under_voltage", under_v.alert_msg);

    // 3. Over Voltage: 255V (> 245V) -> Alert
    alert_status_t over_v = check_thresholds(1.0f, 255.0f, 255.0f, &test_thresholds);
    TEST_ASSERT_TRUE(over_v.is_alert);
    TEST_ASSERT_TRUE(over_v.over_voltage);
    TEST_ASSERT_EQUAL_STRING("over_voltage", over_v.alert_msg);

    // 4. Over Current: 18A (> 15A) -> Alert
    alert_status_t over_i = check_thresholds(18.0f, 220.0f, 3960.0f, &test_thresholds);
    TEST_ASSERT_TRUE(over_i.is_alert);
    TEST_ASSERT_TRUE(over_i.over_current);

    // 5. Over Power: 3500W (> 3000W) -> Alert
    alert_status_t over_p = check_thresholds(14.0f, 230.0f, 3220.0f, &test_thresholds);
    TEST_ASSERT_TRUE(over_p.is_alert);
    TEST_ASSERT_TRUE(over_p.over_power);
}

void test_theanh_mqtt_telemetry_schema(void) {
    char payload[512];
    mock_hal_set_time_ms(50000);
    int len = mqtt_build_telemetry_theanh(payload, sizeof(payload), 77, 0x445566, 3.2f, 220.0f);
    TEST_ASSERT_TRUE(len > 0);

    TEST_ASSERT_TRUE(json_has_key(payload, "current_a"));
    TEST_ASSERT_TRUE(json_has_key(payload, "voltage_v"));
    TEST_ASSERT_TRUE(json_has_key(payload, "power_w"));
    TEST_ASSERT_TRUE(json_has_key(payload, "alert"));
    TEST_ASSERT_TRUE(json_has_key(payload, "alert_msg"));

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
    RUN_TEST(test_theanh_threshold_and_alert_logic);
    RUN_TEST(test_theanh_mqtt_telemetry_schema);
    return UNITY_END();
}
