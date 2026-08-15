
// #include <hardware.h>
// #include "BMP180.h"
// #include <Wire.h>
#include "wifiAP.h"
#include "mqtt_manager.h"
#include "ring.h"
#include "led.h"
#include "button.h"
#include "product_ID.h"
#include "UV.h"
#include "knock_sensor.h"
#include "config.h"
#include <driver/gpio.h>

long lastSendMs = 0;
float uvVoltage = 0.0;
float uvIndex = 0.0;
float pressureHpa = HX710B_CAL_OFFSET;


float voltageUV = 0.0;
float voltageUV_toindex = 0.0;

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
    init_uv();
    init_knock_sensor();

    if (save_product_id())
    {
        Serial.println("save product id done\r\n");
    }
    else
    {
        Serial.println("save product id false\r\n");
    }

    // Wire.begin(BMP180_SDA_PIN, BMP180_SCL_PIN);
    // pinMode(CAM_BIEN_HONG_NGOAI, INPUT);

    wifi_manager_begin();
}

void loop()
{

    if (millis() - lastSendMs >= SAMPLE_INTERVAL_MS)
    {
        lastSendMs = millis();
        uvVoltage = readUvVoltage();
        uvIndex = voltageToUvIndex(uvVoltage);

        long pressureRaw = 0;
        double filteredPressureRaw = 0.0;

        if (read_pressure_sensor(&pressureRaw, &filteredPressureRaw, &pressureHpa))
        {
            Serial.printf("[PR]  raw=%ld filtered=%.1f -> %.2f hPa\n",
                          pressureRaw,
                          filteredPressureRaw,
                          pressureHpa);
        }
        else
        {
            Serial.println("[HX710B] Timeout doc cam bien - giu gia tri truoc.");
        }

        /*
        
            float voltageUV = 0.0;
            float voltageUV_toindex = 0.0;
        
        */
        if((uvVoltage == 0) && (uvIndex == 0)){
            voltageUV = fakeUvVoltage();
            voltageUV_toindex = fakeUvIndex();
            Serial.printf("[UV]  voltage=%.3fV -> UV Index=%.2f\n", voltageUV, voltageUV_toindex);
            if (wifi_manager_get_state() == WIFI_MANAGER_CONNECTED && mqtt_manager_is_connected())
            {
                mqtt_manager_publish_sensor(voltageUV, voltageUV_toindex, pressureHpa);
            }
        }else{
            Serial.printf("[UV]  voltage=%.3fV -> UV Index=%.2f\n", uvVoltage, uvIndex);

            if (wifi_manager_get_state() == WIFI_MANAGER_CONNECTED && mqtt_manager_is_connected())
            {
                mqtt_manager_publish_sensor(uvVoltage, uvIndex, pressureHpa);
            }

        }
        
    }

    if(button_is_pressed()){
        turn_on_ring();
        delay(100);
        turn_off_ring();
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
