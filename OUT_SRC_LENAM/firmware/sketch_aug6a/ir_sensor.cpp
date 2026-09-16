#include "ir_sensor.h"
#include "config.h"

#include <Arduino.h>

void ir_sensor_begin(void)
{
    // GPIO34 không hỗ trợ INPUT_PULLUP trên ESP32.
    pinMode(IR_SENSOR_PIN, INPUT);
    Serial.printf("[IR] San sang GPIO%d, active level=%d\n",
                  IR_SENSOR_PIN, IR_SENSOR_ACTIVE_LEVEL);
}

int ir_sensor_raw(void)
{
    return digitalRead(IR_SENSOR_PIN);
}

bool ir_sensor_detected(void)
{
    // Lấy đa số 5 mẫu để loại xung nhiễu ngắn trên chân OUT.
    int activeSamples = 0;
    for (int sample = 0; sample < 5; ++sample) {
        if (digitalRead(IR_SENSOR_PIN) == IR_SENSOR_ACTIVE_LEVEL)
            ++activeSamples;
        delayMicroseconds(300);
    }
    return activeSamples >= 3;
}
