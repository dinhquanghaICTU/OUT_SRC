#include "HotspotManager.h"

#include <QProcess>

bool HotspotManager::ensureStarted(const Config &config, QString *error)
{
    QProcess checkProcess;
    checkProcess.start(QStringLiteral("nmcli"), {QStringLiteral("-t"), QStringLiteral("-f"), QStringLiteral("NAME"), QStringLiteral("connection"), QStringLiteral("show")});
    checkProcess.waitForFinished(3000);

    const QString output = QString::fromUtf8(checkProcess.readAllStandardOutput());
    const bool exists = output.split('\n').contains(config.connectionName);

    if (!exists) {
        QProcess createProcess;
        QStringList args = {
            QStringLiteral("connection"), QStringLiteral("add"),
            QStringLiteral("type"), QStringLiteral("wifi"),
            QStringLiteral("ifname"), config.interfaceName,
            QStringLiteral("con-name"), config.connectionName,
            QStringLiteral("autoconnect"), QStringLiteral("yes"),
            QStringLiteral("ssid"), config.ssid,
            QStringLiteral("802-11-wireless.mode"), QStringLiteral("ap"),
            QStringLiteral("802-11-wireless.band"), QStringLiteral("bg"),
            QStringLiteral("ipv4.method"), QStringLiteral("shared"),
            QStringLiteral("ipv4.addresses"), config.addressCidr
        };
        if (!config.password.isEmpty()) {
            args << QStringLiteral("802-11-wireless-security.key-mgmt") << QStringLiteral("wpa-psk")
                 << QStringLiteral("802-11-wireless-security.psk") << config.password;
        }

        createProcess.start(QStringLiteral("nmcli"), args);
        createProcess.waitForFinished(5000);
        if (createProcess.exitCode() != 0) {
            if (error) *error = QString::fromUtf8(createProcess.readAllStandardError());
            return false;
        }
    }

    QProcess upProcess;
    upProcess.start(QStringLiteral("nmcli"), {QStringLiteral("connection"), QStringLiteral("up"), config.connectionName});
    upProcess.waitForFinished(5000);
    if (upProcess.exitCode() != 0) {
        if (error) *error = QString::fromUtf8(upProcess.readAllStandardError());
        return false;
    }
    return true;
}

bool HotspotManager::stop(const QString &connectionName, QString *error)
{
    QProcess downProcess;
    downProcess.start(QStringLiteral("nmcli"), {QStringLiteral("connection"), QStringLiteral("down"), connectionName});
    downProcess.waitForFinished(5000);
    if (downProcess.exitCode() != 0) {
        if (error) *error = QString::fromUtf8(downProcess.readAllStandardError());
        return false;
    }
    return true;
}
