#include "mqtt_manager.h"
#include "config.h"
#include "wifiAP.h"
#include "ring.h"

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
static bool s_manual_ring_state = false;

// Default thresholds for weather_pressure
static device_thresholds_t s_thresholds = {
    .temp_min = 0.0f,
    .temp_max = 50.0f,
    .temp_warn = 40.0f,
    .pressure_min = 990.0f,
    .pressure_max = 1030.0f,
    .ir_alarm_seconds = 1.0f,
    .sampling_interval_ms = SAMPLE_INTERVAL_MS
};

// Simple JSON extraction helper functions
static bool json_find_key(const char *json, const char *key, const char **val_start)
{
    if (!json || !key || !val_start)
        return false;
    char search_buf[64];
    snprintf(search_buf, sizeof(search_buf), "\"%s\"", key);
    const char *pos = strstr(json, search_buf);
    if (!pos)
        return false;
    pos += strlen(search_buf);
    while (*pos == ' ' || *pos == '\t' || *pos == '\r' || *pos == '\n' || *pos == ':')
        pos++;
    *val_start = pos;
    return true;
}

static bool json_get_double(const char *json, const char *key, float *out_val)
{
    const char *val_ptr = NULL;
    if (!json_find_key(json, key, &val_ptr))
        return false;
    char *endptr = NULL;
    float v = strtof(val_ptr, &endptr);
    if (endptr == val_ptr)
        return false;
    *out_val = v;
    return true;
}

static bool json_get_int(const char *json, const char *key, int *out_val)
{
    const char *val_ptr = NULL;
    if (!json_find_key(json, key, &val_ptr))
        return false;
    char *endptr = NULL;
    long v = strtol(val_ptr, &endptr, 10);
    if (endptr == val_ptr)
        return false;
    *out_val = (int)v;
    return true;
}

static bool json_get_bool(const char *json, const char *key, bool *out_val)
{
    const char *val_ptr = NULL;
    if (!json_find_key(json, key, &val_ptr))
        return false;
    if (strncmp(val_ptr, "true", 4) == 0 || *val_ptr == '1')
    {
        *out_val = true;
        return true;
    }
    if (strncmp(val_ptr, "false", 5) == 0 || *val_ptr == '0')
    {
        *out_val = false;
        return true;
    }
    return false;
}

static bool json_get_string(const char *json, const char *key, char *out_str, size_t max_len)
{
    const char *val_ptr = NULL;
    if (!json_find_key(json, key, &val_ptr))
        return false;
    if (*val_ptr == '\"')
        val_ptr++;
    size_t i = 0;
    while (*val_ptr && *val_ptr != '\"' && *val_ptr != ',' && *val_ptr != '}' && i < max_len - 1)
    {
        out_str[i++] = *val_ptr++;
    }
    out_str[i] = '\0';
    return i > 0;
}

static void parse_config_desired(const char *payload, int len)
{
    char json[1024];
    if (len >= (int)sizeof(json))
        len = sizeof(json) - 1;
    memcpy(json, payload, len);
    json[len] = '\0';

    int interval_ms = 0;
    if (json_get_int(json, "sampling_interval_ms", &interval_ms) && interval_ms >= 500 && interval_ms <= 3600000)
    {
        s_thresholds.sampling_interval_ms = (uint32_t)interval_ms;
    }
    else
    {
        float interval_sec = 0.0f;
        if (json_get_double(json, "sampling_interval_seconds", &interval_sec) && interval_sec >= 0.5f)
        {
            s_thresholds.sampling_interval_ms = (uint32_t)(interval_sec * 1000.0f);
        }
    }

    float val = 0.0f;
    if (json_get_double(json, "temperature_warn_c", &val) || json_get_double(json, "warning_above", &val))
    {
        s_thresholds.temp_warn = val;
    }
    if (json_get_double(json, "temp_min", &val))
        s_thresholds.temp_min = val;
    if (json_get_double(json, "temp_max", &val))
        s_thresholds.temp_max = val;
    if (json_get_double(json, "pressure_min_hpa", &val) || json_get_double(json, "pressure_min", &val))
        s_thresholds.pressure_min = val;
    if (json_get_double(json, "pressure_max_hpa", &val) || json_get_double(json, "pressure_max", &val))
        s_thresholds.pressure_max = val;
    if (json_get_double(json, "ir_alarm_seconds", &val))
        s_thresholds.ir_alarm_seconds = val;

    printf("[CONFIG] Applied: interval=%u ms, temp_warn=%.1f C, press_min=%.1f, press_max=%.1f\n",
           (unsigned)s_thresholds.sampling_interval_ms,
           s_thresholds.temp_warn,
           s_thresholds.pressure_min,
           s_thresholds.pressure_max);
}

