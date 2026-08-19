#pragma once

#include <QString>

class HotspotManager final
{
public:
    struct Config
    {
        QString interfaceName = QStringLiteral("wlan0");
        QString connectionName = QStringLiteral("MANHQUANG-AP");
        QString ssid = QStringLiteral("MANHQUANG-AP");
        QString password = QStringLiteral("12345678");
        QString addressCidr = QStringLiteral("192.168.4.1/24");
    };

    static bool ensureStarted(const Config &config, QString *error);
    static bool stop(const QString &connectionName, QString *error);
};
