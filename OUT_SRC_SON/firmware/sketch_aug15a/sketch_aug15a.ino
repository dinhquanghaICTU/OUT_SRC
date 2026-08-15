#include "wifiAP.h"
#include "mqtt_manager.h"
#include "led.h"
#include "product_ID.h"
#include "config.h"
#include <driver/gpio.h>
#include "pump_peripheral.h"
#include "relay.h"

long lastSendMs = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pump_peripheral_init();

    init_led();

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
    // delay(300);
    // relay_off();
    pump_peripheral_update();

    if (millis() - lastSendMs >= SAMPLE_INTERVAL_MS)
    {
        lastSendMs = millis();

        float flowLMin = pump_get_flow_l_min();
        float totalLiters = pump_get_total_liters();
        bool pumpOn = pump_get_relay_state();

        Serial.print("Luu luong nuoc: ");
        Serial.print(flowLMin);
        Serial.println(" L/min");

        Serial.print("Tong nuoc: ");
        Serial.print(totalLiters);
        Serial.println(" L");

        Serial.print("Trang thai bom: ");
        Serial.println(pumpOn ? "ON" : "OFF");

        if (wifi_manager_get_state() == WIFI_MANAGER_CONNECTED && mqtt_manager_is_connected())
        {
            mqtt_manager_publish_sensor(flowLMin, totalLiters, pumpOn);
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