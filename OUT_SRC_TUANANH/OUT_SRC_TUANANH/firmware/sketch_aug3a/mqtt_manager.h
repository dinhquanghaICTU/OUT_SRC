#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <stdbool.h>

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

    void test_mqtt();

#ifdef __cplusplus
}
#endif

#endif // MQTT_MANAGER_H
