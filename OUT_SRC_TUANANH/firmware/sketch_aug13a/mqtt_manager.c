#include "mqtt_manager.h"
#include "config.h"
#include "relay.h"
#include "wifiAP.h"

#include <esp_random.h>
#include <esp_timer.h>
#include <inttypes.h>
#include <mqtt_client.h>
#include <stdio.h>
#include <string.h>

static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_started = false;
static bool mqtt_connected = false;
static uint32_t mqtt_boot_id = 0;
static uint32_t telemetry_sequence = 0;
static bool s_auto_mode = false;
static device_config_thresholds_t s_thresholds = {
    .lux_min = 50.0f,
    .lux_max = 500.0f,
    .sample_interval_ms = SAMPLE_INTERVAL_MS
};

static bool parse_json_double(const char *payload, const char *field, double *out)
{
    if (payload == NULL || field == NULL || out == NULL)
        return false;

    char key[64];
    snprintf(key, sizeof(key), "\"%s\"", field);
    const char *p = strstr(payload, key);
    if (!p)
        return false;
    p = strchr(p + strlen(key), ':');
    if (!p)
        return false;
    p++;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        p++;
    *out = atof(p);
    return true;
}

static bool parse_relay_state(const char *payload, bool *state)
{
    if (payload == NULL || state == NULL)
        return false;

    const char *key = strstr(payload, "\"state\"");
    if (key == NULL)
        return false;

    const char *value = strchr(key, ':');
    if (value == NULL)
        return false;

    ++value;
    while (*value == ' ' || *value == '\t' || *value == '\r' || *value == '\n')
        ++value;

    if (strncmp(value, "true", 4) == 0)
    {
        *state = true;
        return true;
    }

    if (strncmp(value, "false", 5) == 0)
    {
        *state = false;
        return true;
    }

    return false;
}

static bool parse_json_string(const char *payload,
                              const char *field,
                              char *output,
                              size_t output_size)
{
    char key[48];
    if (payload == NULL || field == NULL || output == NULL || output_size < 2)
        return false;

    const int key_length = snprintf(key, sizeof(key), "\"%s\"", field);
    if (key_length <= 0 || key_length >= (int)sizeof(key))
        return false;

    const char *cursor = strstr(payload, key);
    if (cursor == NULL || (cursor = strchr(cursor + key_length, ':')) == NULL)
        return false;

    ++cursor;
    while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n')
        ++cursor;
    if (*cursor++ != '"')
        return false;

    const char *end = strchr(cursor, '"');
    if (end == NULL || end == cursor || (size_t)(end - cursor) >= output_size)
        return false;

    memcpy(output, cursor, (size_t)(end - cursor));
    output[end - cursor] = '\0';
    return true;
}

static void publish_command_result(const char *command_id, bool state)
{
    char payload[192];
    const int length = snprintf(payload,
                                sizeof(payload),
                                "{\"command_id\":\"%s\","
                                "\"status\":\"succeeded\","
                                "\"state\":{\"relay\":%s}}",
                                command_id,
                                state ? "true" : "false");

    if (length > 0 && length < (int)sizeof(payload))
        esp_mqtt_client_publish(
            mqtt_client, MQTT_COMMAND_RESULT_TOPIC, payload, length, 1, 0);
}

bool mqtt_manager_publish_config_reported(uint32_t config_version)
{
    char topic[128];
    snprintf(topic, sizeof(topic), "iot/v1/devices/%s/config/reported", PRODUCT_ID);
    char payload[256];
    int len = snprintf(payload, sizeof(payload),
                       "{\"config_version\":%" PRIu32 ",\"status\":\"applied\","
                       "\"sampling_interval_ms\":%" PRIu32 ","
                       "\"thresholds\":{\"lux\":{\"min\":%.1f,\"max\":%.1f}}}",
                       config_version, s_thresholds.sample_interval_ms,
                       s_thresholds.lux_min, s_thresholds.lux_max);
    if (!mqtt_connected || mqtt_client == NULL || len <= 0) return false;
    return esp_mqtt_client_publish(mqtt_client, topic, payload, len, 1, 1) >= 0;
}

