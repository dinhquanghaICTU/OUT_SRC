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

static device_thresholds_t current_thresholds = {
    .voltage_min = 180.0f,
    .voltage_max = 245.0f,
    .current_max = 15.0f,
    .power_max = 3000.0f,
    .sample_interval_ms = SAMPLE_INTERVAL_MS};

static bool parse_json_number(const char *json, const char *key, float *out_val)
{
    if (!json || !key || !out_val)
        return false;

    char search_key[64];
    snprintf(search_key, sizeof(search_key), "\"%s\"", key);
    const char *pos = strstr(json, search_key);
    if (!pos)
        return false;

    pos = strchr(pos + strlen(search_key), ':');
    if (!pos)
        return false;

    pos++;
    while (*pos == ' ' || *pos == '\t' || *pos == '\r' || *pos == '\n')
        pos++;

    char *endptr = NULL;
    float val = strtof(pos, &endptr);
    if (endptr == pos)
        return false;

    *out_val = val;
    return true;
}

static bool parse_json_uint(const char *json, const char *key, uint32_t *out_val)
{
    float temp = 0.0f;
    if (parse_json_number(json, key, &temp))
    {
        if (temp > 0)
        {
            *out_val = (uint32_t)temp;
            return true;
        }
    }
    return false;
}

static void handle_config_payload(const char *payload, int len)
{
    if (!payload || len <= 0)
        return;

    char json_buf[512];
    if (len >= (int)sizeof(json_buf))
        len = sizeof(json_buf) - 1;
    memcpy(json_buf, payload, len);
    json_buf[len] = '\0';

    printf("[CONFIG] Received config: %s\n", json_buf);

    uint32_t interval = 0;
    if (parse_json_uint(json_buf, "sampling_interval_ms", &interval))
    {
        if (interval >= 500 && interval <= 3600000)
        {
            current_thresholds.sample_interval_ms = interval;
            printf("[CONFIG] Updated sample interval: %" PRIu32 " ms\n", interval);
        }
    }

    const char *v_pos = strstr(json_buf, "\"voltage_v\"");
    if (v_pos)
    {
        float v_min = 0, v_max = 0;
        if (parse_json_number(v_pos, "min", &v_min) && v_min > 0)
            current_thresholds.voltage_min = v_min;
        if (parse_json_number(v_pos, "max", &v_max) && v_max > 0)
            current_thresholds.voltage_max = v_max;
    }

    const char *i_pos = strstr(json_buf, "\"current_a\"");
    if (i_pos)
    {
        float i_max = 0;
        if (parse_json_number(i_pos, "max", &i_max) && i_max > 0)
            current_thresholds.current_max = i_max;
    }

    const char *p_pos = strstr(json_buf, "\"power_w\"");
    if (p_pos)
    {
        float p_max = 0;
        if (parse_json_number(p_pos, "max", &p_max) && p_max > 0)
            current_thresholds.power_max = p_max;
    }

    printf("[CONFIG] Active Thresholds -> V: [%.1f - %.1f] V | I_max: %.2f A | P_max: %.1f W | Rate: %" PRIu32 " ms\n",
           current_thresholds.voltage_min,
           current_thresholds.voltage_max,
           current_thresholds.current_max,
           current_thresholds.power_max,
           current_thresholds.sample_interval_ms);
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

        printf("[MQTT] Connected to broker successfully\n");
        printf("[MQTT] Subscribed to topic=%s (msg_id=%d)\n",
               MQTT_CONFIG_DESIRED_TOPIC,
               config_message_id);
        break;
    }

    case MQTT_EVENT_DISCONNECTED:
        mqtt_connected = false;
        printf("[MQTT] Disconnected from broker\n");
        break;

    case MQTT_EVENT_ERROR:
        mqtt_connected = false;
        printf("[MQTT] Broker connection error\n");
        break;

    case MQTT_EVENT_DATA:
    {
        printf("[MQTT] Data on topic: %.*s\n", event->topic_len, event->topic);
        if (event->topic_len == (int)strlen(MQTT_CONFIG_DESIRED_TOPIC) &&
            strncmp(event->topic, MQTT_CONFIG_DESIRED_TOPIC, event->topic_len) == 0)
        {
            handle_config_payload(event->data, event->data_len);
        }
        break;
    }
    default:
        break;
    }
}

