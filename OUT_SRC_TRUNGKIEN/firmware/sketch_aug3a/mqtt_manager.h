#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void mqtt_manager_start(void);
    void mqtt_manager_stop(void);
    bool mqtt_manager_is_connected(void);
    bool mqtt_manager_publish_sensor(float uv_voltage,
                                     float uv_index,
                                     float pressure_hpa);
    bool mqtt_manager_publish_config_reported(uint32_t config_version);

    uint32_t mqtt_manager_get_sampling_interval_ms(void);
    float mqtt_manager_get_uv_warning(void);
    float mqtt_manager_get_uv_critical(void);
    float mqtt_manager_get_pressure_min(void);
    float mqtt_manager_get_pressure_max(void);

    void test_mqtt();

#ifdef __cplusplus
}
#endif

#endif // MQTT_MANAGER_H