static void
mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    (void)handler_args;
    (void)base;
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
    {
        mqtt_connected = true;

        const int config_message_id =
            esp_mqtt_client_subscribe(mqtt_client, MQTT_CONFIG_DESIRED_TOPIC, 1);
        const int command_message_id =
            esp_mqtt_client_subscribe(mqtt_client, MQTT_COMMAND_TOPIC, 1);

        printf("[MQTT] Da ket noi broker\n");
        printf("[MQTT] Subscribe topic=%s, msg_id=%d\n",
               MQTT_CONFIG_DESIRED_TOPIC,
               config_message_id);
        printf("[MQTT] Subscribe topic=%s, msg_id=%d\n",
               MQTT_COMMAND_TOPIC,
               command_message_id);
        mqtt_manager_publish_relay(relay_get_state(), "startup");
        break;
    }

    case MQTT_EVENT_DISCONNECTED:
        mqtt_connected = false;
        printf("[MQTT] Mat ket noi broker\n");
        break;

    case MQTT_EVENT_ERROR:
        mqtt_connected = false;
        printf("[MQTT] Loi ket noi broker\n");
        break;
    case MQTT_EVENT_DATA:
    {
        printf("[MQTT] Topic: %.*s\n", event->topic_len, event->topic);
        printf("[MQTT] Payload: %.*s\n", event->data_len, event->data);

        // 1. Process Command Topic
        if (event->topic_len == (int)strlen(MQTT_COMMAND_TOPIC) &&
            memcmp(event->topic, MQTT_COMMAND_TOPIC, event->topic_len) == 0)
        {
            if (event->data_len >= 128 || event->data_len != event->total_data_len)
            {
                printf("[MQTT] Payload qua dai hoac bi chia nho\n");
                break;
            }

            char payload[128];
            char command_id[64];
            char command_type[32];

            memcpy(payload, event->data, event->data_len);
            payload[event->data_len] = '\0';

            bool cmd_state = false;
            if (!parse_json_string(payload, "command_id", command_id, sizeof(command_id)) ||
                !parse_json_string(payload, "type", command_type, sizeof(command_type)))
            {
                printf("[MQTT] Command khong hop le\n");
                break;
            }

            if (strcmp(command_type, "relay.set") == 0 && parse_relay_state(payload, &cmd_state))
            {
                relay_set(cmd_state);
                printf("[RELAY] Dieu khien tu server: %s\n", cmd_state ? "ON" : "OFF");
                mqtt_manager_publish_relay(cmd_state, "command");
                publish_command_result(command_id, cmd_state);
            }
            else if (strcmp(command_type, "auto.set") == 0 && parse_relay_state(payload, &cmd_state))
            {
                s_auto_mode = cmd_state;
                printf("[AUTO] Che do auto tu server: %s\n", cmd_state ? "ON" : "OFF");
                publish_command_result(command_id, cmd_state);
            }
            else
            {
                printf("[MQTT] Command type %s khong duoc ho tro\n", command_type);
            }
            break;
        }

        // 2. Process Config Desired Topic
        if (event->topic_len == (int)strlen(MQTT_CONFIG_DESIRED_TOPIC) &&
            memcmp(event->topic, MQTT_CONFIG_DESIRED_TOPIC, event->topic_len) == 0)
        {
            char payload[512];
            if (event->data_len >= (int)sizeof(payload)) break;
            memcpy(payload, event->data, event->data_len);
            payload[event->data_len] = '\0';
            printf("[CONFIG] Nhan cau hinh nguong tu server: %s\n", payload);

            double val = 0;
            if (parse_json_double(payload, "sampling_interval_ms", &val) && val >= 500) {
                s_thresholds.sample_interval_ms = (uint32_t)val;
            }
            if (parse_json_double(payload, "min", &val) || parse_json_double(payload, "warning_below", &val)) {
                s_thresholds.lux_min = (float)val;
            }
            if (parse_json_double(payload, "max", &val) || parse_json_double(payload, "warning_above", &val)) {
                s_thresholds.lux_max = (float)val;
            }

            const char *autoKey = strstr(payload, "\"auto_mode\"");
            if (autoKey != NULL) {
                bool newAuto = false;
                if (parse_relay_state(autoKey, &newAuto)) {
                    s_auto_mode = newAuto;
                    printf("[AUTO] Cap nhat auto_mode tu config: %s\n", s_auto_mode ? "ON" : "OFF");
                }
            }

            printf("[CONFIG] Da cap nhat nguong thanh cong: min=%.1f lux, max=%.1f lux, interval=%u ms, auto=%d\n",
                   s_thresholds.lux_min, s_thresholds.lux_max, (unsigned)s_thresholds.sample_interval_ms, s_auto_mode ? 1 : 0);

            double ver = 1;
            parse_json_double(payload, "config_version", &ver);
            mqtt_manager_publish_config_reported((uint32_t)ver);
            break;
        }
        break;
    }
    default:
        break;
    }
}

bool mqtt_manager_get_auto_mode(void)
{
    return s_auto_mode;
}

void mqtt_manager_set_auto_mode(bool auto_mode)
{
    s_auto_mode = auto_mode;
}

device_config_thresholds_t mqtt_manager_get_thresholds(void)
{
    return s_thresholds;
}

