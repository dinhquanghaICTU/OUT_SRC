#include "ULN2003.h"
#include "SR602.h"
#include "lM393.h"
#include "mqtt_manager.h"
#include <Stepper.h>
#include <Arduino.h>


static Stepper motor(STEPS_PER_REV, IN1, IN3, IN2, IN4);

static int position = 0;           
static int direction = 0;          
static int s_passage_count = 0;
static bool s_is_held_open = false;
static float s_motor_speed_rpm = 12.0f;

void ULN2003_init(void)
{
    motor.setSpeed(12);
    position = 0;
    direction = 0;
    s_passage_count = 0;
    s_is_held_open = false;
    s_motor_speed_rpm = 12.0f;
    Serial.println("[ULN2003] Khoi tao thanh cong (Speed=12 RPM)");
}

int getDoorPosition(void)
{
    return position;
}

float getDoorPositionPct(void)
{
    return ((float)position / (float)OPEN_STEPS) * 100.0f;
}

int getDoorDirection(void)
{
    return direction;
}

const char* getDoorStateStr(void)
{
    if (digitalRead(LM393_PIN) == LOW && direction == -1) return "OBSTACLE_STOP";
    if (position >= OPEN_STEPS) return s_is_held_open ? "HOLD_OPEN" : "OPEN";
    if (position <= 0) return "CLOSED";
    if (direction == 1) return "OPENING";
    if (direction == -1) return "CLOSING";
    return "STOP";
}

const char* getMotorDirectionStr(void)
{
    if (direction == 1) return "CW";
    if (direction == -1) return "CCW";
    return "STOP";
}

int getPassageCount(void)
{
    return s_passage_count;
}

float getMotorSpeedRpm(void)
{
    return (direction != 0) ? s_motor_speed_rpm : 0.0f;
}

void showStatus(void)
{
    Serial.print("[DOOR] Vi tri: ");
    Serial.print(getDoorStateStr());
    Serial.print(" (");
    Serial.print(position);
    Serial.print("/");
    Serial.print(OPEN_STEPS);
    Serial.print(")");

    Serial.print(" | SR602(PIR): ");
    Serial.print(digitalRead(SR602_PIN) ? "CO" : "KHONG");

    Serial.print(" | IR(LM393): ");
    Serial.println(digitalRead(LM393_PIN) == LOW ? "CO" : "KHONG");
}

void checkIR(void)
{
    if (digitalRead(LM393_PIN) == LOW)
    {
        int oldDirection = direction;
        direction = 0;
        Serial.println(">>> [IR] PHAT HIEN VAT CAN -> DUNG CUA (CHONG KET)");

        
        mqtt_manager_publish_door(digitalRead(SR602_PIN) == HIGH, true,
                                  getDoorPositionPct(), 0.0f, s_passage_count,
                                  "OBSTACLE_STOP", "STOP", 28.5f);

        while (digitalRead(LM393_PIN) == LOW)
        {
            showStatus();
            delay(100);
        }

        Serial.println(">>> [IR] HET VAT CAN -> TIEP TUC HANH TRINH");
        direction = oldDirection;
    }
}

void openDoor(void)
{
    s_is_held_open = false;
    direction = 1;
    Serial.println(">>> BAT DAU MO CUA...");

    
    mqtt_manager_publish_door(true, digitalRead(LM393_PIN) == LOW,
                              getDoorPositionPct(), 12.0f, s_passage_count,
                              "OPENING", "CW", 28.5f);

    while (position < OPEN_STEPS)
    {
        checkIR();
        motor.step(1);
        position++;

        if (position % 256 == 0)
        {
            showStatus();
            
            mqtt_manager_publish_door(true, digitalRead(LM393_PIN) == LOW,
                                      getDoorPositionPct(), 12.0f, s_passage_count,
                                      "OPENING", "CW", 28.5f);
        }
    }

    position = OPEN_STEPS;
    direction = 0;
    Serial.println(">>> CUA DA MO HOAN TOAN!");

    
    mqtt_manager_publish_door(true, digitalRead(LM393_PIN) == LOW,
                              100.0f, 0.0f, s_passage_count,
                              "OPEN", "STOP", 28.5f);
    mqtt_manager_publish_state("OPEN", 100.0f, "hardware");
}

