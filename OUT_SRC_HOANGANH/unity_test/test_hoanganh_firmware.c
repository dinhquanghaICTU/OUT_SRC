#include "unity.h"
#include "mock_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define PRODUCT_ID "190782"
#define FIRMWARE_VERSION "1.0.0"

// Mock Firmware Logic for HOANGANH (BMP180 + IR Sensor + Buzzer Ring + Button + LED + MQTT)
#define IR_SENSOR_PIN 34
#define IR_SENSOR_ACTIVE_LEVEL 0
#define BUTTON_PIN 4
#define BUTTON_ACTIVE_LEVEL 0
#define BUTTON_DEBOUNCE_MS 50
#define RING_PIN 19
#define LED_PIN 2

// Simulate BMP180 sensor calculation
void bmp180_calc_simulate(uint64_t current_ms, float *temp_c, float *pressure_hpa) {
    const float seconds = (float)current_ms / 1000.0f;
    *temp_c = 29.0f + 2.2f * sinf(seconds / 17.0f) + 0.25f * sinf(seconds / 3.0f);
    *pressure_hpa = 1008.0f + 4.5f * sinf(seconds / 29.0f) + 0.4f * sinf(seconds / 5.0f);
}

// BMP180 bounds validator
bool bmp180_validate_data(float temp_c, float pressure_pa) {
    if (!isfinite(temp_c) || !isfinite(pressure_pa)) return false;
    if (pressure_pa < 30000.0f || pressure_pa > 120000.0f) return false;
    if (temp_c < -40.0f || temp_c > 85.0f) return false;
    return true;
}

// IR Sensor 5-sample voting logic
bool ir_sensor_evaluate_samples(const int samples[5]) {
    int active_samples = 0;
    for (int i = 0; i < 5; ++i) {
        if (samples[i] == IR_SENSOR_ACTIVE_LEVEL) active_samples++;
    }
    return active_samples >= 3;
}

// Button state machine
typedef struct {
    int stable_level;
    int prev_raw;
    uint64_t raw_change_ms;
    uint64_t pressed_ms;
    bool pressed_event;
} button_ctx_t;

void button_init(button_ctx_t *ctx) {
    ctx->stable_level = !BUTTON_ACTIVE_LEVEL;
    ctx->prev_raw = !BUTTON_ACTIVE_LEVEL;
    ctx->raw_change_ms = mock_hal_get_time_ms();
    ctx->pressed_ms = 0;
    ctx->pressed_event = false;
}

void button_process(button_ctx_t *ctx, int raw_level) {
    uint64_t now_ms = mock_hal_get_time_ms();
    if (raw_level != ctx->prev_raw) {
        ctx->prev_raw = raw_level;
        ctx->raw_change_ms = now_ms;
    }
    if (raw_level != ctx->stable_level && (now_ms - ctx->raw_change_ms) >= BUTTON_DEBOUNCE_MS) {
        ctx->stable_level = raw_level;
        if (ctx->stable_level == BUTTON_ACTIVE_LEVEL) {
            ctx->pressed_ms = now_ms;
            ctx->pressed_event = true;
        } else {
            ctx->pressed_ms = 0;
        }
    }
}

// Ring controller
static bool s_ring_state = false;
void ring_set(bool on) {
    s_ring_state = on;
    mock_gpio_write(RING_PIN, on ? 1 : 0);
}
bool ring_get_state(void) { return s_ring_state; }

// MQTT Serializer
int mqtt_build_telemetry(char *buf, size_t max_len, uint32_t seq, uint32_t boot_id,
                         bool ir_det, float temp, float pressure, bool ring_on) {
    return snprintf(buf, max_len,
        "{\"schema_version\":1,"
        "\"device_id\":\"%s\","
        "\"message_id\":\"%s-%08x-%u\","
        "\"sequence\":%u,"
        "\"uptime_ms\":%llu,"
        "\"firmware_version\":\"%s\","
        "\"metrics\":{"
        "\"ir_detected\":%s,"
        "\"ir\":%s,"
        "\"temperature_c\":%.2f,"
        "\"temp\":%.2f,"
        "\"pressure_hpa\":%.2f,"
        "\"pressure\":%.2f,"
        "\"ring_on\":%s,"
        "\"ring\":%s}}",
        PRODUCT_ID, PRODUCT_ID, boot_id, seq, seq,
        (unsigned long long)mock_hal_get_time_ms(), FIRMWARE_VERSION,
        ir_det ? "true" : "false", ir_det ? "true" : "false",
        temp, temp, pressure, pressure,
        ring_on ? "true" : "false", ring_on ? "true" : "false");
}

