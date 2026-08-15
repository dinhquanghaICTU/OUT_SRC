#include "wifiAP.h"
#include "mqtt_manager.h"
#include "led.h"
#include "product_ID.h"
#include "config.h"
#include "BMP180.h"
#include "ir_sensor.h"
#include <driver/gpio.h>
#include "ring.h"

long lastSendMs = 0;
bool currentIrDetected = false;
bool previousIrDetected = false;
bool ringActive = false;
unsigned long ringStopAtMs = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    delay(100);

    init_led();
    bmp180_begin();
    ir_sensor_begin();
    init_ring();

    if (save_product_id())
        Serial.println("save product id done\r\n");
    else
        Serial.println("save product id false\r\n");

    wifi_manager_begin();
}

void loop()
{
    currentIrDetected = ir_sensor_detected();

    // Chỉ kích còi ở cạnh phát hiện mới; không block loop bằng delay().
    if (currentIrDetected && !previousIrDetected)
    {
        turn_on_ring();
        ringActive = true;
        ringStopAtMs = millis() + 1000UL;
        Serial.println("[IR] Phat hien vat - bat coi trong 1 giay");
    }
    previousIrDetected = currentIrDetected;

    if (ringActive && (long)(millis() - ringStopAtMs) >= 0)
    {
        turn_off_ring();
        ringActive = false;
        Serial.println("[IR] Tat coi");
    }

    wifi_manager_update();

    if (wifi_manager_is_connected())
        mqtt_manager_start();
    else
        mqtt_manager_stop();

    if (millis() - lastSendMs >= SAMPLE_INTERVAL_MS)
    {
        lastSendMs = millis();

        if (wifi_manager_is_connected() && mqtt_manager_is_connected())
        {
            float temperatureC = 0.0f;
            float pressureHpa = 0.0f;

            if (bmp180_read(&temperatureC, &pressureHpa))
            {
                Serial.printf("[BMP180] Nhiet do=%.2f C, Ap suat=%.2f hPa\n",
                              temperatureC, pressureHpa);
                Serial.printf("[IR] raw=%d, detected=%d\n",
                              ir_sensor_raw(), currentIrDetected ? 1 : 0);

                if (!mqtt_manager_publish_sensor(temperatureC, pressureHpa, currentIrDetected))
                    Serial.println("[BMP180] Gui MQTT that bai");
            }
            else
            {
                Serial.println("[BMP180] Doc cam bien that bai");
#if BMP180_SIMULATE_ON_FAILURE
                bmp180_simulate(&temperatureC, &pressureHpa);
                Serial.printf("[BMP180][MO PHONG] Nhiet do=%.2f C, Ap suat=%.2f hPa\n",
                              temperatureC, pressureHpa);
                if (!mqtt_manager_publish_sensor(temperatureC, pressureHpa, currentIrDetected))
                    Serial.println("[BMP180][MO PHONG] Gui MQTT that bai");
#endif
            }
        }
        else
        {
            Serial.println("[MQTT] Chua san sang: WiFi/MQTT chua ket noi");
        }
    }

    if (wifi_manager_is_connected())
        led_set_mode(mqtt_manager_is_connected() ? LED_MODE_ON : LED_MODE_BLINK_SLOW);
    else
        led_set_mode(LED_MODE_BLINK_FAST);

    led_update();
    delay(5);
}
