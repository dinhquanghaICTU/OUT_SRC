#include "led.h"
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_timer.h>

static led_mode_t current_mode = LED_MODE_OFF;
static int led_level = 0;
static int64_t previous_toggle_ms = 0;

static int64_t led_time_ms(void)
{
    return esp_timer_get_time() / 1000;
}

void init_led(void)
{
    gpio_reset_pin((gpio_num_t)LED_PIN);
    gpio_set_direction((gpio_num_t)LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)LED_PIN, 0);
}

void turn_on_led(void)
{
    led_level = 1;
    gpio_set_level((gpio_num_t)LED_PIN, 1);
}

void turn_off_led(void)
{
    led_level = 0;
    gpio_set_level((gpio_num_t)LED_PIN, 0);
}

void led_blink(int ms)
{
    turn_on_led();
    vTaskDelay(pdMS_TO_TICKS(ms));
    turn_off_led();
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void led_set_mode(led_mode_t mode)
{
    if (current_mode == mode)
        return;

    current_mode = mode;
    previous_toggle_ms = led_time_ms();

    if (mode == LED_MODE_ON)
        turn_on_led();
    else
        turn_off_led();
}

void led_update(void)
{
    int interval_ms;
    const int64_t now_ms = led_time_ms();

    if (current_mode == LED_MODE_OFF || current_mode == LED_MODE_ON)
        return;

    interval_ms = current_mode == LED_MODE_BLINK_FAST ? 250 : 700;

    if (now_ms - previous_toggle_ms < interval_ms)
        return;

    previous_toggle_ms = now_ms;

    if (led_level)
        turn_off_led();
    else
        turn_on_led();
}
