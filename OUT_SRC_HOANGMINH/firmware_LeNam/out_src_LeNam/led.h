#ifndef __LED_H__
#define __LED_H__

#define LED_PIN 2

typedef enum
{
    LED_MODE_OFF = 0,
    LED_MODE_ON,
    LED_MODE_BLINK_SLOW,
    LED_MODE_BLINK_FAST
} led_mode_t;

#ifdef __cplusplus
extern "C"
{
#endif

    void init_led(void);
    void turn_on_led(void);
    void turn_off_led(void);
    void led_blink(int ms);
    void led_set_mode(led_mode_t mode);
    void led_update(void);

#ifdef __cplusplus
}
#endif

#endif //__LED_H__
