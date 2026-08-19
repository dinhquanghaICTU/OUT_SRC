#pragma once

#include <QString>

struct User
{
    int id = 0;
    QString username;
    QString role = QStringLiteral("user"); // admin, operator, user
    bool enabled = true;
    QString createdAt;
};
