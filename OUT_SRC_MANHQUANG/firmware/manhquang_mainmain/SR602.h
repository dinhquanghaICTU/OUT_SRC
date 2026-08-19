#ifndef __SR602_H__
#define __SR602_H__

#include <stdbool.h>

#define SR602_PIN 23


#ifdef __cplusplus
extern "C"
{
#endif
   void SR602_init();
   bool get_state_SR602();

#ifdef __cplusplus
}
#endif

#endif //__SR602_H__