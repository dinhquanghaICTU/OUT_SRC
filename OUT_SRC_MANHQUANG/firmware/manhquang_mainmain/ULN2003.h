#ifndef __ULN2003_H__
#define __ULN2003_H__

#include <stdbool.h>

#define IN1 18
#define IN2 19
#define IN3 25
#define IN4 26

#define OPEN_STEPS 1024
#define STEPS_PER_REV 2048

#ifdef __cplusplus
extern "C"
{
#endif

    void ULN2003_init(void);
    void openDoor(void);
    void closeDoor(void);
    void stopDoor(void);
    void holdOpenDoor(void);
    void waitMotionEnd(void);
    void wait3Seconds(void);
    void checkIR(void);
    void showStatus(void);
    int  getDoorPosition(void);
    float getDoorPositionPct(void);
    int  getDoorDirection(void);
    const char* getDoorStateStr(void);
    const char* getMotorDirectionStr(void);
    int  getPassageCount(void);
    float getMotorSpeedRpm(void);

#ifdef __cplusplus
}
#endif

#endif //__ULN2003_H__
