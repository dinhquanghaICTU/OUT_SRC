#include "mqtt_manager.h"
#include "config.h"
#include "wifiAP.h"

#include <esp_random.h>
#include <esp_timer.h>
#include <inttypes.h>
#include <mqtt_client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_started = false;
static bool mqtt_connected = false;
static uint32_t mqtt_boot_id = 0;
static uint32_t telemetry_sequence = 0;

static uint32_t s_sampling_interval_ms = SAMPLE_INTERVAL_MS;
static float s_uv_warning_above = 15.0f;
static float s_uv_critical_above = 20.0f;
static float s_pressure_min = 500.0f;
static float s_pressure_max = 2000.0f;

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

uint32_t mqtt_manager_get_sampling_interval_ms(void)
{
    return s_sampling_interval_ms > 0 ? s_sampling_interval_ms : SAMPLE_INTERVAL_MS;
}

float mqtt_manager_get_uv_warning(void)
{
    return s_uv_warning_above;
}

float mqtt_manager_get_uv_critical(void)
{
    return s_uv_critical_above;
}

float mqtt_manager_get_pressure_min(void)
{
    return s_pressure_min;
}

float mqtt_manager_get_pressure_max(void)
{
    return s_pressure_max;
}

bool mqtt_manager_publish_config_reported(uint32_t config_version)
{
    char payload[384];
    int length;

    if (!mqtt_connected || mqtt_client == NULL)
        return false;

    length = snprintf(payload,
                      sizeof(payload),
                      "{\"schema_version\":1,"
                      "\"device_id\":\"%s\","
                      "\"config_version\":%" PRIu32 ","
                      "\"sampling_interval_ms\":%" PRIu32 ","
                      "\"thresholds\":{"
                      "\"uv_index\":{\"warning_above\":%.2f,\"critical_above\":%.2f},"
                      "\"pressure_hpa\":{\"min\":%.2f,\"max\":%.2f}}}",
                      PRODUCT_ID,
                      config_version,
                      s_sampling_interval_ms,
                      s_uv_warning_above,
                      s_uv_critical_above,
                      s_pressure_min,
                      s_pressure_max);

    if (length <= 0 || length >= (int)sizeof(payload))
        return false;

    const int message_id =
        esp_mqtt_client_publish(mqtt_client, MQTT_PUB_CONFIG_REPORTED_TOPIC, payload, length, 1, 1);

    if (message_id < 0)
        return false;

    printf("[MQTT] Reported config published msg_id=%d: %s\n", message_id, payload);
    return true;
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
        mqtt_connected = true;

        int message_id =
            esp_mqtt_client_subscribe(mqtt_client, MQTT_SUB_CONFIG_DESIRED_TOPIC, 1);

        printf("[MQTT] Da ket noi broker\n");
        printf("[MQTT] Subscribe topic=%s, msg_id=%d\n",
               MQTT_SUB_CONFIG_DESIRED_TOPIC,
               message_id);

        mqtt_manager_publish_config_reported(1);
        break;

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

        // Check if config desired topic
        if (event->topic_len == (int)strlen(MQTT_SUB_CONFIG_DESIRED_TOPIC) &&
            memcmp(event->topic, MQTT_SUB_CONFIG_DESIRED_TOPIC, event->topic_len) == 0)
        {
            char payload[512];
            int len = event->data_len < (int)(sizeof(payload) - 1) ? event->data_len : (int)(sizeof(payload) - 1);
            memcpy(payload, event->data, len);
            payload[len] = '\0';

            double val = 0;
            if (parse_json_double(payload, "sampling_interval_ms", &val))
            {
                if (val >= 500)
                    s_sampling_interval_ms = (uint32_t)val;
            }

            if (parse_json_double(payload, "warning_above", &val))
            {
                s_uv_warning_above = (float)val;
            }
            if (parse_json_double(payload, "critical_above", &val))
            {
                s_uv_critical_above = (float)val;
            }

            if (parse_json_double(payload, "min", &val))
            {
                s_pressure_min = (float)val;
            }
            if (parse_json_double(payload, "max", &val))
            {
                s_pressure_max = (float)val;
            }

            double ver = 1;
            parse_json_double(payload, "config_version", &ver);

            printf("[CONFIG] Nhan cau hinh: Interval=%" PRIu32 "ms, UV_Warn=%.2f, UV_Crit=%.2f, P_Min=%.2f, P_Max=%.2f\n",
                   s_sampling_interval_ms, s_uv_warning_above, s_uv_critical_above, s_pressure_min, s_pressure_max);

            mqtt_manager_publish_config_reported((uint32_t)ver);
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

bool mqtt_manager_publish_sensor(float uv_voltage, float uv_index, float pressure_hpa)
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
                      "\"uv_voltage\":%.3f,"
                      "\"uv_index\":%.2f,"
                      "\"pressure_hpa\":%.2f}}",
                      PRODUCT_ID,
                      PRODUCT_ID,
                      mqtt_boot_id,
                      sequence,
                      sequence,
                      uptime_ms,
                      FIRMWARE_VERSION,
                      uv_voltage,
                      uv_index,
                      pressure_hpa);

    if (length <= 0 || length >= (int)sizeof(payload))
        return false;

    const int message_id =
        esp_mqtt_client_publish(mqtt_client, MQTT_PUB_TELEMETRY_TOPIC, payload, length, 1, 0);

    if (message_id < 0)
        return false;

    printf("[MQTT] Publish topic=%s, msg_id=%d\n", MQTT_PUB_TELEMETRY_TOPIC, message_id);
    printf("[MQTT] Payload: %s\n", payload);
    return true;
}