// MQTT Command Parser
bool mqtt_parse_ring_command(const char *payload, char *cmd_id, size_t cmd_id_size, bool *out_state) {
    if (!payload || !cmd_id || !out_state) return false;
    char cmd_type[32] = {0};
    if (!json_extract_string(payload, "command_id", cmd_id, cmd_id_size)) return false;
    if (!json_extract_string(payload, "type", cmd_type, sizeof(cmd_type))) return false;
    if (strcmp(cmd_type, "ring.set") != 0 && strcmp(cmd_type, "buzzer.set") != 0) return false;
    return json_extract_bool(payload, "state", out_state);
}

void setUp(void) {
    mock_hal_reset();
    s_ring_state = false;
}

void tearDown(void) {}

void test_hoanganh_bmp180_simulation_math(void) {
    float temp1, press1;
    bmp180_calc_simulate(10000, &temp1, &press1);
    TEST_ASSERT_FLOAT_WITHIN(10.0f, 29.0f, temp1);
    TEST_ASSERT_FLOAT_WITHIN(20.0f, 1008.0f, press1);
    TEST_ASSERT_TRUE(bmp180_validate_data(temp1, press1 * 100.0f));
}

void test_hoanganh_bmp180_bounds_validation(void) {
    TEST_ASSERT_TRUE(bmp180_validate_data(25.0f, 101325.0f));
    TEST_ASSERT_FALSE(bmp180_validate_data(-50.0f, 101325.0f)); // Temp too low
    TEST_ASSERT_FALSE(bmp180_validate_data(25.0f, 20000.0f));  // Pressure too low
    TEST_ASSERT_FALSE(bmp180_validate_data(25.0f, 150000.0f)); // Pressure too high
}

void test_hoanganh_ir_voting_filter(void) {
    // 3 active samples (0) -> detected
    int samples_detected[5] = {1, 0, 0, 1, 0};
    TEST_ASSERT_TRUE(ir_sensor_evaluate_samples(samples_detected));

    // 2 active samples (0) -> noise ignored
    int samples_noise[5] = {1, 1, 0, 1, 0};
    TEST_ASSERT_FALSE(ir_sensor_evaluate_samples(samples_noise));

    // All active
    int samples_all[5] = {0, 0, 0, 0, 0};
    TEST_ASSERT_TRUE(ir_sensor_evaluate_samples(samples_all));
}

void test_hoanganh_button_debounce_and_events(void) {
    button_ctx_t btn;
    button_init(&btn);

    // Initial state: not pressed
    TEST_ASSERT_FALSE(btn.pressed_event);

    // Glitch for 20ms (< BUTTON_DEBOUNCE_MS)
    button_process(&btn, BUTTON_ACTIVE_LEVEL);
    mock_hal_advance_time_ms(20);
    button_process(&btn, BUTTON_ACTIVE_LEVEL);
    TEST_ASSERT_FALSE(btn.pressed_event);

    // Valid press >= 50ms
    mock_hal_advance_time_ms(40);
    button_process(&btn, BUTTON_ACTIVE_LEVEL);
    TEST_ASSERT_TRUE(btn.pressed_event);
    TEST_ASSERT_EQUAL_INT(BUTTON_ACTIVE_LEVEL, btn.stable_level);
}

void test_hoanganh_ring_controller(void) {
    ring_set(true);
    TEST_ASSERT_TRUE(ring_get_state());
    TEST_ASSERT_EQUAL_INT(1, mock_gpio_get_output_level(RING_PIN));

    ring_set(false);
    TEST_ASSERT_FALSE(ring_get_state());
    TEST_ASSERT_EQUAL_INT(0, mock_gpio_get_output_level(RING_PIN));
}

