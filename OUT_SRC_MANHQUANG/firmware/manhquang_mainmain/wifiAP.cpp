#include "wifiAP.h"
#include "config.h"

#include <Arduino.h>
#include <WiFi.h>

namespace
{
wifi_manager_state_t manager_state = WIFI_MANAGER_IDLE;
char mqtt_broker_uri[96] = MQTT_BROKER_URI;
unsigned long last_retry_ms = 0;
bool wifi_event_registered = false;
bool printed_connected = false;

void update_mqtt_broker_from_gateway()
{
    const IPAddress gateway = WiFi.gatewayIP();
    if (gateway == IPAddress(0, 0, 0, 0))
        return;

    snprintf(mqtt_broker_uri, sizeof(mqtt_broker_uri),
             "mqtt://%u.%u.%u.%u:1883",
             gateway[0], gateway[1], gateway[2], gateway[3]);
}

void begin_fixed_station(void)
{
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    WiFi.setMinSecurity(WIFI_AUTH_WPA_PSK);
    WiFi.begin(PI_WIFI_SSID, PI_WIFI_PASSWORD);

    manager_state = WIFI_MANAGER_CONNECTING;
    last_retry_ms = millis();
    printed_connected = false;

    Serial.print("[WiFi] Ket noi Pi AP: ");
    Serial.println(PI_WIFI_SSID);
}

void handle_wifi_event(arduino_event_id_t event, arduino_event_info_t info)
{
    if (event == ARDUINO_EVENT_WIFI_STA_CONNECTED)
    {
        Serial.println("[WiFi] Da associate voi Pi AP, cho DHCP cap IP");
        return;
    }

    if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP)
    {
        update_mqtt_broker_from_gateway();
        Serial.print("[WiFi] Da nhan IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("[MQTT] Broker tu gateway: ");
        Serial.println(mqtt_broker_uri);
        return;
    }

    if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED)
    {
        const wifi_err_reason_t reason = (wifi_err_reason_t)info.wifi_sta_disconnected.reason;
        Serial.print("[WiFi] Mat ket noi, reason: ");
        Serial.print((unsigned int)reason);
        Serial.print(" - ");
        Serial.println(WiFi.STA.disconnectReasonName(reason));
        printed_connected = false;
    }
}
} // namespace

void wifi_manager_begin(void)
{
    if (!wifi_event_registered)
    {
        WiFi.onEvent(handle_wifi_event);
        wifi_event_registered = true;
    }

    strlcpy(mqtt_broker_uri, MQTT_BROKER_URI, sizeof(mqtt_broker_uri));
    begin_fixed_station();
}

void wifi_manager_update(void)
{
    const unsigned long now_ms = millis();

    if (WiFi.status() == WL_CONNECTED)
    {
        manager_state = WIFI_MANAGER_CONNECTED;
        if (!printed_connected)
        {
            printed_connected = true;
            update_mqtt_broker_from_gateway();
            Serial.print("[WiFi] Connected Pi AP, IP: ");
            Serial.println(WiFi.localIP());
            Serial.print("[MQTT] Broker tu gateway: ");
            Serial.println(mqtt_broker_uri);
        }
        return;
    }

    manager_state = WIFI_MANAGER_CONNECTING;

    if (now_ms - last_retry_ms >= WIFI_RETRY_INTERVAL_MS)
    {
        last_retry_ms = now_ms;
        Serial.println("[WiFi] Thu ket noi lai Pi AP...");
        if (WiFi.status() == WL_DISCONNECTED)
            WiFi.reconnect();
        else
            WiFi.begin(PI_WIFI_SSID, PI_WIFI_PASSWORD);
    }
}

void wifi_manager_enter_config(void)
{
    Serial.println("[WiFi] Fixed Pi AP mode: reconnect");
    begin_fixed_station();
}

bool wifi_manager_is_connected(void)
{
    return WiFi.status() == WL_CONNECTED;
}

wifi_manager_state_t wifi_manager_get_state(void)
{
    if (WiFi.status() == WL_CONNECTED)
        return WIFI_MANAGER_CONNECTED;
    return manager_state;
}

const char *wifi_manager_get_ap_ssid(void)
{
    return PI_WIFI_SSID;
}

const char *wifi_manager_get_mqtt_broker_uri(void)
{
    return mqtt_broker_uri;
}

bool clearWiFiCredentials(void)
{
    Serial.println("[WiFi] Fixed Pi AP mode: khong co cau hinh flash de xoa");
    begin_fixed_station();
    return true;
}
