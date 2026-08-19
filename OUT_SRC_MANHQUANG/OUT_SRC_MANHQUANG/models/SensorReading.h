#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QString>

struct SensorReading
{
    // Cảm biến chuyển động SR602 (PIR)
    bool motionDetected = false;
    int pirTriggerCount = 0;

    // Cảm biến vật cản hồng ngoại IR (Anti-pinch / Safety beam)
    bool irBlocked = false;
    bool antiPinchActive = false;

    // Động cơ bước (Stepper Motor) & Trạng thái cửa
    double doorPositionPct = 0.0;     // 0.0 (Closed) -> 100.0 (Open)
    int currentStep = 0;              // Số bước hiện tại
    int targetStep = 0;               // Số bước mục tiêu
    double motorSpeedRpm = 0.0;       // Tốc độ động cơ bước RPM
    QString motorDirection = QStringLiteral("STOP"); // CW (Open), CCW (Close), STOP
    QString doorState = QStringLiteral("CLOSED");    // CLOSED, OPENING, OPEN, CLOSING, OBSTACLE_STOP, LOCKED

    // Thống kê & Môi trường
    int passageCount = 0;             // Tổng lượt người qua cửa
    double temperatureC = 28.5;       // Nhiệt độ mạch / động cơ
    double motorCurrentMa = 0.0;      // Dòng điện động cơ (mA)
    QDateTime measuredAt;
};

Q_DECLARE_METATYPE(SensorReading)
