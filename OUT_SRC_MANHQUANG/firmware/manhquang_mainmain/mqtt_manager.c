#include "mqtt_manager.h"
#include "config.h"
#include "wifiAP.h"
#include "ULN2003.h"
#include "SR602.h"
#include "lM393.h"

#include <esp_random.h>
#include <esp_timer.h>
#include <inttypes.h>
#include <mqtt_client.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_started = false;
static bool mqtt_connected = false;
static uint32_t mqtt_boot_id = 0;
static uint32_t telemetry_sequence = 0;

static bool parse_json_string(const char *json, const char *key, char *out, size_t max_len)
{
    if (!json || !key || !out || max_len == 0) return false;
    char needle[64];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *p = strstr(json, needle);
    if (!p) return false;
    p += strlen(needle);
    while (*p && (*p == ' ' || *p == ':' || *p == '\t')) p++;
    if (*p != '\"') return false;
    p++;
    size_t i = 0;
    while (*p && *p != '\"' && i < max_len - 1) {
        out[i++] = *p++;
    }
    out[i] = '\0';
    return true;
}

static bool parse_bool_value(const char *json, const char *key, bool *val)
{
    if (!json || !key || !val) return false;
    char needle[64];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *p = strstr(json, needle);
    if (!p) return false;
    p += strlen(needle);
    while (*p && (*p == ' ' || *p == ':' || *p == '\t')) p++;
    if (strncmp(p, "true", 4) == 0) {
        *val = true;
        return true;
    }
    if (strncmp(p, "false", 5) == 0) {
        *val = false;
        return true;
    }
    return false;
}