void holdOpenDoor(void)
{
    openDoor();
    s_is_held_open = true;
    Serial.println(">>> CHE DO GIU CUA MO LIEN TUC!");
    mqtt_manager_publish_door(true, digitalRead(LM393_PIN) == LOW,
                              100.0f, 0.0f, s_passage_count,
                              "HOLD_OPEN", "STOP", 28.5f);
    mqtt_manager_publish_state("HOLD_OPEN", 100.0f, "hold_open");
}

void stopDoor(void)
{
    direction = 0;
    s_is_held_open = false;
    Serial.println(">>> DUNG KHAN CAP DONG CO!");
    mqtt_manager_publish_door(digitalRead(SR602_PIN) == HIGH, digitalRead(LM393_PIN) == LOW,
                              getDoorPositionPct(), 0.0f, s_passage_count,
                              "STOP", "STOP", 28.5f);
    mqtt_manager_publish_state("STOP", getDoorPositionPct(), "stop");
}

void waitMotionEnd(void)
{
    if (s_is_held_open) return;
    Serial.println(">>> CHO NGUOI DI QUA (HET CHUYEN DONG)...");

    while (digitalRead(SR602_PIN) == HIGH && !s_is_held_open)
    {
        showStatus();
        delay(100);
    }

    Serial.println(">>> SR602: KHONG CON CHUYEN DONG");
    
    mqtt_manager_publish_door(false, digitalRead(LM393_PIN) == LOW,
                              100.0f, 0.0f, s_passage_count,
                              "OPEN", "STOP", 28.5f);
}

void wait3Seconds(void)
{
    if (s_is_held_open) return;
    Serial.println(">>> CHO 3 GIAY TRUOC KHI DONG...");
    unsigned long startTime = millis();

    while (millis() - startTime < 3000 && !s_is_held_open)
    {
        showStatus();

        if (digitalRead(SR602_PIN) == HIGH)
        {
            Serial.println(">>> CO NGUOI DEN -> RESET TIMER 3 GIAY");
            startTime = millis();
        }

        delay(100);
    }
}

void closeDoor(void)
{
    if (s_is_held_open) return;
    direction = -1;
    Serial.println(">>> BAT DAU DONG CUA...");

    
    mqtt_manager_publish_door(false, digitalRead(LM393_PIN) == LOW,
                              getDoorPositionPct(), 12.0f, s_passage_count,
                              "CLOSING", "CCW", 28.5f);

    while (position > 0 && !s_is_held_open)
    {
        
        if (digitalRead(SR602_PIN) == HIGH)
        {
            Serial.println(">>> PHAT HIEN NGUOI KHI DANG DONG -> MO NGUOC LAI!");
            openDoor();
            waitMotionEnd();
            wait3Seconds();
            direction = -1;
            Serial.println(">>> TIEP TUC DONG CUA...");
            continue;
        }

        checkIR();
        direction = -1;

        motor.step(-1);
        position--;

        if (position % 256 == 0)
        {
            showStatus();
            
            mqtt_manager_publish_door(false, digitalRead(LM393_PIN) == LOW,
                                      getDoorPositionPct(), 12.0f, s_passage_count,
                                      "CLOSING", "CCW", 28.5f);
        }
    }

    if (!s_is_held_open)
    {
        position = 0;
        direction = 0;
        s_passage_count++;
        Serial.println(">>> CUA DA DONG HOAN TOAN!");

        
        mqtt_manager_publish_door(false, digitalRead(LM393_PIN) == LOW,
                                  0.0f, 0.0f, s_passage_count,
                                  "CLOSED", "STOP", 28.5f);
        mqtt_manager_publish_state("CLOSED", 0.0f, "hardware");
    }
}
