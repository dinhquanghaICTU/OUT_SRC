#ifndef ICTU_BMP180_H
#define ICTU_BMP180_H

#include <stdbool.h>

bool bmp180_begin(void);
bool bmp180_is_ready(void);
bool bmp180_read(float *temperature_c, float *pressure_hpa);
void bmp180_simulate(float *temperature_c, float *pressure_hpa);

#endif // ICTU_BMP180_H
