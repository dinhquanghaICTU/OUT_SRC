#pragma once

#include <QDateTime>
#include <QMetaType>

struct SensorReading
{
    double pressureHpa = 0.0;
    double distanceCm = 0.0;
    double temperatureC = 0.0;
    QDateTime measuredAt;
};

Q_DECLARE_METATYPE(SensorReading)
