#include "lM393.h"
#include <driver/gpio.h>
#include <Arduino.h>


void LM393_init(void){
    pinMode(LM393_PIN, INPUT);
}


bool get_state_LM393(){
    // Serial.println(digitalRead(LM393_PIN));
    if (digitalRead(LM393_PIN) == LOW){
        Serial.println("co vat can ir");
        return 1;
    }
    else{
        Serial.println("khong vat can ir");
        return 0;
    }
}


