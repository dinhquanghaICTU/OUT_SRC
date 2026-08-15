#ifndef __LM35_H__
#define __LM35_H__



#define LM35_PIN 32
#define SAMPLE_COUNT 10

#ifdef __cplusplus
extern "C"
{
    
#endif

    // void mqtt_manager_start(void);
    // void mqtt_manager_stop(void);
    // bool mqtt_manager_is_connected(void);
    // bool mqtt_manager_publish_sensor(float uv_voltage,
    //                                  float uv_index,
    //                                  float pressure_hpa);

    // void test_mqtt();
    void init_lm35();
    float readLm35Voltage();


#ifdef __cplusplus
}
#endif

#endif //__LM35_H__