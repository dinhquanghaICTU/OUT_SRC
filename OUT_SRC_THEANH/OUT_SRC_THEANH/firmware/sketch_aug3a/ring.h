#ifndef __RING_H__
#define __RING_H__

#include <stdbool.h>

// Chân còi cảnh báo D25 (GPIO 25)
#define RING_PIN 25

#ifdef __cplusplus
extern "C"
{
#endif

    void init_ring(void);
    void turn_on_ring(void);
    void turn_off_ring(void);
    void ring_set_state(bool state);
    bool ring_get_state(void);

#ifdef __cplusplus
}
#endif

#endif //__RING_H__
