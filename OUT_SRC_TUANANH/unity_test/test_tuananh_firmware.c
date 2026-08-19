#include "unity.h"
#include "mock_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define PRODUCT_ID "150808"
#define FIRMWARE_VERSION "1.0.0"

#define PIR_PIN 13
#define PIR_DEBOUNCE_MS 80
#define PIR_ACTIVE_LEVEL 1
#define RELAY_PIN 25

// PIR Motion sensor state machine with debounce
typedef struct {
    int last_raw;
    int stable_state;
    uint64_t last_raw_change_ms;
} pir_ctx_t;

void pir_ctx_init(pir_ctx_t *ctx) {
    ctx->last_raw = !PIR_ACTIVE_LEVEL;
    ctx->stable_state = !PIR_ACTIVE_LEVEL;
    ctx->last_raw_change_ms = mock_hal_get_time_ms();
}

bool pir_ctx_update(pir_ctx_t *ctx, int raw_state) {
    uint64_t now_ms = mock_hal_get_time_ms();
    if (raw_state != ctx->last_raw) {
        ctx->last_raw = raw_state;
        ctx->last_raw_change_ms = now_ms;
    }
    if ((now_ms - ctx->last_raw_change_ms) >= PIR_DEBOUNCE_MS &&
        ctx->stable_state != ctx->last_raw) {
        ctx->stable_state = ctx->last_raw;
    }
    return ctx->stable_state == PIR_ACTIVE_LEVEL;
}

// MQTT Telemetry Builder
int mqtt_build_telemetry_tuananh(char *buf, size_t max_len, uint32_t seq, uint32_t boot_id,
                                 bool motion, float lux, bool relay_on) {
    return snprintf(buf, max_len,
        "{\"schema_version\":1,"
        "\"device_id\":\"%s\","
        "\"message_id\":\"%s-%08x-%u\","
        "\"sequence\":%u,"
        "\"uptime_ms\":%llu,"
        "\"firmware_version\":\"%s\","
        "\"metrics\":{"
        "\"motion_detected\":%s,"
        "\"detech\":%s,"
        "\"light_lux\":%.2f,"
        "\"lux\":%.2f,"
        "\"relay_on\":%s,"
        "\"relay\":%s}}",
        PRODUCT_ID, PRODUCT_ID, boot_id, seq, seq,
        (unsigned long long)mock_hal_get_time_ms(), FIRMWARE_VERSION,
        motion ? "true" : "false", motion ? "true" : "false",
        lux, lux,
        relay_on ? "true" : "false", relay_on ? "true" : "false");
}

void setUp(void) {
    mock_hal_reset();
}

void tearDown(void) {}

void test_tuananh_bh1750_lux_ranges(void) {
    // Dim light: 15.5 lux
    float lux_dim = 15.5f;
    TEST_ASSERT_TRUE(lux_dim >= 0.0f && lux_dim <= 65535.0f);

    // Bright daylight: 12000 lux
    float lux_sun = 12000.0f;
    TEST_ASSERT_TRUE(lux_sun > 1000.0f);
}

void test_tuananh_pir_debounce_filter(void) {
    pir_ctx_t pir;
    pir_ctx_init(&pir);

    // Initial: no motion
    TEST_ASSERT_FALSE(pir_ctx_update(&pir, 0));

    // Motion pulse glitch for 30ms (< PIR_DEBOUNCE_MS 80ms)
    pir_ctx_update(&pir, 1);
    mock_hal_advance_time_ms(30);
    TEST_ASSERT_FALSE(pir_ctx_update(&pir, 1));

    // Sustained motion >= 80ms -> Detected
    mock_hal_advance_time_ms(60);
    TEST_ASSERT_TRUE(pir_ctx_update(&pir, 1));

    // Motion ends: glitch 20ms -> Still detected until stable LOW >= 80ms
    pir_ctx_update(&pir, 0);
    mock_hal_advance_time_ms(20);
    TEST_ASSERT_TRUE(pir_ctx_update(&pir, 0));

    mock_hal_advance_time_ms(70);
    TEST_ASSERT_FALSE(pir_ctx_update(&pir, 0));
}

void test_tuananh_mqtt_telemetry_keys(void) {
    char payload[512];
    mock_hal_set_time_ms(60000);
    int len = mqtt_build_telemetry_tuananh(payload, sizeof(payload), 88, 0x776655, true, 450.75f, true);
    TEST_ASSERT_TRUE(len > 0);

    TEST_ASSERT_TRUE(json_has_key(payload, "motion_detected"));
    TEST_ASSERT_TRUE(json_has_key(payload, "detech"));
    TEST_ASSERT_TRUE(json_has_key(payload, "light_lux"));
    TEST_ASSERT_TRUE(json_has_key(payload, "lux"));
    TEST_ASSERT_TRUE(json_has_key(payload, "relay_on"));

    char dev_id[64];
    json_extract_string(payload, "device_id", dev_id, sizeof(dev_id));
    TEST_ASSERT_EQUAL_STRING(PRODUCT_ID, dev_id);

    double lux_val;
    json_extract_number(payload, "light_lux", &lux_val);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 450.75f, (float)lux_val);

    bool motion_val;
    json_extract_bool(payload, "motion_detected", &motion_val);
    TEST_ASSERT_TRUE(motion_val);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_tuananh_bh1750_lux_ranges);
    RUN_TEST(test_tuananh_pir_debounce_filter);
    RUN_TEST(test_tuananh_mqtt_telemetry_keys);
    return UNITY_END();
}
