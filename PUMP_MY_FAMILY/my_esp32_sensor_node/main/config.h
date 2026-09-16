#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>

#define TRIG_PIN 5
#define ECHO_PIN 18

#define BATTERY_ADC_PIN 34
// Tỉ lệ cầu phân áp điện trở: (R1 + R2) / R2
// Mặc định 2 điện trở bằng nhau (VD: R1=100k, R2=100k) -> 2.0f
// Khi pin 4.2V -> ADC nhận 2.1V. Điện áp pin = V_adc * 2.0
#define BATTERY_DIVIDER_RATIO 2.0f

#define STATUS_LED_PIN 2

#define ESP_NOW_WIFI_CHANNEL 1

#define USE_BROADCAST_MAC 1

#define SENSOR_SEND_INTERVAL_MS 2000

#endif // __CONFIG_H__
