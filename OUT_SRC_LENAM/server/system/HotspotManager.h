#ifndef HOTSPOTMANAGER_H
#define HOTSPOTMANAGER_H

#include <QString>

class HotspotManager
{
public:
    struct Config
    {
        QString connectionName = QStringLiteral("ICTU_IOT_AP");
        QString interfaceName = QStringLiteral("wlan1");
        QString parentInterfaceName = QStringLiteral("wlan0");
        QString ssid = QStringLiteral("ICTU_IOT_AP");
        QString password = QStringLiteral("12345678");
        QString addressCidr = QStringLiteral("192.168.4.1/24");
    };

    static bool ensureStarted(const Config &config, QString *error = nullptr);
};

#endif // HOTSPOTMANAGER_H
