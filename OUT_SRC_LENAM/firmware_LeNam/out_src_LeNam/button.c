#include "button.h"

#include <driver/gpio.h>
#include <esp_timer.h>

static int stable_level = !BUTTON_ACTIVE_LEVEL;
static int previous_raw_level = !BUTTON_ACTIVE_LEVEL;
static int64_t raw_change_time_ms = 0;
static int64_t pressed_time_ms = 0;
static bool pressed_event = false;
static bool held_event_reported = false;

static int64_t current_time_ms(void)
{
    return esp_timer_get_time() / 1000;
}

void init_button(void)
{
    gpio_reset_pin((gpio_num_t)BUTTON_PIN);
    gpio_set_direction((gpio_num_t)BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)BUTTON_PIN, GPIO_PULLUP_ONLY);

    stable_level = gpio_get_level((gpio_num_t)BUTTON_PIN);
    previous_raw_level = stable_level;
    raw_change_time_ms = current_time_ms();
    pressed_time_ms = 0;
    pressed_event = false;
    held_event_reported = false;
}

void update_button(void)
{
    const int raw_level = gpio_get_level((gpio_num_t)BUTTON_PIN);
    const int64_t now_ms = current_time_ms();

    if (raw_level != previous_raw_level)
    {
        previous_raw_level = raw_level;
        raw_change_time_ms = now_ms;
    }

    if (raw_level != stable_level && now_ms - raw_change_time_ms >= BUTTON_DEBOUNCE_MS)
    {
        stable_level = raw_level;

        if (stable_level == BUTTON_ACTIVE_LEVEL)
        {
            pressed_time_ms = now_ms;
            pressed_event = true;
            held_event_reported = false;
        }
        else
        {
            pressed_time_ms = 0;
            held_event_reported = false;
        }
    }
}

bool button_is_pressed(void)
{
    update_button();
    return stable_level == BUTTON_ACTIVE_LEVEL;
}

bool button_was_pressed(void)
{
    bool event;

    update_button();
    event = pressed_event;
    pressed_event = false;

    return event;
}

bool button_was_held(uint32_t hold_ms)
{
    update_button();

    if (stable_level != BUTTON_ACTIVE_LEVEL || pressed_time_ms == 0 || held_event_reported)
        return false;

    if ((uint64_t)(current_time_ms() - pressed_time_ms) < hold_ms)
        return false;

    held_event_reported = true;
    return true;
}

