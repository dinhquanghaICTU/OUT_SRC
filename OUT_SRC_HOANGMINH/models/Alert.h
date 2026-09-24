#pragma once

#include <QDateTime>
#include <QString>

struct Alert
{
    int id = 0;
    QString type;
    double value = 0.0;
    QDateTime createdAt;
    bool acknowledged = false;
};
