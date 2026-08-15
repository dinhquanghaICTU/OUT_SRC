#ifndef PUMP_PERIPHERAL_H
#define PUMP_PERIPHERAL_H

#include <stdbool.h>

#ifndef WATER_FLOW_PIN
#define WATER_FLOW_PIN 27
#endif

#ifdef __cplusplus
extern "C" {
#endif

void pump_peripheral_init(void);
void pump_peripheral_update(void);

float pump_get_flow_l_min(void);
float pump_get_total_liters(void);
unsigned long pump_get_pulse_count(void);

bool pump_get_relay_state(void);
void pump_set_relay(bool on);
void pump_toggle_relay(void);

#ifdef __cplusplus
}
#endif

#endif // PUMP_PERIPHERAL_H
