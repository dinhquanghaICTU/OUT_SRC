#include "unity.h"
#include "mock_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define PRODUCT_ID "190782"
#define FIRMWARE_VERSION "1.0.0"

#define RELAY_PIN 5
#define YFS201_CALIBRATION_FACTOR 4.5f
#define YFS201_PULSES_PER_LITER 270.0f

// HC-SR04 ultrasonic distance calculation: duration_us * 0.0343f / 2.0f
float hcsr04_calc_distance(unsigned long duration_us) {
    if (duration_us == 0 || duration_us > 30000UL) return -1.0f;
    return (float)duration_us * 0.0343f / 2.0f;
}

// Flow sensor & pump calculations
typedef struct {
    unsigned long pulse_count;
    float flow_l_min;
    float total_liters;
    bool relay_on;
    bool auto_mode;
    float dist_start_cm;
    float dist_stop_cm;
} pump_system_t;

void pump_system_init(pump_system_t *ps) {
    ps->pulse_count = 0;
    ps->flow_l_min = 0.0f;
    ps->total_liters = 0.0f;
    ps->relay_on = false;
    ps->auto_mode = true;
    ps->dist_start_cm = 80.0f; // Nước cạn -> Bật bơm
    ps->dist_stop_cm = 20.0f;  // Nước đầy -> Tắt bơm
    mock_gpio_set_mode(RELAY_PIN, MOCK_GPIO_MODE_OUTPUT);
    mock_gpio_write(RELAY_PIN, 0);
}

void pump_system_update_flow(pump_system_t *ps, unsigned long pulse_delta, float elapsed_sec) {
    ps->pulse_count += pulse_delta;
    float freq_hz = elapsed_sec > 0.0f ? (float)pulse_delta / elapsed_sec : 0.0f;
    ps->flow_l_min = freq_hz / YFS201_CALIBRATION_FACTOR;
    ps->total_liters += (float)pulse_delta / YFS201_PULSES_PER_LITER;
}

void pump_system_check_auto(pump_system_t *ps, float distance_cm) {
    if (!ps->auto_mode || distance_cm <= 0.0f || distance_cm > 450.0f) return;
    if (distance_cm >= ps->dist_start_cm) {
        ps->relay_on = true;
        mock_gpio_write(RELAY_PIN, 1);
    } else if (distance_cm <= ps->dist_stop_cm) {
        ps->relay_on = false;
        mock_gpio_write(RELAY_PIN, 0);
    }
}

// MQTT Telemetry Builder
int mqtt_build_telemetry_son(char *buf, size_t max_len, uint32_t seq, uint32_t boot_id,
                             float flow, float liters, bool pump_on, float dist_cm) {
    return snprintf(buf, max_len,
        "{\"schema_version\":1,"
        "\"device_id\":\"%s\","
        "\"message_id\":\"%s-%08x-%u\","
        "\"sequence\":%u,"
        "\"uptime_ms\":%llu,"
        "\"firmware_version\":\"%s\","
        "\"metrics\":{"
        "\"flow_rate_l_min\":%.2f,"
        "\"flow_l_min\":%.2f,"
        "\"total_liters\":%.2f,"
        "\"liters\":%.2f,"
        "\"pump_on\":%s,"
        "\"distance_cm\":%.2f,"
        "\"distance\":%.2f,"
        "\"relay_on\":%s,"
        "\"relay\":%s}}",
        PRODUCT_ID, PRODUCT_ID, boot_id, seq, seq,
        (unsigned long long)mock_hal_get_time_ms(), FIRMWARE_VERSION,
        flow, flow, liters, liters,
        pump_on ? "true" : "false",
        dist_cm, dist_cm,
        pump_on ? "true" : "false", pump_on ? "true" : "false");
}

void setUp(void) {
    mock_hal_reset();
}

void tearDown(void) {}

void test_son_hcsr04_distance_math(void) {
    // 583 us -> ~10.0 cm
    float d1 = hcsr04_calc_distance(583);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 10.0f, d1);

    // 2915 us -> ~50.0 cm
    float d2 = hcsr04_calc_distance(2915);
    TEST_ASSERT_FLOAT_WITHIN(0.2f, 50.0f, d2);

    // Timeout duration -> -1
    float d_err = hcsr04_calc_distance(0);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -1.0f, d_err);
}

void test_son_flow_rate_and_liter_accumulator(void) {
    pump_system_t ps;
    pump_system_init(&ps);

    // 270 pulses in 1 second -> 270 Hz / 4.5 = 60 L/min, 1.0 Liters total
    pump_system_update_flow(&ps, 270, 1.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 60.0f, ps.flow_l_min);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, ps.total_liters);

    // Another 540 pulses in 2 seconds -> 270 Hz / 4.5 = 60 L/min, +2.0 Liters = 3.0 Liters
    pump_system_update_flow(&ps, 540, 2.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 60.0f, ps.flow_l_min);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.0f, ps.total_liters);
}

void test_son_auto_pump_logic(void) {
    pump_system_t ps;
    pump_system_init(&ps);

    // Water level drops -> distance reaches 85 cm (>= 80 cm start) -> Pump turns ON
    pump_system_check_auto(&ps, 85.0f);
    TEST_ASSERT_TRUE(ps.relay_on);
    TEST_ASSERT_EQUAL_INT(1, mock_gpio_get_output_level(RELAY_PIN));

    // Water rising -> distance 50 cm (in hysteresis band) -> Pump stays ON
    pump_system_check_auto(&ps, 50.0f);
    TEST_ASSERT_TRUE(ps.relay_on);

    // Water reaches top -> distance 15 cm (<= 20 cm stop) -> Pump turns OFF
    pump_system_check_auto(&ps, 15.0f);
    TEST_ASSERT_FALSE(ps.relay_on);
    TEST_ASSERT_EQUAL_INT(0, mock_gpio_get_output_level(RELAY_PIN));
}

void test_son_mqtt_telemetry_metrics(void) {
    char payload[512];
    mock_hal_set_time_ms(45000);
    int len = mqtt_build_telemetry_son(payload, sizeof(payload), 55, 0x112233, 12.5f, 48.0f, true, 72.4f);
    TEST_ASSERT_TRUE(len > 0);

    TEST_ASSERT_TRUE(json_has_key(payload, "flow_rate_l_min"));
    TEST_ASSERT_TRUE(json_has_key(payload, "total_liters"));
    TEST_ASSERT_TRUE(json_has_key(payload, "pump_on"));
    TEST_ASSERT_TRUE(json_has_key(payload, "distance_cm"));

    double flow_val, liters_val, dist_val;
    json_extract_number(payload, "flow_rate_l_min", &flow_val);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 12.5f, (float)flow_val);
    json_extract_number(payload, "total_liters", &liters_val);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 48.0f, (float)liters_val);
    json_extract_number(payload, "distance_cm", &dist_val);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 72.4f, (float)dist_val);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_son_hcsr04_distance_math);
    RUN_TEST(test_son_flow_rate_and_liter_accumulator);
    RUN_TEST(test_son_auto_pump_logic);
    RUN_TEST(test_son_mqtt_telemetry_metrics);
    return UNITY_END();
}
