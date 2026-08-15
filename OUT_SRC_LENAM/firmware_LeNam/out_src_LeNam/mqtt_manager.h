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
    bool mqtt_manager_publish_sensor(float temperature_c,
                                     float sound_vpp);
    bool mqtt_manager_publish_relay(bool state, const char *changed_by);

#ifdef __cplusplus
}
#endif

#endif // MQTT_MANAGER_H
