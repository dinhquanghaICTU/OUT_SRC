#ifndef CONFIG_H
#define CONFIG_H

#define NAME_AP "TRUNGKIEN_DEVICE"
#define PI_WIFI_SSID "ICTU_IOT_AP"
#define PI_WIFI_PASSWORD "12345678"
#define WIFI_AP_PASSWORD "12345678"
#define WIFI_CONNECT_TIMEOUT_MS 45000UL
#define WIFI_CONNECT_MAX_ATTEMPTS 2U
#define WIFI_RETRY_INTERVAL_MS 15000UL
#define SAMPLE_INTERVAL_MS 2000

#define PRODUCT_ID "Tuananh-150304"
#define FIRMWARE_VERSION "1.0.0"

/* Sua URI nay thanh dia chi Raspberry Pi chay Mosquitto. */
#define MQTT_BROKER_URI "mqtt://192.168.4.1:1883" // fallback; runtime lay gateway cua Pi AP

#define MQTT_TOPIC_PREFIX "iot/v1/devices/" PRODUCT_ID
#define MQTT_PUB_TELEMETRY_TOPIC MQTT_TOPIC_PREFIX "/telemetry"
#define MQTT_SUB_CONFIG_DESIRED_TOPIC MQTT_TOPIC_PREFIX "/config/desired"

#endif // CONFIG_H
