#ifndef WIFI_AP_H
#define WIFI_AP_H

#include <stdbool.h>

typedef enum
{
    WIFI_MANAGER_IDLE = 0,
    WIFI_MANAGER_AP_CONFIG,
    WIFI_MANAGER_CONNECTING,
    WIFI_MANAGER_CONNECTED
} wifi_manager_state_t;

#ifdef __cplusplus
extern "C"
{
#endif

    void wifi_manager_begin(void);
    void wifi_manager_update(void);
    void wifi_manager_enter_config(void);
    bool wifi_manager_is_connected(void);
    wifi_manager_state_t wifi_manager_get_state(void);
    const char *wifi_manager_get_ap_ssid(void);
    const char *wifi_manager_get_mqtt_broker_uri(void);

    bool clearWiFiCredentials(void);

#ifdef __cplusplus
}
#endif

#endif
