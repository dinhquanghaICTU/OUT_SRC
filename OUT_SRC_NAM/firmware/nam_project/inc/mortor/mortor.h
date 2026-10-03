#ifndef __MORTOR_H__
#define __MORTOR_H__

#include <Arduino.h>
#include "esp32-hal-gpio.h"

#define MOTOR_PIN_IN1 25
#define MOTOR_PIN_IN2 26
#define MOTOR_PIN_ENA 27

#define ENCODER_PIN_A 18
#define ENCODER_PIN_B 19

// Thong so co khi tinh van toc (chinh theo dong co & banh xe thuc te):
#define ENCODER_PPR            11.0f   // So xung tren 1 vong rotor (JGA25-370 thuong la 11 xung)
#define GEAR_RATIO             34.0f   // Ty so truyen hop so (vi du 1:21.3, 1:34, 1:78... xem tren than motor)
#define WHEEL_DIAMETER_M       0.065f  // Duong kinh banh xe (met) - mac dinh 65mm = 0.065m

#ifdef __cplusplus
extern "C" {
#endif

void mortor_init(void);
void mortor_set_speed(int speed);
int  mortor_get_speed(void);
long mortor_get_encoder(void);
void mortor_reset_encoder(void);

float mortor_calculate_speed_mps(long delta_pulses, float dt_seconds);
float mortor_calculate_rpm(long delta_pulses, float dt_seconds);

#ifdef __cplusplus
}
#endif

#endif //__MORTOR_H__
