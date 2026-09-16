#ifndef __BUZZER_H__
#define __BUZZER_H__

#include <stdbool.h>

#define BUZZER_PIN 25

// Module còi 3 chân thông dụng dùng kích mức LOW (0 = KÊU, 1 = TẮT)
#define BUZZER_ACTIVE_LOW 1

#define BUZZER_ON_LEVEL  (BUZZER_ACTIVE_LOW ? 0 : 1)
#define BUZZER_OFF_LEVEL (BUZZER_ACTIVE_LOW ? 1 : 0)

#ifdef __cplusplus
extern "C" {
#endif

void buzzer_init(void);
void buzzer_set(bool state);
void buzzer_on(void);
void buzzer_off(void);
bool buzzer_get_state(void);

#ifdef __cplusplus
}
#endif

#endif // __BUZZER_H__
