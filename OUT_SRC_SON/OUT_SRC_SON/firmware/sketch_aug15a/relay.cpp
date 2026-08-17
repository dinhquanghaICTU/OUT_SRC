#include "relay.h"
#include <driver/gpio.h>
#include <Arduino.h>

static bool current_relay_state = false;

void init_relay()
{
    gpio_reset_pin((gpio_num_t)RELAY_PIN);
    gpio_set_direction((gpio_num_t)RELAY_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)RELAY_PIN, 0);
    current_relay_state = false;
}

void relay_on()
{
    gpio_set_level((gpio_num_t)RELAY_PIN, 1);
    current_relay_state = true;
}



void relay_off()
{
    gpio_set_level((gpio_num_t)RELAY_PIN, 0);
    current_relay_state = false;
}

void relay_set(bool state)
{
    if (state)
        relay_on();
    else
        relay_off();
}

bool relay_get_state(void)
{
    return current_relay_state;
}
