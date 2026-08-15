#ifndef __RING_H__
#define __RING_H__

#define RING_PIN 4

#ifdef __cplusplus
extern "C"
{
#endif

    void init_ring(void);
    void turn_on_ring(void);
    void turn_off_ring(void);

#ifdef __cplusplus
}
#endif

#endif //__RING_H__
