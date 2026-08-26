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
        float voltage_min;
        float voltage_max;
        float current_max;
        float power_max;
        uint32_t sample_interval_ms;
    } device_thresholds_t;

    typedef struct
    {
        bool is_alert;
        bool over_voltage;
        bool under_voltage;
        bool over_current;
        bool over_power;
        const char *alert_msg;
    } alert_status_t;

    void mqtt_manager_init(void);
    void mqtt_manager_start(void);
    void mqtt_manager_stop(void);
    bool mqtt_manager_is_connected(void);
    uint32_t mqtt_manager_get_sample_interval(void);
    const device_thresholds_t *mqtt_manager_get_thresholds(void);
    alert_status_t mqtt_manager_check_thresholds(float currentA, float voltageV, float powerW);
    bool mqtt_manager_publish_sensor(float currentA, float voltageV);

#ifdef __cplusplus
}
#endif

#endif // MQTT_MANAGER_H
