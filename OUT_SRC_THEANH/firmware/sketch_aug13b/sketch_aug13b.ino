#include "ACS712.h"
#include "ZMPT101B.h"
#include "config.h"
#include "led.h"
#include "mqtt_manager.h"
#include "product_ID.h"
#include "relay.h"
#include "wifiAP.h"
#include <driver/gpio.h>

long lastSendMs = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  init_relay();
  acs712_init();
  zmpt101b_init();
  init_led();

  if (save_product_id()) {
    Serial.println("save product id done\r\n");
  } else {
    Serial.println("save product id false\r\n");
  }

  wifi_manager_begin();
}

void loop() {
  if (millis() - lastSendMs >= SAMPLE_INTERVAL_MS) {
    lastSendMs = millis();

    acs712_update();
    zmpt101b_update();

    float currentA = acs712_get_current_a();
    float voltageV = zmpt101b_get_voltage_v();

    Serial.printf("[SENSOR] Voltage: %.2f V | Current: %.3f A\r\n", voltageV, currentA);

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

  switch (wifi_manager_get_state()) {
  case WIFI_MANAGER_AP_CONFIG:
    led_set_mode(LED_MODE_BLINK_FAST);
    break;

  case WIFI_MANAGER_CONNECTING:
    led_set_mode(LED_MODE_BLINK_SLOW);
    break;

  case WIFI_MANAGER_CONNECTED:
    if (mqtt_manager_is_connected()) {
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