static void parse_command(const char *payload, int len)
{
    char json[512];
    if (len >= (int)sizeof(json))
        len = sizeof(json) - 1;
    memcpy(json, payload, len);
    json[len] = '\0';

    char cmd_id[64] = {0};
    char cmd_type[64] = {0};
    bool next_state = false;

    json_get_string(json, "command_id", cmd_id, sizeof(cmd_id));
    json_get_string(json, "type", cmd_type, sizeof(cmd_type));

    bool has_state = json_get_bool(json, "state", &next_state);
    if (!has_state)
    {
        const char *params = strstr(json, "\"params\"");
        if (params)
            json_get_bool(params, "state", &next_state);
    }

    printf("[COMMAND] ID=%s, Type=%s, State=%s\n", cmd_id, cmd_type, next_state ? "ON" : "OFF");

    if (strstr(cmd_type, "ring") || strstr(cmd_type, "relay") || strstr(cmd_type, "buzzer") || strstr(cmd_type, "actuator"))
    {
        s_manual_ring_state = next_state;
        if (next_state)
            turn_on_ring();
        else
            turn_off_ring();

        mqtt_manager_publish_state(next_state);

        if (cmd_id[0] != '\0')
        {
            char res_buf[256];
            snprintf(res_buf, sizeof(res_buf),
                     "{\"command_id\":\"%s\",\"status\":\"succeeded\",\"state\":{\"ring\":%s,\"relay\":%s}}",
                     cmd_id, next_state ? "true" : "false", next_state ? "true" : "false");
            esp_mqtt_client_publish(mqtt_client, MQTT_COMMAND_RESULT_TOPIC, res_buf, 0, 1, 0);
        }
    }
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
        printf("[MQTT] Da ket noi broker: %s\n", PRODUCT_ID);

        esp_mqtt_client_subscribe(mqtt_client, MQTT_CONFIG_DESIRED_TOPIC, 1);
        esp_mqtt_client_subscribe(mqtt_client, MQTT_COMMAND_TOPIC, 1);

        // Publish status online
        {
            char status_buf[128];
            snprintf(status_buf, sizeof(status_buf),
                     "{\"online\":true,\"firmware_version\":\"%s\",\"device_type\":\"weather_pressure\"}",
                     FIRMWARE_VERSION);
            esp_mqtt_client_publish(mqtt_client, MQTT_STATUS_TOPIC, status_buf, 0, 1, 1);
        }

        // Publish initial state
        mqtt_manager_publish_state(s_manual_ring_state);
        break;

    case MQTT_EVENT_DISCONNECTED:
        mqtt_connected = false;
        printf("[MQTT] Mat ket noi broker\n");
        break;

    case MQTT_EVENT_ERROR:
        mqtt_connected = false;
        printf("[MQTT] Loi broker\n");
        break;

    case MQTT_EVENT_DATA:
    {
        printf("[MQTT] Received topic: %.*s\n", event->topic_len, event->topic);
        if (event->topic_len > 0 && strstr(event->topic, "config/desired"))
        {
            parse_config_desired(event->data, event->data_len);
        }
        else if (event->topic_len > 0 && strstr(event->topic, "commands"))
        {
            parse_command(event->data, event->data_len);
        }
        break;
    }
    default:
        break;
    }
}

void mqtt_manager_init(void)
{
    s_thresholds.sampling_interval_ms = SAMPLE_INTERVAL_MS;
    s_thresholds.temp_min = 0.0f;
    s_thresholds.temp_max = 50.0f;
    s_thresholds.temp_warn = 40.0f;
    s_thresholds.pressure_min = 990.0f;
    s_thresholds.pressure_max = 1030.0f;
    s_thresholds.ir_alarm_seconds = 1.0f;
    s_manual_ring_state = false;
}

