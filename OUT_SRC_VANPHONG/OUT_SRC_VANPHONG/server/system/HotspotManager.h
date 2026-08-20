#pragma once

#include <QString>

class HotspotManager final
{
public:
    struct Config
    {
        QString interfaceName = QStringLiteral("wlan1");
        QString connectionName = QStringLiteral("ICTU_VANPHONG_AP");
        QString ssid = QStringLiteral("ICTU_VANPHONG_AP");
        QString password = QStringLiteral("12345678");
        QString addressCidr = QStringLiteral("192.168.4.1/24");
    };

    static bool ensureStarted(const Config &config, QString *error);
    static bool stop(const QString &connectionName, QString *error);
};
