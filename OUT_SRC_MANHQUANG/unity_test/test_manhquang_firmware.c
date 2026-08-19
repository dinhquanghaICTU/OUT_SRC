#include "unity.h"
#include "mock_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define PRODUCT_ID "manhquang-190782"
#define FIRMWARE_VERSION "1.0.0"

#define SR602_PIN 23
#define LM393_PIN 5
#define OPEN_STEPS 1024

// Stepper / Door State Machine Mock
typedef struct {
    int position;
    int direction;
    int passage_count;
    bool is_held_open;
} door_mock_t;

void door_init(door_mock_t *d) {
    d->position = 0;
    d->direction = 0;
    d->passage_count = 0;
    d->is_held_open = false;
}

float door_get_pct(door_mock_t *d) {
    return ((float)d->position / (float)OPEN_STEPS) * 100.0f;
}

const char* door_get_state(door_mock_t *d, bool ir_blocked) {
    if (ir_blocked && d->direction == -1) return "OBSTACLE_STOP";
    if (d->position >= OPEN_STEPS) return d->is_held_open ? "HOLD_OPEN" : "OPEN";
    if (d->position <= 0) return "CLOSED";
    if (d->direction == 1) return "OPENING";
    if (d->direction == -1) return "CLOSING";
    return "STOP";
}

void door_open(door_mock_t *d) {
    d->direction = 1;
    d->position = OPEN_STEPS;
    d->direction = 0;
}

void door_close(door_mock_t *d, bool ir_blocked) {
    if (ir_blocked) {
        // Obstacle reverse
        d->direction = 1;
        d->position = OPEN_STEPS;
        d->direction = 0;
        return;
    }
    d->direction = -1;
    d->position = 0;
    d->direction = 0;
    d->passage_count++;
}

int mqtt_build_telemetry(char *buf, size_t max_len, uint32_t seq, uint32_t boot_id,
                         bool motion, bool ir, float pos_pct, float speed, int passages,
                         const char *door_state, const char *motor_dir, float temp) {
    return snprintf(buf, max_len,
        "{\"schema_version\":1,"
        "\"device_id\":\"%s\","
        "\"message_id\":\"%s-%08x-%u\","
        "\"sequence\":%u,"
        "\"uptime_ms\":%llu,"
        "\"firmware_version\":\"%s\","
        "\"metrics\":{"
        "\"motion_detected\":%s,"
        "\"ir_blocked\":%s,"
        "\"door_position_pct\":%.1f,"
        "\"motor_speed_rpm\":%.1f,"
        "\"passage_count\":%d,"
        "\"door_state\":\"%s\","
        "\"motor_direction\":\"%s\","
        "\"temperature_c\":%.2f}}",
        PRODUCT_ID, PRODUCT_ID, boot_id, seq, seq,
        (unsigned long long)mock_hal_get_time_ms(), FIRMWARE_VERSION,
        motion ? "true" : "false",
        ir ? "true" : "false",
        pos_pct, speed, passages,
        door_state, motor_dir, temp);
}

void setUp(void) {
    mock_hal_reset();
}

void tearDown(void) {}

void test_manhquang_door_open_close_cycles(void) {
    door_mock_t door;
    door_init(&door);

    TEST_ASSERT_EQUAL_INT(0, door.position);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, door_get_pct(&door));
    TEST_ASSERT_EQUAL_STRING("CLOSED", door_get_state(&door, false));

    // Open door
    door_open(&door);
    TEST_ASSERT_EQUAL_INT(OPEN_STEPS, door.position);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, door_get_pct(&door));
    TEST_ASSERT_EQUAL_STRING("OPEN", door_get_state(&door, false));

    // Close door without obstacle
    door_close(&door, false);
    TEST_ASSERT_EQUAL_INT(0, door.position);
    TEST_ASSERT_EQUAL_STRING("CLOSED", door_get_state(&door, false));
    TEST_ASSERT_EQUAL_INT(1, door.passage_count);
}

void test_manhquang_anti_pinch_obstacle_reverse(void) {
    door_mock_t door;
    door_init(&door);
    door_open(&door);

    // Close with obstacle (LM393 blocked) -> Auto reverse to OPEN
    door_close(&door, true);
    TEST_ASSERT_EQUAL_INT(OPEN_STEPS, door.position);
    TEST_ASSERT_EQUAL_STRING("OPEN", door_get_state(&door, false));
    TEST_ASSERT_EQUAL_INT(0, door.passage_count); // Passage not completed
}

void test_manhquang_mqtt_telemetry_schema(void) {
    char payload[512];
    mock_hal_set_time_ms(12000);
    int len = mqtt_build_telemetry(payload, sizeof(payload), 10, 0x123456, true, false, 100.0f, 12.0f, 3, "OPEN", "CW", 28.5f);
    TEST_ASSERT_TRUE(len > 0);

    TEST_ASSERT_TRUE(json_has_key(payload, "schema_version"));
    TEST_ASSERT_TRUE(json_has_key(payload, "device_id"));
    TEST_ASSERT_TRUE(json_has_key(payload, "motion_detected"));
    TEST_ASSERT_TRUE(json_has_key(payload, "ir_blocked"));
    TEST_ASSERT_TRUE(json_has_key(payload, "door_position_pct"));
    TEST_ASSERT_TRUE(json_has_key(payload, "passage_count"));
    TEST_ASSERT_TRUE(json_has_key(payload, "door_state"));

    char dev_id[64];
    json_extract_string(payload, "device_id", dev_id, sizeof(dev_id));
    TEST_ASSERT_EQUAL_STRING(PRODUCT_ID, dev_id);

    double pos_val;
    json_extract_number(payload, "door_position_pct", &pos_val);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, (float)pos_val);

    bool motion_val;
    json_extract_bool(payload, "motion_detected", &motion_val);
    TEST_ASSERT_TRUE(motion_val);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_manhquang_door_open_close_cycles);
    RUN_TEST(test_manhquang_anti_pinch_obstacle_reverse);
    RUN_TEST(test_manhquang_mqtt_telemetry_schema);
    return UNITY_END();
}
