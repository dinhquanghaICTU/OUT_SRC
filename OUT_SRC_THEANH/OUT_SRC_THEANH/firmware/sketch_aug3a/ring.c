#include "ring.h"
#include <driver/gpio.h>

static bool s_ring_active = false;

void init_ring(void)
{
    gpio_reset_pin((gpio_num_t)RING_PIN);
    gpio_set_direction((gpio_num_t)RING_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)RING_PIN, 0);
    s_ring_active = false;
}

void turn_on_ring(void)
{
    gpio_set_level((gpio_num_t)RING_PIN, 1);
    s_ring_active = true;
}

void turn_off_ring(void)
{
    gpio_set_level((gpio_num_t)RING_PIN, 0);
    s_ring_active = false;
}

void ring_set_state(bool state)
{
    if (state) {
        turn_on_ring();
    } else {
        turn_off_ring();
    }
}

bool ring_get_state(void)
{
    return s_ring_active;
}
