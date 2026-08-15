#ifndef __RELAY_H__
#define __RELAY_H__

#include <stdbool.h>

#define RELAY_PIN 5

#ifdef __cplusplus
extern "C"
{
    
#endif

void init_relay();
void relay_on();
void relay_off();
void relay_set(bool state);
bool relay_get_state(void);

#ifdef __cplusplus
}
#endif


#endif //__RELAY_H__
