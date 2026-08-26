
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

long lastSendMs = 0;
float luxx = 0.0;
static bool previousDetech = false;
static unsigned long motionHoldUntilMs = 0;
static bool autoRelayTriggered = false;

void setup()
{
    Serial.begin(115200);
    delay(500);
    init_led();
    init_relay();
    pir_init();
    BH1750_white_parper_init();
    if (save_product_id())
    {
        Serial.println("[SYSTEM] Save product ID OK");
    }
    else
    {
        Serial.println("[SYSTEM] Save product ID Failed");
    }

    wifi_manager_begin();
}

void loop()
{
    pir_update();
    const bool detech = pir_is_detected();
    bool stateChanged = (detech != previousDetech);
    previousDetech = detech;

    const device_config_thresholds_t cfg = mqtt_manager_get_thresholds();
    const uint32_t intervalMs = cfg.sample_interval_ms > 0 ? cfg.sample_interval_ms : SAMPLE_INTERVAL_MS;

    luxx = BH1750_white_parper_getdata();

    // Auto Trigger logic on hardware based on configured thresholds
    static bool prevRelayState = false;
    if (mqtt_manager_get_auto_mode())
    {
        const bool isDark = (luxx > 0.0f && luxx <= cfg.lux_min);
        const bool isTooBright = (luxx >= cfg.lux_max && cfg.lux_max > 0.0f);
        const bool shouldBeOn = detech || isDark;

        if (shouldBeOn)
        {
            if (!relay_get_state())
            {
                relay_set(true);
                Serial.printf("[AUTO] BAT Relay (Lux=%.2f <= Min=%.1f, Motion=%d)\n",
                              luxx, cfg.lux_min, detech ? 1 : 0);
                if (wifi_manager_is_connected() && mqtt_manager_is_connected())
                {
                    mqtt_manager_publish_relay(true, isDark ? "auto_lux" : "auto_pir");
                }
            }
        }
        else if (isTooBright && relay_get_state())
        {
            relay_set(false);
            Serial.printf("[AUTO] Troi sang -> TAT Relay (Lux=%.2f >= Max=%.1f)\n",
                          luxx, cfg.lux_max);
            if (wifi_manager_is_connected() && mqtt_manager_is_connected())
            {
                mqtt_manager_publish_relay(false, "auto_lux_bright");
            }
        }
    }

    bool relayChanged = (relay_get_state() != prevRelayState);
    prevRelayState = relay_get_state();

    // Send sensor telemetry periodically OR immediately upon motion/relay state change
    if ((millis() - lastSendMs >= intervalMs) || stateChanged || relayChanged)
    {
        lastSendMs = millis();

        Serial.printf("[SENSOR] Motion=%d, Lux=%.2f, Relay=%d, Auto=%d\n",
                      detech ? 1 : 0, luxx, relay_get_state() ? 1 : 0, mqtt_manager_get_auto_mode() ? 1 : 0);

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
        led_set_mode(LED_MODE_ON);
        break;

    default:
        led_set_mode(LED_MODE_OFF);
        break;
    }

    led_update();
    delay(5);
}


