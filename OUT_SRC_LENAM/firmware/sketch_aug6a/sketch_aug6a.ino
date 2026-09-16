#include "wifiAP.h"
#include "mqtt_manager.h"
#include "led.h"
#include "product_ID.h"
#include "config.h"
#include "BMP180.h"
#include "ir_sensor.h"
#include "ring.h"
#include <driver/gpio.h>

static unsigned long lastSendMs = 0;
static bool currentIrDetected = false;
static bool previousIrDetected = false;
static bool irAlarmActive = false;
static unsigned long irAlarmStopAtMs = 0;
static bool thresholdAlertActive = false;
static float s_lastTempC = 25.0f;
static float s_lastPressHpa = 1013.25f;

static void send_telemetry(bool irDetected)
{
    if (!wifi_manager_is_connected() || !mqtt_manager_is_connected())
        return;

    float temperatureC = s_lastTempC;
    float pressureHpa = s_lastPressHpa;

    if (!bmp180_read(&temperatureC, &pressureHpa))
    {
#if BMP180_SIMULATE_ON_FAILURE
        bmp180_simulate(&temperatureC, &pressureHpa);
#endif
    }
    s_lastTempC = temperatureC;
    s_lastPressHpa = pressureHpa;

    alert_status_t alert = mqtt_manager_check_thresholds(temperatureC, pressureHpa, irDetected);
    thresholdAlertActive = alert.active;
    led_set_mode(LED_MODE_ON);

    mqtt_manager_publish_sensor(temperatureC, pressureHpa, irDetected, &alert);
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    init_led();
    bmp180_begin();
    ir_sensor_begin();
    init_ring();
    mqtt_manager_init();

    if (save_product_id())
        Serial.println("[SYSTEM] Save product ID OK");
    else
        Serial.println("[SYSTEM] Save product ID Failed");

    wifi_manager_begin();
}

void loop()
{
    currentIrDetected = ir_sensor_detected();
    device_thresholds_t thresholds = mqtt_manager_get_thresholds();
    bool irEventTriggered = false;

    // 1. Trigger pulse alarm on newly detected obstacle
    if (currentIrDetected && !previousIrDetected)
    {
        irAlarmActive = true;
        unsigned long durationMs = (unsigned long)(thresholds.ir_alarm_seconds * 1000.0f);
        if (durationMs < 200) durationMs = 200;
        irAlarmStopAtMs = millis() + durationMs;
        Serial.printf("[IR] Phat hien vat can - Bat coi trong %u ms\n", (unsigned)durationMs);
        irEventTriggered = true;
    }
    previousIrDetected = currentIrDetected;

    // Check IR alarm expiration
    if (irAlarmActive && (long)(millis() - irAlarmStopAtMs) >= 0)
    {
        irAlarmActive = false;
        Serial.println("[IR] Het thoi gian coi IR");
        irEventTriggered = true;
    }

    // 2. WiFi & MQTT background management
    wifi_manager_update();

    if (wifi_manager_is_connected())
        mqtt_manager_start();
    else
        mqtt_manager_stop();

    // 3. Sensor sampling & Threshold evaluation
    const bool effectiveIr = currentIrDetected || irAlarmActive;
    const uint32_t intervalMs = mqtt_manager_get_sample_interval();
    const bool intervalExpired = (millis() - lastSendMs >= intervalMs);

    if (wifi_manager_is_connected() && mqtt_manager_is_connected())
    {
        if (irEventTriggered || intervalExpired)
        {
            lastSendMs = millis();
            send_telemetry(effectiveIr);
        }
    }
    else
    {
        if (wifi_manager_is_connected())
            led_set_mode(LED_MODE_BLINK_SLOW);
        else
            led_set_mode(LED_MODE_BLINK_FAST);
    }

    // 4. Combined Buzzer Ring Control: Manual ON || IR Alarm || Threshold Alert
    bool shouldRing = mqtt_manager_get_ring_state() || irAlarmActive || thresholdAlertActive;
    if (shouldRing)
    {
        turn_on_ring();
    }
    else
    {
        turn_off_ring();
    }

    led_update();
    delay(5);
}