void mqtt_manager_set_thresholds(float min_lux, float max_lux, uint32_t interval_ms)
{
    s_thresholds.lux_min = min_lux;
    s_thresholds.lux_max = max_lux;
    if (interval_ms >= 500) s_thresholds.sample_interval_ms = interval_ms;
}

void mqtt_manager_start(void)
{
    if (mqtt_started)
        return;

    if (mqtt_boot_id == 0)
        mqtt_boot_id = esp_random();

    const char *broker_uri = wifi_manager_get_mqtt_broker_uri();
    const esp_mqtt_client_config_t config = {.broker.address.uri = broker_uri,
                                             .session.keepalive = 30,
                                             .network.reconnect_timeout_ms = 5000,
                                             .buffer.size = 1024,
                                             .buffer.out_size = 1024};

    mqtt_client = esp_mqtt_client_init(&config);
    if (mqtt_client == NULL)
    {
        printf("[MQTT] Khoi tao client that bai\n");
        return;
    }

    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);

    if (esp_mqtt_client_start(mqtt_client) != ESP_OK)
    {
        printf("[MQTT] Start client that bai\n");
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
        return;
    }

    mqtt_started = true;
    printf("[MQTT] Dang ket noi: %s\n", broker_uri);
}

void mqtt_manager_stop(void)
{
    if (!mqtt_started || mqtt_client == NULL)
        return;

    esp_mqtt_client_stop(mqtt_client);
    esp_mqtt_client_destroy(mqtt_client);
    mqtt_client = NULL;
    mqtt_started = false;
    mqtt_connected = false;
    printf("[MQTT] Da dung client\n");
}

bool mqtt_manager_is_connected(void)
{
    return mqtt_connected;
}

bool mqtt_manager_publish_sensor(bool detech, float luxx, bool relay_state)
{
    char payload[450];
    int length;
    const uint32_t sequence = ++telemetry_sequence;
    const uint64_t uptime_ms = (uint64_t)(esp_timer_get_time() / 1000);

    if (!mqtt_connected || mqtt_client == NULL)
        return false;

    const bool is_alert = (luxx > 0 && (luxx < s_thresholds.lux_min || luxx > s_thresholds.lux_max));
    const char *alert_msg = "normal";
    if (luxx > 0 && luxx < s_thresholds.lux_min) alert_msg = "lux_low";
    else if (luxx > 0 && luxx > s_thresholds.lux_max) alert_msg = "lux_high";

    length = snprintf(payload,
                      sizeof(payload),
                      "{\"schema_version\":1,"
                      "\"device_id\":\"%s\","
                      "\"message_id\":\"%s-%08" PRIx32 "-%" PRIu32 "\","
                      "\"sequence\":%" PRIu32 ","
                      "\"uptime_ms\":%" PRIu64 ","
                      "\"firmware_version\":\"%s\","
                      "\"metrics\":{"
                      "\"motion_detected\":%s,"
                      "\"detech\":%s,"
                      "\"light_lux\":%.2f,"
                      "\"lux\":%.2f,"
                      "\"relay_on\":%s,"
                      "\"relay\":%s,"
                      "\"alert\":%s,"
                      "\"alert_msg\":\"%s\"}}",
                      PRODUCT_ID,
                      PRODUCT_ID,
                      mqtt_boot_id,
                      sequence,
                      sequence,
                      uptime_ms,
                      FIRMWARE_VERSION,
                      detech ? "true" : "false",
                      detech ? "true" : "false",
                      luxx,
                      luxx,
                      relay_state ? "true" : "false",
                      relay_state ? "true" : "false",
                      is_alert ? "true" : "false",
                      alert_msg
                    );

    if (length <= 0 || length >= (int)sizeof(payload))
        return false;

    const int message_id =
        esp_mqtt_client_publish(mqtt_client, MQTT_TELEMETRY_TOPIC, payload, length, 1, 0);
    if (message_id < 0)
        return false;

    printf("[MQTT] Publish topic=%s, msg_id=%d\n", MQTT_TELEMETRY_TOPIC, message_id);
    printf("[MQTT] Payload: %s\n", payload);
    return true;
}



bool mqtt_manager_publish_relay(bool state, const char *changed_by)
{
    char payload[192];
    int length;

    if (!mqtt_connected || mqtt_client == NULL)
        return false;

    length = snprintf(payload,
                      sizeof(payload),
                      "{\"relay\":%s,"
                      "\"changed_by\":\"%s\","
                      "\"uptime_ms\":%" PRIu64 "}",
                      state ? "true" : "false",
                      changed_by,
                      (uint64_t)(esp_timer_get_time() / 1000));

    if (length <= 0 || length >= (int)sizeof(payload))
        return false;

    return esp_mqtt_client_publish(mqtt_client,
                                   MQTT_STATE_TOPIC,
                                   payload,
                                   length,
                                   1,
                                   1) >= 0;
}
