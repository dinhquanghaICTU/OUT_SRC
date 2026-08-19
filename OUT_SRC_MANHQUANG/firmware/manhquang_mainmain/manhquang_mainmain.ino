#include "wifiAP.h"
#include "mqtt_manager.h"
#include "led.h"
#include "product_ID.h"
#include "config.h"
#include <driver/gpio.h>
#include "lM393.h"
#include "SR602.h"
#include "ULN2003.h"

long lastSendMs = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    LM393_init();
    SR602_init();
    init_led();
    ULN2003_init();

    if (save_product_id())
    {
        Serial.println("save product id done\r\n");
    }
    else
    {
        Serial.println("save product id false\r\n");
    }

    wifi_manager_begin();
}

void loop()
{
    
    if (getDoorPosition() == 0 && getDoorDirection() == 0)
    {
        
        if (digitalRead(SR602_PIN) == HIGH)
        {
            Serial.println(">>> [AUTO] SR602 PHAT HIEN NGUOI -> TU DONG MO CUA");
            openDoor();
            waitMotionEnd();
            wait3Seconds();
            closeDoor();
        }
    }

    
    if (millis() - lastSendMs >= SAMPLE_INTERVAL_MS)
    {
        lastSendMs = millis();
        bool motion = (digitalRead(SR602_PIN) == HIGH);
        bool ir = (digitalRead(LM393_PIN) == LOW);
        float pos_pct = getDoorPositionPct();
        float speed = getMotorSpeedRpm();
        int passages = getPassageCount();
        const char *door_state = getDoorStateStr();
        const char *motor_dir = getMotorDirectionStr();
        float temp_c = 28.5f;

        if (wifi_manager_get_state() == WIFI_MANAGER_CONNECTED && mqtt_manager_is_connected())
        {
            mqtt_manager_publish_door(motion, ir, pos_pct, speed, passages, door_state, motor_dir, temp_c);
        }
    }

    wifi_manager_update();

    if (wifi_manager_is_connected())
        mqtt_manager_start();
    else
        mqtt_manager_stop();

    switch (wifi_manager_get_state())
    {
    case WIFI_MANAGER_AP_CONFIG:
        led_set_mode(LED_MODE_BLINK_FAST);
        break;

    case WIFI_MANAGER_CONNECTING:
        led_set_mode(LED_MODE_BLINK_SLOW);
        break;

    case WIFI_MANAGER_CONNECTED:
        if (mqtt_manager_is_connected())
        {
            led_set_mode(LED_MODE_ON);
        }
        else
        {
            led_set_mode(LED_MODE_BLINK_SLOW);
        }
        break;

    default:
        led_set_mode(LED_MODE_OFF);
        break;
    }

    led_update();
    delay(5);
}
