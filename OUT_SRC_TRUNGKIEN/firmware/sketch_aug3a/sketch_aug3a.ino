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
static bool s_is_alert = false;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    init_ring();
    turn_off_ring();
    delay(50);

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

    wifi_manager_begin();
}

void loop()
{
    uint32_t currentSampleInterval = mqtt_manager_get_sampling_interval_ms();
    if (millis() - lastSendMs >= currentSampleInterval)
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

        float effectiveUvVoltage = uvVoltage;
        float effectiveUvIndex = uvIndex;

        if ((uvVoltage == 0) && (uvIndex == 0))
        {
            voltageUV = generateSolarUvVoltage();
            voltageUV_toindex = generateSolarUvIndex();
            effectiveUvVoltage = voltageUV;
            effectiveUvIndex = voltageUV_toindex;
            Serial.printf("[UV]  voltage=%.3fV -> UV Index=%.2f\n", voltageUV, voltageUV_toindex);
        }
        else
        {
            Serial.printf("[UV]  voltage=%.3fV -> UV Index=%.2f\n", uvVoltage, uvIndex);
        }

        if (wifi_manager_get_state() == WIFI_MANAGER_CONNECTED && mqtt_manager_is_connected())
        {
            mqtt_manager_publish_sensor(effectiveUvVoltage, effectiveUvIndex, pressureHpa);
        }

        // --- Kiem tra nguong canh bao tu MQTT Config ---
        float uvWarn = mqtt_manager_get_uv_warning();
        float uvCrit = mqtt_manager_get_uv_critical();
        float pMin = mqtt_manager_get_pressure_min();
        float pMax = mqtt_manager_get_pressure_max();

        s_is_alert = false;
        if (millis() > 3000)
        {
            if (effectiveUvIndex >= uvWarn || effectiveUvIndex >= uvCrit)
            {
                s_is_alert = true;
                Serial.printf("[ALERT] UV vuot nguong: %.2f (Warn=%.2f, Crit=%.2f)\n", effectiveUvIndex, uvWarn, uvCrit);
            }
            if (pressureHpa < pMin || pressureHpa > pMax)
            {
                s_is_alert = true;
                Serial.printf("[ALERT] Ap suat vuot nguong: %.2f (Min=%.2f, Max=%.2f)\n", pressureHpa, pMin, pMax);
            }
        }

        if (s_is_alert)
        {
            turn_on_ring(); // Coi keu canh bao
            turn_on_led();  // Den sang dung yen (khong can nhap nhay)
        }
        else
        {
            turn_off_ring(); // Binh thuong tat coi
        }
    }

    if (button_is_pressed())
    {
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

    if (s_is_alert)
    {
        // Khi vuot nguong: Den sang dung yen (khong can nhap nhay)
        turn_on_led();
    }
    else
    {
        switch (wifi_manager_get_state())
        {
        case WIFI_MANAGER_AP_CONFIG:
            led_set_mode(LED_MODE_BLINK_FAST);
            break;

        case WIFI_MANAGER_CONNECTING:
            led_set_mode(LED_MODE_BLINK_SLOW);
            break;

        case WIFI_MANAGER_CONNECTED:
            led_set_mode(LED_MODE_ON);
            break;

        default:
            led_set_mode(LED_MODE_OFF);
            break;
        }

        led_update();
    }

    delay(5);
}
