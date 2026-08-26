#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        float lux_min;
        float lux_max;
        uint32_t sample_interval_ms;
    } device_config_thresholds_t;

    void mqtt_manager_start(void);
    void mqtt_manager_stop(void);
    bool mqtt_manager_is_connected(void);
    bool mqtt_manager_publish_sensor(bool detech, float luxx, bool relay_state);  
    bool mqtt_manager_publish_relay(bool state, const char *changed_by);
    bool mqtt_manager_get_auto_mode(void);
    void mqtt_manager_set_auto_mode(bool auto_mode);
    device_config_thresholds_t mqtt_manager_get_thresholds(void);
    void mqtt_manager_set_thresholds(float min_lux, float max_lux, uint32_t interval_ms);
    bool mqtt_manager_publish_config_reported(uint32_t config_version);

#ifdef __cplusplus
}
#endif

#endif // MQTT_MANAGER_H
