#ifndef __ZMPT101B_H__
#define __ZMPT101B_H__

#include <stdbool.h>

#define ZMPT101B_PIN 35
#define ZMPT101B_ADC_REF_V 3.3F

#define ZMPT101B_ZERO_SAMPLE_COUNT 300
#define ZMPT101B_SAMPLE_COUNT 260
#define ZMPT101B_SAMPLE_DELAY_US 450

// Hệ số này phải calibrate theo module/biến trở thực tế.
// Đo điện áp thật bằng đồng hồ, rồi chỉnh sao cho get_voltage_v() khớp.
#define ZMPT101B_CALIBRATION 700.0F

// Debounce/lọc analog.
#define ZMPT101B_FILTER_ALPHA 0.25F
#define ZMPT101B_SENSOR_DEADBAND_V 0.003F
#define ZMPT101B_VOLTAGE_DEADBAND_V 1.0F

#ifdef __cplusplus
extern "C"
{
#endif

void zmpt101b_init();
bool zmpt101b_update();
float zmpt101b_get_voltage_v();
float zmpt101b_get_sensor_rms_v();
float zmpt101b_get_zero_voltage_v();

#ifdef __cplusplus
}
#endif

#endif // __ZMPT101B_H__
