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
    
    // Gửi Telemetry trạng thái cửa thông minh lên MQTT
    bool mqtt_manager_publish_door(bool motion_detected,
                                   bool ir_blocked,
                                   float door_position_pct,
                                   float motor_speed_rpm,
                                   int passage_count,
                                   const char *door_state,
                                   const char *motor_direction,
                                   float temperature_c);

    bool mqtt_manager_publish_state(const char *door_state, float position_pct, const char *changed_by);

#ifdef __cplusplus
}
#endif

#endif // MQTT_MANAGER_H
