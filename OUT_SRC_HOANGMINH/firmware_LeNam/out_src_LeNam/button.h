#ifndef BUTTON_H
#define BUTTON_H

#define BUTTON_PIN 13
#define BUTTON_ACTIVE_LEVEL 0
#define BUTTON_DEBOUNCE_MS 30
#define BUTTON_CONFIG_HOLD_MS 5000U

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void init_button(void);
    void update_button(void);
    bool button_is_pressed(void);
    bool button_was_pressed(void);
    bool button_was_held(uint32_t hold_ms);

#ifdef __cplusplus
}
#endif

#endif // BUTTON_H
