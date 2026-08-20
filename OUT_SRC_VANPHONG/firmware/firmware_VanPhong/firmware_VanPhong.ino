#include "wifiAP.h"
#include "mqtt_manager.h"
#include "led.h"
#include "product_ID.h"
#include "config.h"
#include <driver/gpio.h>
#include "relay.h"
#include "LM393.h"
#include "DHT11.h"

long lastSendMs = 0;

float temperature = 0.0;
float humidity = 0.0;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    init_relay();
    init_led();
    LM393_init();
    DHT11_init();

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

    if (millis() - lastSendMs >= SAMPLE_INTERVAL_MS)
    {
        lastSendMs = millis();

        Serial.print("nhiet do: ");
        temperature = DHT11_get_temperature();
        Serial.println(temperature);

        Serial.print("do am: ");
        humidity = DHT11_get_readHumidity();
        Serial.println(humidity);

        if (wifi_manager_get_state() == WIFI_MANAGER_CONNECTED && mqtt_manager_is_connected())
        {
            mqtt_manager_publish_sensor(temperature,humidity);
            // Serial.println("check");
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