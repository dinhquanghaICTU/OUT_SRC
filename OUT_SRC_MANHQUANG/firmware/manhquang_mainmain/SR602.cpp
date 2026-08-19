#include "SR602.h"
#include <driver/gpio.h>
#include <Arduino.h>

void SR602_init(){
    pinMode(SR602_PIN, INPUT);
}

bool get_state_SR602(){
    if (digitalRead(SR602_PIN) == HIGH){
        Serial.println("co va cham");
        return 1;
    } 
    else{
        Serial.println("khong co va cham");
        return 0;
    }
}