static void publish_command_result(const char *command_id, const char *status, const char *door_state)
{
    char payload[256];
    int length;

    if (!mqtt_connected || mqtt_client == NULL || command_id == NULL)
        return;

    length = snprintf(payload,
                      sizeof(payload),
                      "{\"command_id\":\"%s\","
                      "\"status\":\"%s\","
                      "\"data\":{\"door_state\":\"%s\",\"position_pct\":%.1f},"
                      "\"uptime_ms\":%" PRIu64 "}",
                      command_id,
                      status ? status : "SUCCESS",
                      door_state ? door_state : "OK",
                      getDoorPositionPct(),
                      (uint64_t)(esp_timer_get_time() / 1000));

    if (length <= 0 || length >= (int)sizeof(payload))
        return;

    esp_mqtt_client_publish(mqtt_client,
                           MQTT_COMMAND_RESULT_TOPIC,
                           payload,
                           length,
                           1,
                           0);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    (void)handler_args;
    (void)base;
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
    {
        mqtt_connected = true;

        esp_mqtt_client_subscribe(mqtt_client, MQTT_CONFIG_DESIRED_TOPIC, 1);
        esp_mqtt_client_subscribe(mqtt_client, MQTT_COMMAND_TOPIC, 1);

        printf("[MQTT] Da ket noi broker thanh cong\n");
        printf("[MQTT] Subscribed topic lenh: %s\n", MQTT_COMMAND_TOPIC);

        const char *online_payload = "{\"online\":true,\"device_type\":\"smart_door\"}";
        esp_mqtt_client_publish(mqtt_client, MQTT_STATUS_TOPIC, online_payload, strlen(online_payload), 1, 1);

        mqtt_manager_publish_state(getDoorStateStr(), getDoorPositionPct(), "startup");
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
        printf("[MQTT] Nhan Topic: %.*s\n", event->topic_len, event->topic);
        printf("[MQTT] Payload: %.*s\n", event->data_len, event->data);

        // 1. Config desired topic
        if (event->topic_len == (int)strlen(MQTT_CONFIG_DESIRED_TOPIC) &&
            memcmp(event->topic, MQTT_CONFIG_DESIRED_TOPIC, event->topic_len) == 0)
        {
            printf("[MQTT] Nhan cau hinh config/desired tu Server\n");
            break;
        }

        // 2. Commands topic
        if (event->topic_len != (int)strlen(MQTT_COMMAND_TOPIC) ||
            memcmp(event->topic, MQTT_COMMAND_TOPIC, event->topic_len) != 0)
        {
            break;
        }

        char payload[256];
        int len = event->data_len < 255 ? event->data_len : 255;
        memcpy(payload, event->data, len);
        payload[len] = '\0';

        char command_id[64] = "cmd-default";
        char command_type[32] = "";
        char action[32] = "";

        parse_json_string(payload, "command_id", command_id, sizeof(command_id));
        parse_json_string(payload, "type", command_type, sizeof(command_type));
        parse_json_string(payload, "action", action, sizeof(action));

        bool relay_state = false;
        bool has_state = parse_bool_value(payload, "state", &relay_state);

        printf("[MQTT] Dang phan tich lenh: type='%s', action='%s', state=%s\n",
               command_type, action, has_state ? (relay_state ? "true" : "false") : "none");

        // Lệnh Mở cửa (open)
        if (strcmp(command_type, "door.open") == 0 ||
            strcmp(action, "open") == 0 ||
            strcmp(command_type, "open") == 0 ||
            (strcmp(command_type, "relay.set") == 0 && relay_state == true))
        {
            printf(">>> [DIEU KHIEN CUA] Nhan lenh tu Server: MO CUA (openDoor)\n");
            openDoor();
            publish_command_result(command_id, "SUCCESS", "OPEN");
            mqtt_manager_publish_state("OPEN", 100.0f, "command");
        }
        // Lệnh Đóng cửa (close)
        else if (strcmp(command_type, "door.close") == 0 ||
                 strcmp(action, "close") == 0 ||
                 strcmp(command_type, "close") == 0 ||
                 (strcmp(command_type, "relay.set") == 0 && relay_state == false))
        {
            printf(">>> [DIEU KHIEN CUA] Nhan lenh tu Server: DONG CUA (closeDoor)\n");
            closeDoor();
            publish_command_result(command_id, "SUCCESS", "CLOSED");
            mqtt_manager_publish_state("CLOSED", 0.0f, "command");
        }
        // Lệnh Giữ mở cửa (hold open)
        else if (strcmp(command_type, "door.hold_open") == 0 ||
                 strcmp(action, "hold_open") == 0 ||
                 strcmp(command_type, "hold_open") == 0)
        {
            printf(">>> [DIEU KHIEN CUA] Nhan lenh tu Server: GIU MO CUA (holdOpenDoor)\n");
            holdOpenDoor();
            publish_command_result(command_id, "SUCCESS", "HOLD_OPEN");
            mqtt_manager_publish_state("HOLD_OPEN", 100.0f, "command");
        }
        // Lệnh Dừng khẩn cấp (stop)
        else if (strcmp(command_type, "door.stop") == 0 ||
                 strcmp(action, "stop") == 0 ||
                 strcmp(command_type, "stop") == 0)
        {
            printf(">>> [DIEU KHIEN CUA] Nhan lenh tu Server: DUNG KHAN CAP (stopDoor)\n");
            stopDoor();
            publish_command_result(command_id, "SUCCESS", "STOP");
            mqtt_manager_publish_state("STOP", getDoorPositionPct(), "command");
        }
        else
        {
            printf("[MQTT] Lenh chua duoc ho tro: %s\n", command_type);
            publish_command_result(command_id, "UNKNOWN_COMMAND", getDoorStateStr());
        }

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
    printf("[MQTT] Dang ket noi broker: %s\n", broker_uri);
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

bool mqtt_manager_publish_door(bool motion_detected,
                               bool ir_blocked,
                               float door_position_pct,
                               float motor_speed_rpm,
                               int passage_count,
                               const char *door_state,
                               const char *motor_direction,
                               float temperature_c)
{
    char payload[512];
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
                      "\"motion_detected\":%s,"
                      "\"ir_blocked\":%s,"
                      "\"door_position_pct\":%.1f,"
                      "\"motor_speed_rpm\":%.1f,"
                      "\"passage_count\":%d,"
                      "\"door_state\":\"%s\","
                      "\"motor_direction\":\"%s\","
                      "\"temperature_c\":%.2f}}",
                      PRODUCT_ID,
                      PRODUCT_ID,
                      mqtt_boot_id,
                      sequence,
                      sequence,
                      uptime_ms,
                      FIRMWARE_VERSION,
                      motion_detected ? "true" : "false",
                      ir_blocked ? "true" : "false",
                      door_position_pct,
                      motor_speed_rpm,
                      passage_count,
                      door_state ? door_state : "CLOSED",
                      motor_direction ? motor_direction : "STOP",
                      temperature_c);

    if (length <= 0 || length >= (int)sizeof(payload))
        return false;

    const int message_id =
        esp_mqtt_client_publish(mqtt_client, MQTT_TELEMETRY_TOPIC, payload, length, 1, 0);

    if (message_id < 0)
        return false;

    printf("[MQTT] Telemetry: pos=%.1f%%, state=%s, pir=%d, ir=%d\n",
           door_position_pct, door_state, motion_detected, ir_blocked);
    return true;
}

bool mqtt_manager_publish_state(const char *door_state, float position_pct, const char *changed_by)
{
    char payload[256];
    int length;

    if (!mqtt_connected || mqtt_client == NULL)
        return false;

    length = snprintf(payload,
                      sizeof(payload),
                      "{\"door_state\":\"%s\","
                      "\"door_position_pct\":%.1f,"
                      "\"changed_by\":\"%s\","
                      "\"uptime_ms\":%" PRIu64 "}",
                      door_state ? door_state : "CLOSED",
                      position_pct,
                      changed_by ? changed_by : "system",
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