void mqtt_manager_start(void)
{
    if (mqtt_started)
        return;

    if (mqtt_boot_id == 0)
        mqtt_boot_id = esp_random();

    const char *broker_uri = wifi_manager_get_mqtt_broker_uri();
    const esp_mqtt_client_config_t config = {
        .broker.address.uri = broker_uri,
        .session.keepalive = 30,
        .network.reconnect_timeout_ms = 5000,
        .buffer.size = 1024,
        .buffer.out_size = 1024
    };

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

uint32_t mqtt_manager_get_sample_interval(void)
{
    return s_thresholds.sampling_interval_ms;
}

device_thresholds_t mqtt_manager_get_thresholds(void)
{
    return s_thresholds;
}

alert_status_t mqtt_manager_check_thresholds(float temp_c, float pressure_hpa, bool ir_detected)
{
    alert_status_t alert = {0};
    alert.active = false;
    alert.temp_high = (temp_c > s_thresholds.temp_warn || temp_c > s_thresholds.temp_max);
    alert.temp_low = (temp_c < s_thresholds.temp_min);
    alert.pressure_high = (pressure_hpa > s_thresholds.pressure_max);
    alert.pressure_low = (pressure_hpa < s_thresholds.pressure_min);
    alert.ir_alert = ir_detected;

    if (alert.temp_high)
    {
        alert.active = true;
        strncpy(alert.message, "over_temperature", sizeof(alert.message) - 1);
    }
    else if (alert.temp_low)
    {
        alert.active = true;
        strncpy(alert.message, "under_temperature", sizeof(alert.message) - 1);
    }
    else if (alert.pressure_high)
    {
        alert.active = true;
        strncpy(alert.message, "over_pressure", sizeof(alert.message) - 1);
    }
    else if (alert.pressure_low)
    {
        alert.active = true;
        strncpy(alert.message, "under_pressure", sizeof(alert.message) - 1);
    }
    else if (alert.ir_alert)
    {
        alert.active = true;
        strncpy(alert.message, "ir_detected", sizeof(alert.message) - 1);
    }
    else
    {
        strncpy(alert.message, "normal", sizeof(alert.message) - 1);
    }

    return alert;
}

bool mqtt_manager_publish_sensor(float temperature_c, float pressure_hpa,
                                 bool ir_detected, const alert_status_t *alert)
{
    char payload[512];
    int length;
    const uint32_t sequence = ++telemetry_sequence;
    const uint64_t uptime_ms = (uint64_t)(esp_timer_get_time() / 1000);

    if (!mqtt_connected || mqtt_client == NULL)
        return false;

    const bool has_alert = (alert != NULL && alert->active);
    const char *alert_msg = (alert != NULL) ? alert->message : "normal";

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
                      "\"pressure_hpa\":%.2f,"
                      "\"ir_detected\":%d,"
                      "\"ring_on\":%s,"
                      "\"alert\":%s,"
                      "\"alert_msg\":\"%s\"}}",
                      PRODUCT_ID,
                      PRODUCT_ID,
                      mqtt_boot_id,
                      sequence,
                      sequence,
                      uptime_ms,
                      FIRMWARE_VERSION,
                      temperature_c,
                      pressure_hpa,
                      ir_detected ? 1 : 0,
                      s_manual_ring_state ? "true" : "false",
                      has_alert ? "true" : "false",
                      alert_msg);

    if (length <= 0 || length >= (int)sizeof(payload))
        return false;

    const int message_id =
        esp_mqtt_client_publish(mqtt_client, MQTT_TELEMETRY_TOPIC, payload, length, 1, 0);
    if (message_id < 0)
        return false;

    printf("[MQTT] Telemetry -> msg_id=%d | T=%.1f C, P=%.1f hPa, IR=%d, Alert=%s\n",
           message_id, temperature_c, pressure_hpa, ir_detected ? 1 : 0, alert_msg);
    return true;
}

bool mqtt_manager_publish_state(bool ring_state)
{
    if (!mqtt_connected || mqtt_client == NULL)
        return false;

    char payload[128];
    snprintf(payload, sizeof(payload), "{\"ring\":%s,\"relay\":%s}",
             ring_state ? "true" : "false", ring_state ? "true" : "false");
    return esp_mqtt_client_publish(mqtt_client, MQTT_STATE_TOPIC, payload, 0, 1, 1) >= 0;
}

bool mqtt_manager_get_ring_state(void)
{
    return s_manual_ring_state;
}

void mqtt_manager_set_ring_state(bool state)
{
    s_manual_ring_state = state;
}
