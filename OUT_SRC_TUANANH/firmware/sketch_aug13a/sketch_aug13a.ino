
#include "wifiAP.h"
#include "mqtt_manager.h"
#include "led.h"
#include "product_ID.h"
#include "config.h"
#include <driver/gpio.h>
#include "relay.h"
#include "PIR.h"
#include <Wire.h>
#include "BH1750_white_parper.h"


// BH1750 lightMeter; BH1750.h

long lastSendMs = 0;

float luxx = 0.0;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    delay(100);
    init_led();
    init_relay();
    pir_init();
    BH1750_white_parper_init();
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
    // relay_on();
    // delay(100);
    // relay_off();
    
    pir_update();

    if (millis() - lastSendMs >= SAMPLE_INTERVAL_MS)
    {
        lastSendMs = millis();
        const bool detech = pir_is_detected();
        Serial.print("DETECH");
        Serial.println(pir_is_detected());

        luxx = BH1750_white_parper_getdata();

        Serial.print("luxxx ");
        Serial.print(luxx);



        if (wifi_manager_get_state() == WIFI_MANAGER_CONNECTED && mqtt_manager_is_connected())
        {
            mqtt_manager_publish_sensor(detech, luxx, relay_get_state());
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
    
        }
        else
        {
            led_set_mode(LED_MODE_ON);
        }
        break;

    default:
        led_set_mode(LED_MODE_OFF);
        break;
    }

    led_update();
    delay(5);
}