void mqtt_manager_init(void)
{
    current_thresholds.voltage_min = 180.0f;
    current_thresholds.voltage_max = 245.0f;
    current_thresholds.current_max = 15.0f;
    current_thresholds.power_max = 3000.0f;
    current_thresholds.sample_interval_ms = SAMPLE_INTERVAL_MS;
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
        printf("[MQTT] Client initialization failed\n");
        return;
    }

    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);

    if (esp_mqtt_client_start(mqtt_client) != ESP_OK)
    {
        printf("[MQTT] Start client failed\n");
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
        return;
    }

    mqtt_started = true;
    printf("[MQTT] Connecting to broker: %s\n", broker_uri);
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
    printf("[MQTT] Client stopped\n");
}

bool mqtt_manager_is_connected(void)
{
    return mqtt_connected;
}

uint32_t mqtt_manager_get_sample_interval(void)
{
    return current_thresholds.sample_interval_ms > 0 ? current_thresholds.sample_interval_ms : 2000;
}

const device_thresholds_t *mqtt_manager_get_thresholds(void)
{
    return &current_thresholds;
}

alert_status_t mqtt_manager_check_thresholds(float currentA, float voltageV, float powerW)
{
    alert_status_t status = {
        .is_alert = false,
        .over_voltage = false,
        .under_voltage = false,
        .over_current = false,
        .over_power = false,
        .alert_msg = "normal"};

    // Ignore 0V offline/disconnected readings
    if (voltageV > 10.0f)
    {
        if (voltageV < current_thresholds.voltage_min)
        {
            status.under_voltage = true;
            status.is_alert = true;
            status.alert_msg = "under_voltage";
        }
        else if (voltageV > current_thresholds.voltage_max)
        {
            status.over_voltage = true;
            status.is_alert = true;
            status.alert_msg = "over_voltage";
        }
    }

    if (currentA > current_thresholds.current_max)
    {
        status.over_current = true;
        status.is_alert = true;
        status.alert_msg = "over_current";
    }

    if (powerW > current_thresholds.power_max)
    {
        status.over_power = true;
        status.is_alert = true;
        status.alert_msg = "over_power";
    }

    return status;
}

bool mqtt_manager_publish_sensor(float currentA, float voltageV)
{
    char payload[420];
    int length;
    const uint32_t sequence = ++telemetry_sequence;
    const uint64_t uptime_ms = (uint64_t)(esp_timer_get_time() / 1000);
    const float powerW = voltageV * currentA;
    const alert_status_t alert = mqtt_manager_check_thresholds(currentA, voltageV, powerW);

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
                      "\"current_a\":%.3f,"
                      "\"voltage_v\":%.2f,"
                      "\"power_w\":%.2f,"
                      "\"alert\":%s,"
                      "\"alert_msg\":\"%s\"}}",
                      PRODUCT_ID,
                      PRODUCT_ID,
                      mqtt_boot_id,
                      sequence,
                      sequence,
                      uptime_ms,
                      FIRMWARE_VERSION,
                      currentA,
                      voltageV,
                      powerW,
                      alert.is_alert ? "true" : "false",
                      alert.alert_msg);

    if (length <= 0 || length >= (int)sizeof(payload))
        return false;

    const int message_id =
        esp_mqtt_client_publish(mqtt_client, MQTT_TELEMETRY_TOPIC, payload, length, 1, 0);
    if (message_id < 0)
        return false;

    printf("[MQTT] Telemetry sent: V=%.1fV, I=%.3fA, P=%.1fW (Alert: %s)\n",
           voltageV, currentA, powerW, alert.alert_msg);
    return true;
}
