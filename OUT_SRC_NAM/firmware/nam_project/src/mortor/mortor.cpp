#include "inc/mortor/mortor.h"

static volatile long s_encoder_count = 0;
static int s_current_speed = 0;
static portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;

static void IRAM_ATTR encoder_isr(void) {
  int b = digitalRead(ENCODER_PIN_B);
  if (b == HIGH) {
    s_encoder_count++;
  } else {
    s_encoder_count--;
  }
}

void mortor_init(void) {
  pinMode(MOTOR_PIN_IN1, OUTPUT);
  pinMode(MOTOR_PIN_IN2, OUTPUT);
  pinMode(MOTOR_PIN_ENA, OUTPUT);

  digitalWrite(MOTOR_PIN_IN1, LOW);
  digitalWrite(MOTOR_PIN_IN2, LOW);
  analogWrite(MOTOR_PIN_ENA, 0);

  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), encoder_isr, RISING);
}

void mortor_set_speed(int speed) {
  if (speed > 255)
    speed = 255;
  if (speed < -255)
    speed = -255;

  s_current_speed = speed;

  if (speed > 0) {
    digitalWrite(MOTOR_PIN_IN1, HIGH);
    digitalWrite(MOTOR_PIN_IN2, LOW);
    analogWrite(MOTOR_PIN_ENA, speed);
  } else if (speed < 0) {
    digitalWrite(MOTOR_PIN_IN1, LOW);
    digitalWrite(MOTOR_PIN_IN2, HIGH);
    analogWrite(MOTOR_PIN_ENA, -speed);
  } else {
    digitalWrite(MOTOR_PIN_IN1, LOW);
    digitalWrite(MOTOR_PIN_IN2, LOW);
    analogWrite(MOTOR_PIN_ENA, 0);
  }
}

int mortor_get_speed(void) {
  return s_current_speed;
}

long mortor_get_encoder(void) {
  portENTER_CRITICAL(&s_mux);
  long count = s_encoder_count;
  portEXIT_CRITICAL(&s_mux);
  return count;
}

void mortor_reset_encoder(void) {
  portENTER_CRITICAL(&s_mux);
  s_encoder_count = 0;
  portEXIT_CRITICAL(&s_mux);
}

float mortor_calculate_rpm(long delta_pulses, float dt_seconds) {
  if (dt_seconds <= 0.0f) return 0.0f;
  float total_ppr = ENCODER_PPR * GEAR_RATIO;
  float rps = ((float)delta_pulses / total_ppr) / dt_seconds;
  return rps * 60.0f;
}

float mortor_calculate_speed_mps(long delta_pulses, float dt_seconds) {
  if (dt_seconds <= 0.0f) return 0.0f;
  float total_ppr = ENCODER_PPR * GEAR_RATIO;
  float circumference = 3.14159265f * WHEEL_DIAMETER_M;
  float rps = ((float)delta_pulses / total_ppr) / dt_seconds;
  return rps * circumference;
}
