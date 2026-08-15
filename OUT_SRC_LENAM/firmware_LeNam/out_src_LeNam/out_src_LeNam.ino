
#include "wifiAP.h"
#include "mqtt_manager.h"
#include "ring.h"
#include "led.h"
#include "button.h"
#include "product_ID.h"
#include "config.h"
#include <driver/gpio.h>
#include "LM35.h"
#include "microphone.h"
#include "relay.h"

long lastSendMs = 0;

float raw_lm35 = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    init_ring();
    delay(100);
    // turn_on_ring();
    // delay(100);
    // turn_off_ring();

    init_led();
    init_button();
    init_relay();
    init_lm35();
    init_microphone();
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

        /*
            get data in LM35
        */
        const int raw = analogRead(LM35_PIN);
        const float voltage = readLm35Voltage();
        const float temperature = voltage * 100.0F;

        Serial.printf(
            "[LM35] raw=%d, voltage=%.3fV, temperature=%.2f C\n",
            raw,
            voltage,
            temperature
        );

        Serial.print("temperature raw:");
        Serial.println(raw_lm35);


        const float soundVpp = get_microphone();

        if (wifi_manager_get_state() == WIFI_MANAGER_CONNECTED && mqtt_manager_is_connected())
        {
            mqtt_manager_publish_sensor(temperature, soundVpp);
        }
    }

    if (button_was_held(BUTTON_CONFIG_HOLD_MS))
    {

        Serial.printf("[BUTTON] GPIO%d=%d pressed=%d\n",
                      BUTTON_PIN,
                      gpio_get_level((gpio_num_t)BUTTON_PIN),
                      button_is_pressed());
        clearWiFiCredentials();
        mqtt_manager_stop();
        wifi_manager_enter_config();
    }

    /*
        handle relay
    
    */
    if (button_was_pressed())
    {
        const bool relay_enabled = !relay_get_state();
        relay_set(relay_enabled);

        if (relay_enabled)
        {
            Serial.println("[RELAY] ON");
        }
        else
        {
            Serial.println("[RELAY] OFF");
        }

        if (mqtt_manager_is_connected())
            mqtt_manager_publish_relay(relay_enabled, "button");
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
        // led_set_mode(mqtt_manager_is_connected()? LED_MODE_ON);
        if (mqtt_manager_is_connected())
        {
            // led_set_mode(LED_MODE_ON);
            // Serial.println("check/r/n");
            // for (int i; i <= 10; i++)
            // {
            //     delay(1000);
            //     test_mqtt();
            // }
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

// E (30210) mqtt_client: Client was not initialized