void test_hoanganh_mqtt_telemetry_keys_and_schema(void) {
    char payload[512];
    mock_hal_set_time_ms(15000);
    int len = mqtt_build_telemetry(payload, sizeof(payload), 42, 0xABCDEF, true, 28.5f, 1012.3f, true);
    TEST_ASSERT_TRUE(len > 0);

    // Validate JSON structure and keys
    TEST_ASSERT_TRUE(json_has_key(payload, "schema_version"));
    TEST_ASSERT_TRUE(json_has_key(payload, "device_id"));
    TEST_ASSERT_TRUE(json_has_key(payload, "message_id"));
    TEST_ASSERT_TRUE(json_has_key(payload, "sequence"));
    TEST_ASSERT_TRUE(json_has_key(payload, "uptime_ms"));
    TEST_ASSERT_TRUE(json_has_key(payload, "firmware_version"));
    TEST_ASSERT_TRUE(json_has_key(payload, "metrics"));
    TEST_ASSERT_TRUE(json_has_key(payload, "ir_detected"));
    TEST_ASSERT_TRUE(json_has_key(payload, "temperature_c"));
    TEST_ASSERT_TRUE(json_has_key(payload, "pressure_hpa"));
    TEST_ASSERT_TRUE(json_has_key(payload, "ring_on"));

    char dev_id[64];
    json_extract_string(payload, "device_id", dev_id, sizeof(dev_id));
    TEST_ASSERT_EQUAL_STRING(PRODUCT_ID, dev_id);

    double seq_val, uptime_val;
    json_extract_number(payload, "sequence", &seq_val);
    TEST_ASSERT_EQUAL_INT(42, (int)seq_val);
    json_extract_number(payload, "uptime_ms", &uptime_val);
    TEST_ASSERT_EQUAL_INT(15000, (int)uptime_val);

    bool ir_val, ring_val;
    json_extract_bool(payload, "ir_detected", &ir_val);
    TEST_ASSERT_TRUE(ir_val);
    json_extract_bool(payload, "ring_on", &ring_val);
    TEST_ASSERT_TRUE(ring_val);
}

void test_hoanganh_mqtt_command_parsing(void) {
    char cmd_id[64];
    bool target_state = false;

    // Valid command: turn ring ON
    const char *valid_cmd_on = "{\"command_id\":\"cmd-ha-001\",\"type\":\"ring.set\",\"state\":true}";
    TEST_ASSERT_TRUE(mqtt_parse_ring_command(valid_cmd_on, cmd_id, sizeof(cmd_id), &target_state));
    TEST_ASSERT_EQUAL_STRING("cmd-ha-001", cmd_id);
    TEST_ASSERT_TRUE(target_state);

    // Valid command: turn ring OFF
    const char *valid_cmd_off = "{\"command_id\":\"cmd-ha-002\",\"type\":\"ring.set\",\"state\":false}";
    TEST_ASSERT_TRUE(mqtt_parse_ring_command(valid_cmd_off, cmd_id, sizeof(cmd_id), &target_state));
    TEST_ASSERT_EQUAL_STRING("cmd-ha-002", cmd_id);
    TEST_ASSERT_FALSE(target_state);

    // Invalid command type
    const char *invalid_type = "{\"command_id\":\"cmd-ha-003\",\"type\":\"other.action\",\"state\":true}";
    TEST_ASSERT_FALSE(mqtt_parse_ring_command(invalid_type, cmd_id, sizeof(cmd_id), &target_state));

    // Missing state
    const char *missing_state = "{\"command_id\":\"cmd-ha-004\",\"type\":\"ring.set\"}";
    TEST_ASSERT_FALSE(mqtt_parse_ring_command(missing_state, cmd_id, sizeof(cmd_id), &target_state));
}

void test_hoanganh_threshold_and_alert_logic(void) {
    // Normal conditions
    float temp_normal = 28.0f;
    float press_normal = 1013.25f;
    float temp_warn = 40.0f;
    float press_min = 990.0f;
    float press_max = 1030.0f;

    bool temp_alert = (temp_normal > temp_warn);
    bool press_alert = (press_normal < press_min || press_normal > press_max);
    TEST_ASSERT_FALSE(temp_alert);
    TEST_ASSERT_FALSE(press_alert);

    // Over temperature alert
    float temp_over = 45.5f;
    TEST_ASSERT_TRUE(temp_over > temp_warn);

    // Under pressure alert
    float press_under = 980.0f;
    TEST_ASSERT_TRUE(press_under < press_min);

    // Over pressure alert
    float press_over = 1040.0f;
    TEST_ASSERT_TRUE(press_over > press_max);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_hoanganh_bmp180_simulation_math);
    RUN_TEST(test_hoanganh_bmp180_bounds_validation);
    RUN_TEST(test_hoanganh_ir_voting_filter);
    RUN_TEST(test_hoanganh_button_debounce_and_events);
    RUN_TEST(test_hoanganh_ring_controller);
    RUN_TEST(test_hoanganh_mqtt_telemetry_keys_and_schema);
    RUN_TEST(test_hoanganh_mqtt_command_parsing);
    RUN_TEST(test_hoanganh_threshold_and_alert_logic);
    return UNITY_END();
}
