#ifndef __KNOCKSENSOR_H__
#define __KNOCKSENSOR_H__

#include <stdbool.h>

#define KNOCKSENSOR_PIN_DO 27
#define KNOCKSENSOR_PIN_SCK 14

#define HX710B_MOVING_AVG_WINDOW 5
#define HX710B_CAL_OFFSET 1013.25
#define HX710B_CAL_SCALE 0.00005

#ifdef __cplusplus
extern "C"
{
#endif

    void init_knock_sensor(void);
    bool read_pressure_sensor(long *raw_value,
                              double *filtered_raw,
                              float *pressure_hpa);

#ifdef __cplusplus
}
#endif

#endif //__KNOCKSENSOR_H__
