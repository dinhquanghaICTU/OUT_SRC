#pragma once

#include <QDateTime>
#include <QString>

struct Alert
{
    int id = 0;
    QString severity = QStringLiteral("warning"); // info, warning, danger
    QString title;
    QString message;
    QString source; // SOIL_SENSOR, DHT11_TEMP, WATER_TANK, PUMP_PROTECT
    QDateTime timestamp;
    bool resolved = false;
};
