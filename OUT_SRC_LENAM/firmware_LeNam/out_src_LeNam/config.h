#ifndef __CONFIG_H__
#define __CONFIG_H__

#define WIFI_AP_PASSWORD "12345678"
#define WIFI_CONNECT_TIMEOUT_MS 45000UL
#define WIFI_CONNECT_MAX_ATTEMPTS 2U
#define WIFI_RETRY_INTERVAL_MS 15000UL
#define SAMPLE_INTERVAL_MS 2000
#define NAME_AP "DEVICE_LE_NAM"
#define PI_WIFI_SSID "ICTU_IOT_AP"
#define PI_WIFI_PASSWORD "12345678"

#define PRODUCT_ID "190782"
#define FIRMWARE_VERSION "1.0.0"

/*
    ip connect to mqtt server ->raspberry pi 


*/
#define MQTT_BROKER_URI "mqtt://192.168.4.1:1883"  // fallback; runtime lay gateway cua Pi AP
#define MQTT_TOPIC_PREFIX "iot/v1/devices/" PRODUCT_ID
#define MQTT_TELEMETRY_TOPIC MQTT_TOPIC_PREFIX "/telemetry"
#define MQTT_CONFIG_DESIRED_TOPIC MQTT_TOPIC_PREFIX "/config/desired"
#define MQTT_COMMAND_TOPIC MQTT_TOPIC_PREFIX "/commands"
#define MQTT_COMMAND_RESULT_TOPIC MQTT_TOPIC_PREFIX "/command-result"
#define MQTT_STATE_TOPIC MQTT_TOPIC_PREFIX "/state"


#endif //__CONFIG_H__
