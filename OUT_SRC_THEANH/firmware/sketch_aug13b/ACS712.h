#ifndef __ACS712_H__
#define __ACS712_H__

#include <stdbool.h>

#define ACS712_PIN 34

// ESP32 ADC reference after analogReadMilliVolts calibration.
#define ACS712_ADC_REF_V 3.3F

// Chọn đúng loại ACS712 của mày:
// 5A  = 0.185 V/A
// 20A = 0.100 V/A
// 30A = 0.066 V/A
#define ACS712_SENSITIVITY_V_PER_A 0.100F

#define ACS712_ZERO_SAMPLE_COUNT 300
#define ACS712_SAMPLE_COUNT 220
#define ACS712_SAMPLE_DELAY_US 450

// Debounce/lọc analog: alpha càng nhỏ càng mượt nhưng phản hồi chậm hơn.
#define ACS712_FILTER_ALPHA 0.25F
#define ACS712_CURRENT_DEADBAND_A 0.03F
#define ACS712_VOLTAGE_DEADBAND_V 0.01F

#ifdef __cplusplus
extern "C"
{
#endif

void acs712_init();
bool acs712_update();
float acs712_get_current_a();
float acs712_get_sensor_voltage_v();
float acs712_get_zero_voltage_v();

#ifdef __cplusplus
}
#endif

#endif // __ACS712_H__
