#ifndef ICTU_IR_SENSOR_H
#define ICTU_IR_SENSOR_H

#include <stdbool.h>

void ir_sensor_begin(void);
bool ir_sensor_detected(void);
int ir_sensor_raw(void);

#endif // ICTU_IR_SENSOR_H
