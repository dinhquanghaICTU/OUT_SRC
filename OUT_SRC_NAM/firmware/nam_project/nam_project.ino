#include "inc/mortor/mortor.h"
#include <Arduino.h>
#include <ArduinoJson.h>

static unsigned long g_telemetry_interval = 100;
static unsigned long g_last_telemetry_time = 0;
static long g_last_pulses = 0;

static char g_rx_buffer[256];
static size_t g_rx_index = 0;

void send_ack(const char *cmd, const char *status, int pwm_val = 0) {
  JsonDocument ackDoc;
  ackDoc["type"] = "ack";
  ackDoc["cmd"] = cmd;
  ackDoc["status"] = status;
  ackDoc["pwm"] = pwm_val;
  serializeJson(ackDoc, Serial);
  Serial.println();
}

void process_command(const char *line) {

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, line);

  if (!err) {
    const char *cmd = doc["cmd"] | "";

    if (strcmp(cmd, "set_speed") == 0 || strcmp(cmd, "speed") == 0) {
      int pwm = 0;
      if (doc["pwm"].is<int>()) {
        pwm = doc["pwm"].as<int>();
      } else if (doc["speed"].is<int>()) {
        pwm = doc["speed"].as<int>();
      }
      mortor_set_speed(pwm);
      send_ack("set_speed", "ok", mortor_get_speed());
      return;
    } else if (strcmp(cmd, "stop") == 0) {
      mortor_set_speed(0);
      send_ack("stop", "ok", 0);
      return;
    } else if (strcmp(cmd, "reset_encoder") == 0) {
      mortor_reset_encoder();
      g_last_pulses = 0;
      send_ack("reset_encoder", "ok", mortor_get_speed());
      return;
    } else if (strcmp(cmd, "set_rate") == 0) {
      unsigned long rate = doc["interval"] | 100;
      if (rate >= 20 && rate <= 2000) {
        g_telemetry_interval = rate;
        send_ack("set_rate", "ok", (int)rate);
      } else {
        send_ack("set_rate", "invalid_range", (int)g_telemetry_interval);
      }
      return;
    } else if (strcmp(cmd, "ping") == 0) {
      send_ack("ping", "pong", mortor_get_speed());
      return;
    }
  }

  char cmd_str[32] = {0};
  int val = 0;
  if (sscanf(line, "SET %d", &val) == 1 || sscanf(line, "set %d", &val) == 1) {
    mortor_set_speed(val);
    send_ack("set_speed", "ok", mortor_get_speed());
  } else if (strcasecmp(line, "STOP") == 0) {
    mortor_set_speed(0);
    send_ack("stop", "ok", 0);
  } else if (strcasecmp(line, "RESET") == 0) {
    mortor_reset_encoder();
    g_last_pulses = 0;
    send_ack("reset_encoder", "ok", mortor_get_speed());
  } else if (strcasecmp(line, "PING") == 0) {
    send_ack("ping", "pong", mortor_get_speed());
  } else {
    send_ack("unknown", "error", 0);
  }
}

void check_serial_input() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (g_rx_index > 0) {
        g_rx_buffer[g_rx_index] = '\0';
        process_command(g_rx_buffer);
        g_rx_index = 0;
      }
    } else if (g_rx_index < sizeof(g_rx_buffer) - 1) {
      g_rx_buffer[g_rx_index++] = c;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  mortor_init();
  mortor_set_speed(0);

  JsonDocument bootDoc;
  bootDoc["type"] = "system";
  bootDoc["status"] = "ready";
  bootDoc["device"] = "ESP32_MOTOR_CONTROLLER";
  bootDoc["in1"] = MOTOR_PIN_IN1;
  bootDoc["in2"] = MOTOR_PIN_IN2;
  bootDoc["ena"] = MOTOR_PIN_ENA;
  bootDoc["enc_a"] = ENCODER_PIN_A;
  bootDoc["enc_b"] = ENCODER_PIN_B;
  bootDoc["ppr"] = ENCODER_PPR;
  bootDoc["gear_ratio"] = GEAR_RATIO;
  bootDoc["wheel_dia_m"] = WHEEL_DIAMETER_M;
  serializeJson(bootDoc, Serial);
  Serial.println();

  g_last_telemetry_time = millis();
  g_last_pulses = mortor_get_encoder();
}

void loop() {

  check_serial_input();

  unsigned long now = millis();
  if (now - g_last_telemetry_time >= g_telemetry_interval) {
    float dt = (now - g_last_telemetry_time) / 1000.0f;
    unsigned long dt_ms = now - g_last_telemetry_time;
    g_last_telemetry_time = now;

    long current_pulses = mortor_get_encoder();
    long delta_pulses = current_pulses - g_last_pulses;
    g_last_pulses = current_pulses;

    float rpm = mortor_calculate_rpm(delta_pulses, dt);
    float speed_mps = mortor_calculate_speed_mps(delta_pulses, dt);
    int current_pwm = mortor_get_speed();
    int dir = (current_pwm > 0) ? 1 : ((current_pwm < 0) ? -1 : 0);

    JsonDocument telemDoc;
    telemDoc["type"] = "telemetry";
    telemDoc["rpm"] = round(rpm * 10.0f) / 10.0f;
    telemDoc["speed_mps"] = round(speed_mps * 1000.0f) / 1000.0f;
    telemDoc["speed_cms"] = round(speed_mps * 1000.0f) / 10.0f;
    telemDoc["pulses"] = current_pulses;
    telemDoc["delta"] = delta_pulses;
    telemDoc["pwm"] = current_pwm;
    telemDoc["dir"] = dir;
    telemDoc["dt_ms"] = dt_ms;

    serializeJson(telemDoc, Serial);
    Serial.println();
  }
}
