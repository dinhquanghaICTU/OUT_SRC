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
        float temp_min;
        float temp_max;
        float temp_warn;
        float pressure_min;
        float pressure_max;
        float ir_alarm_seconds;
        uint32_t sampling_interval_ms;
    } device_thresholds_t;

    typedef struct
    {
        bool active;
        bool temp_high;
        bool temp_low;
        bool pressure_high;
        bool pressure_low;
        bool ir_alert;
        char message[32];
    } alert_status_t;

    void mqtt_manager_init(void);
    void mqtt_manager_start(void);
    void mqtt_manager_stop(void);
    bool mqtt_manager_is_connected(void);

    uint32_t mqtt_manager_get_sample_interval(void);
    device_thresholds_t mqtt_manager_get_thresholds(void);
    alert_status_t mqtt_manager_check_thresholds(float temp_c, float pressure_hpa, bool ir_detected);

    bool mqtt_manager_publish_sensor(float temperature_c, float pressure_hpa,
                                     bool ir_detected, const alert_status_t *alert);
    bool mqtt_manager_publish_state(bool ring_state);

    bool mqtt_manager_get_ring_state(void);
    void mqtt_manager_set_ring_state(bool state);

#ifdef __cplusplus
}
#endif

#endif // MQTT_MANAGER_H
