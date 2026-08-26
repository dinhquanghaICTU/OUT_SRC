#include "ACS712.h"
#include "ZMPT101B.h"
#include "config.h"
#include "led.h"
#include "mqtt_manager.h"
#include "product_ID.h"
#include "wifiAP.h"
#include <driver/gpio.h>

long lastSendMs = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  acs712_init();
  zmpt101b_init();
  init_led();
  mqtt_manager_init();

  if (save_product_id()) {
    Serial.println("save product id done\r\n");
  } else {
    Serial.println("save product id false\r\n");
  }

  wifi_manager_begin();
}

void loop() {
  const uint32_t interval = mqtt_manager_get_sample_interval();
  if (millis() - lastSendMs >= interval) {
    lastSendMs = millis();

    acs712_update();
    zmpt101b_update();

    float currentA = acs712_get_current_a();
    float voltageV = zmpt101b_get_voltage_v();
    float powerW = voltageV * currentA;

    alert_status_t alert = mqtt_manager_check_thresholds(currentA, voltageV, powerW);

    if (alert.is_alert) {
      Serial.printf("[WARNING] %s | Voltage: %.2f V | Current: %.3f A | Power: %.2f W\r\n",
                    alert.alert_msg, voltageV, currentA, powerW);
    } else {
      Serial.printf("[SENSOR] Voltage: %.2f V | Current: %.3f A | Power: %.2f W\r\n",
                    voltageV, currentA, powerW);
    }

    if (wifi_manager_get_state() == WIFI_MANAGER_CONNECTED &&
        mqtt_manager_is_connected()) {
      mqtt_manager_publish_sensor(currentA, voltageV);
    }
  }

  wifi_manager_update();

  if (wifi_manager_is_connected())
    mqtt_manager_start();
  else
    mqtt_manager_stop();

  float currentA = acs712_get_current_a();
  float voltageV = zmpt101b_get_voltage_v();
  float powerW = voltageV * currentA;
  alert_status_t alert = mqtt_manager_check_thresholds(currentA, voltageV, powerW);

  switch (wifi_manager_get_state()) {
  case WIFI_MANAGER_AP_CONFIG:
    led_set_mode(LED_MODE_BLINK_FAST);
    break;

  case WIFI_MANAGER_CONNECTING:
    led_set_mode(LED_MODE_BLINK_SLOW);
    break;

  case WIFI_MANAGER_CONNECTED:
    if (alert.is_alert) {
      led_set_mode(LED_MODE_BLINK_FAST); // Warning blink when threshold exceeded
    } else if (mqtt_manager_is_connected()) {
      led_set_mode(LED_MODE_ON);
    } else {
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