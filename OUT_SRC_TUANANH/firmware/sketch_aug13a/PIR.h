#ifndef __PIR_H__
#define __PIR_H__

#define PIR_PIN    13 
#define PIR_DEBOUNCE_MS 80UL
#define PIR_ACTIVE_LEVEL HIGH

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C"
{
    
#endif

void pir_init();
int pir_read_raw();
bool pir_update();
bool pir_is_detected();

#ifdef __cplusplus
}
#endif

#endif //__PIR_H__