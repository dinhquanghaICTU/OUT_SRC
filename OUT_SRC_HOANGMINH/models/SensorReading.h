#pragma once

#include <QDateTime>
#include <QMetaType>

struct SensorReading
{
    double temperatureC = 0.0;
    double soundVpp = 0.0;
    bool fanOn = false;
    double pressureHpa = 0.0;
    double distanceCm = 0.0;
    QDateTime measuredAt;
};

Q_DECLARE_METATYPE(SensorReading)
