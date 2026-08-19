#ifndef __LM393_H__
#define __LM393_H__

#include <stdbool.h>

#define LM393_PIN 5


#ifdef __cplusplus
extern "C"
{
#endif
    void LM393_init(void);
    bool get_state_LM393();

#ifdef __cplusplus
}
#endif

#endif //__LM393_H__