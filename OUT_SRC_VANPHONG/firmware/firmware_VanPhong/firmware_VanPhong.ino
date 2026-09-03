#include "DHT11.h"
#include "LM393.h"
#include "config.h"
#include "led.h"
#include "mqtt_manager.h"
#include "product_ID.h"
#include "relay.h"
#include "wifiAP.h"
#include <driver/gpio.h>

long lastSendMs = 0;

float temperature = 0.0;
float air_humidity = 0.0;
float soil_moisture = 0.0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  init_relay();
  init_led();
  LM393_init();
  DHT11_init();

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

    temperature = DHT11_get_temperature();
    air_humidity = DHT11_get_readHumidity();
    soil_moisture = get_soil_moisture_percent();

    Serial.print("Nhiet do: ");
    Serial.println(temperature);
    Serial.print("Do am KK: ");
    Serial.println(air_humidity);
    Serial.print("Do am Dat: ");
    Serial.println(soil_moisture);

    if (wifi_manager_get_state() == WIFI_MANAGER_CONNECTED &&
        mqtt_manager_is_connected()) {
      mqtt_manager_publish_sensor(temperature, air_humidity, soil_moisture);
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