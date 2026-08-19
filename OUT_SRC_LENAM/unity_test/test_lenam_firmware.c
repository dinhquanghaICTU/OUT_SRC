#include "unity.h"
#include "mock_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define PRODUCT_ID "190782"
#define FIRMWARE_VERSION "1.0.0"

#define LM35_PIN 35
#define MICROPHONE_PIN 34
#define RELAY_PIN 5
#define RING_PIN 19
#define LED_PIN 2

// LM35 conversion logic: ADC millivolts / 1000.0f -> V; Temp = V * 100.0f
float lm35_calc_temp(uint32_t mv_avg) {
    float voltage = (float)mv_avg / 1000.0f;
    return voltage * 100.0f;
}

// MAX9814 peak-to-peak amplitude voltage calculation
float mic_calc_amplitude_voltage(int raw_min, int raw_max) {
    int peak_to_peak = raw_max - raw_min;
    if (peak_to_peak < 0) peak_to_peak = 0;
    return (float)peak_to_peak * 3.3f / 4095.0f;
}

// Relay Controller
static bool s_relay_state = false;
void relay_init(void) {
    mock_gpio_set_mode(RELAY_PIN, MOCK_GPIO_MODE_OUTPUT);
    mock_gpio_write(RELAY_PIN, 0);
    s_relay_state = false;
}
void relay_set(bool state) {
    s_relay_state = state;
    mock_gpio_write(RELAY_PIN, state ? 1 : 0);
}
bool relay_get_state(void) { return s_relay_state; }

// MQTT Telemetry Builder
int mqtt_build_telemetry_lenam(char *buf, size_t max_len, uint32_t seq, uint32_t boot_id,
                               float temp_c, float mic_vpp, bool relay_on) {
    return snprintf(buf, max_len,
        "{\"schema_version\":1,"
        "\"device_id\":\"%s\","
        "\"message_id\":\"%s-%08x-%u\","
        "\"sequence\":%u,"
        "\"uptime_ms\":%llu,"
        "\"firmware_version\":\"%s\","
        "\"metrics\":{"
        "\"temperature_c\":%.2f,"
        "\"temp\":%.2f,"
        "\"sound_level_db\":%.2f,"
        "\"sound_vpp\":%.3f,"
        "\"relay_on\":%s,"
        "\"relay\":%s}}",
        PRODUCT_ID, PRODUCT_ID, boot_id, seq, seq,
        (unsigned long long)mock_hal_get_time_ms(), FIRMWARE_VERSION,
        temp_c, temp_c,
        mic_vpp * 50.0f, mic_vpp,
        relay_on ? "true" : "false", relay_on ? "true" : "false");
}

// MQTT Command Parser
bool mqtt_parse_relay_command(const char *payload, char *cmd_id, size_t cmd_id_size, bool *out_state) {
    if (!payload || !cmd_id || !out_state) return false;
    char cmd_type[32] = {0};
    if (!json_extract_string(payload, "command_id", cmd_id, cmd_id_size)) return false;
    if (!json_extract_string(payload, "type", cmd_type, sizeof(cmd_type))) return false;
    if (strcmp(cmd_type, "relay.set") != 0) return false;
    return json_extract_bool(payload, "state", out_state);
}

void setUp(void) {
    mock_hal_reset();
    relay_init();
}

void tearDown(void) {}

void test_lenam_lm35_temperature_conversion(void) {
    // 250 mV -> 25.0 °C
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.0f, lm35_calc_temp(250));
    // 375 mV -> 37.5 °C
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 37.5f, lm35_calc_temp(375));
    // 0 mV -> 0.0 °C
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, lm35_calc_temp(0));
}

void test_lenam_mic_peak_to_peak_calculation(void) {
    // Quiet room: min=2000, max=2050 (peak-to-peak = 50 raw)
    float vpp_quiet = mic_calc_amplitude_voltage(2000, 2050);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.040f, vpp_quiet);

    // Loud noise: min=500, max=3500 (peak-to-peak = 3000 raw)
    float vpp_loud = mic_calc_amplitude_voltage(500, 3500);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.417f, vpp_loud);
}

void test_lenam_relay_control_and_gpio(void) {
    TEST_ASSERT_FALSE(relay_get_state());
    TEST_ASSERT_EQUAL_INT(0, mock_gpio_get_output_level(RELAY_PIN));

    relay_set(true);
    TEST_ASSERT_TRUE(relay_get_state());
    TEST_ASSERT_EQUAL_INT(1, mock_gpio_get_output_level(RELAY_PIN));

    relay_set(false);
    TEST_ASSERT_FALSE(relay_get_state());
    TEST_ASSERT_EQUAL_INT(0, mock_gpio_get_output_level(RELAY_PIN));
}

void test_lenam_mqtt_telemetry_keys_and_values(void) {
    char payload[512];
    mock_hal_set_time_ms(24000);
    int len = mqtt_build_telemetry_lenam(payload, sizeof(payload), 10, 0x123456, 31.4f, 0.85f, true);
    TEST_ASSERT_TRUE(len > 0);

    TEST_ASSERT_TRUE(json_has_key(payload, "schema_version"));
    TEST_ASSERT_TRUE(json_has_key(payload, "device_id"));
    TEST_ASSERT_TRUE(json_has_key(payload, "message_id"));
    TEST_ASSERT_TRUE(json_has_key(payload, "sequence"));
    TEST_ASSERT_TRUE(json_has_key(payload, "metrics"));
    TEST_ASSERT_TRUE(json_has_key(payload, "temperature_c"));
    TEST_ASSERT_TRUE(json_has_key(payload, "relay_on"));

    char dev_id[64];
    json_extract_string(payload, "device_id", dev_id, sizeof(dev_id));
    TEST_ASSERT_EQUAL_STRING(PRODUCT_ID, dev_id);

    double temp_val;
    json_extract_number(payload, "temperature_c", &temp_val);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 31.4f, (float)temp_val);

    bool relay_val;
    json_extract_bool(payload, "relay_on", &relay_val);
    TEST_ASSERT_TRUE(relay_val);
}

void test_lenam_mqtt_command_relay_execution(void) {
    char cmd_id[64];
    bool target_state = false;

    const char *cmd_json = "{\"command_id\":\"cmd-ln-99\",\"type\":\"relay.set\",\"state\":true}";
    TEST_ASSERT_TRUE(mqtt_parse_relay_command(cmd_json, cmd_id, sizeof(cmd_id), &target_state));
    TEST_ASSERT_EQUAL_STRING("cmd-ln-99", cmd_id);
    TEST_ASSERT_TRUE(target_state);

    relay_set(target_state);
    TEST_ASSERT_TRUE(relay_get_state());
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_lenam_lm35_temperature_conversion);
    RUN_TEST(test_lenam_mic_peak_to_peak_calculation);
    RUN_TEST(test_lenam_relay_control_and_gpio);
    RUN_TEST(test_lenam_mqtt_telemetry_keys_and_values);
    RUN_TEST(test_lenam_mqtt_command_relay_execution);
    return UNITY_END();
}
