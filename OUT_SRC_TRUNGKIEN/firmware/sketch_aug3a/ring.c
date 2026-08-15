#include "ring.h"
#include <driver/gpio.h>

void init_ring(void)
{
    gpio_reset_pin((gpio_num_t)RING_PIN);
    gpio_set_direction((gpio_num_t)RING_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)RING_PIN, 0);
}

void turn_on_ring(void)
{
    gpio_set_level((gpio_num_t)RING_PIN, 1);
}

void turn_off_ring(void)
{
    gpio_set_level((gpio_num_t)RING_PIN, 0);
}
