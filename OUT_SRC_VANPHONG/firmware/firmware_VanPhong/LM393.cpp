#include "LM393.h"
#include <driver/gpio.h>
#include <Arduino.h>


void LM393_init(){
    pinMode(LM393_PIN, INPUT);
}


float get_data_LM393(){
    return digitalRead(LM393_PIN);
}
