#pragma once

#include <QDateTime>
#include <QString>

struct Alert
{
    int id = 0;
    QString severity = QStringLiteral("warning"); // info, warning, danger
    QString title;
    QString message;
    QString source; // IR_SENSOR, SR602_PIR, STEPPER_MOTOR, ACCESS_CONTROL
    QDateTime timestamp;
    bool resolved = false;
};
