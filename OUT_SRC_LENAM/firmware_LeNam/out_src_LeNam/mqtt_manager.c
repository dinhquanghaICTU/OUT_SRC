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

        if (event->topic_len != (int)strlen(MQTT_COMMAND_TOPIC) ||
            memcmp(event->topic, MQTT_COMMAND_TOPIC, event->topic_len) != 0)
        {
            break;
        }

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

        bool relay_state = false;
        if (!parse_json_string(payload, "command_id", command_id, sizeof(command_id)) ||
            !parse_json_string(payload, "type", command_type, sizeof(command_type)) ||
            strcmp(command_type, "relay.set") != 0 ||
            !parse_relay_state(payload, &relay_state))
        {
            printf("[MQTT] Command relay.set khong hop le\n");
            break;
        }

        relay_set(relay_state);

        printf(
            "[RELAY] Dieu khien tu server: %s\n",
            relay_state ? "ON" : "OFF"
        );

        mqtt_manager_publish_relay(relay_state, "command");
        publish_command_result(command_id, relay_state);
        break;
    }
    default:
        break;
    }
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

bool mqtt_manager_publish_sensor(float temperature_c, float sound_vpp)
{
    char payload[384];
    int length;
    const uint32_t sequence = ++telemetry_sequence;
    const uint64_t uptime_ms = (uint64_t)(esp_timer_get_time() / 1000);

    if (!mqtt_connected || mqtt_client == NULL)
        return false;

    length = snprintf(payload,
                      sizeof(payload),
                      "{\"schema_version\":1,"
                      "\"device_id\":\"%s\","
                      "\"message_id\":\"%s-%08" PRIx32 "-%" PRIu32 "\","
                      "\"sequence\":%" PRIu32 ","
                      "\"uptime_ms\":%" PRIu64 ","
                      "\"firmware_version\":\"%s\","
                      "\"metrics\":{"
                      "\"temperature_c\":%.2f,"
                      "\"sound_vpp\":%.3f}}",
                      PRODUCT_ID,
                      PRODUCT_ID,
                      mqtt_boot_id,
                      sequence,
                      sequence,
                      uptime_ms,
                      FIRMWARE_VERSION,
                      temperature_c,
                      sound_vpp);

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
