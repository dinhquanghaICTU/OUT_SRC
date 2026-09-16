#include "buzzer.h"
#include <Arduino.h>

static bool s_buzzer_state = false;

void buzzer_init(void) {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, BUZZER_OFF_LEVEL);
    s_buzzer_state = false;
}

void buzzer_set(bool state) {
    if (s_buzzer_state == state)
        return;
    s_buzzer_state = state;
    digitalWrite(BUZZER_PIN, state ? BUZZER_ON_LEVEL : BUZZER_OFF_LEVEL);
    Serial.printf("[BUZZER] Pin D%d -> %s (Muc logic: %d)\r\n", 
                  BUZZER_PIN, 
                  state ? "ON (KIEU)" : "OFF (TAT)", 
                  state ? BUZZER_ON_LEVEL : BUZZER_OFF_LEVEL);
}

void buzzer_on(void) {
    buzzer_set(true);
}

void buzzer_off(void) {
    buzzer_set(false);
}

bool buzzer_get_state(void) {
    return s_buzzer_state;
}
