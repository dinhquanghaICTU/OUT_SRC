#include "HotspotManager.h"

#include <QDebug>
#include <QProcess>
#include <QStringList>

namespace
{
bool runCommand(const QString &program, const QStringList &args, QString *output, QString *error, int timeoutMs = 20000)
{
    QProcess process;
    process.start(program, args);

    if (!process.waitForStarted(5000)) {
        if (error) *error = QStringLiteral("Không chạy được %1").arg(program);
        return false;
    }

    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished(2000);
        if (error) *error = QStringLiteral("%1 timeout: %2").arg(program, args.join(' '));
        return false;
    }

    const QString stdoutText = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    const QString stderrText = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
    if (output) *output = stdoutText;

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        if (error) {
            *error = stderrText.isEmpty()
                         ? QStringLiteral("%1 lỗi: %2").arg(program, args.join(' '))
                         : stderrText;
        }
        return false;
    }

    return true;
}

bool runNmcli(const QStringList &args, QString *output, QString *error)
{
    return runCommand(QStringLiteral("nmcli"), args, output, error);
}

bool runIw(const QStringList &args, QString *output, QString *error)
{
    return runCommand(QStringLiteral("iw"), args, output, error, 8000);
}

bool connectionExists(const QString &name)
{
    QString output;
    QString error;
    return runNmcli({QStringLiteral("-t"), QStringLiteral("-f"), QStringLiteral("NAME"),
                     QStringLiteral("connection"), QStringLiteral("show"), name},
                    &output, &error);
}

bool networkInterfaceExists(const QString &name)
{
    QString output;
    QString error;
    return runIw({QStringLiteral("dev"), name, QStringLiteral("info")}, &output, &error);
}

bool ensureVirtualApInterface(const QString &parentName, const QString &apName, QString *error)
{
    if (apName.trimmed().isEmpty() || parentName.trimmed().isEmpty() || apName == parentName) {
        return true;
    }

    if (networkInterfaceExists(apName)) {
        return true;
    }

    QString localError;
    qInfo().noquote() << QStringLiteral("Hotspot: tạo interface ảo %1 từ %2").arg(apName, parentName);
    if (!runIw({QStringLiteral("dev"), parentName, QStringLiteral("interface"),
                QStringLiteral("add"), apName, QStringLiteral("type"), QStringLiteral("__ap")},
               nullptr, &localError)) {
        if (error) {
            *error = QStringLiteral("Không tạo được interface ảo %1 từ %2: %3")
                         .arg(apName, parentName, localError);
        }
        return false;
    }
    return true;
}
} // namespace

bool HotspotManager::ensureStarted(const Config &config, QString *error)
{
    if (config.ssid.trimmed().isEmpty()) {
        if (error) *error = QStringLiteral("SSID hotspot rỗng");
        return false;
    }
    if (config.password.size() < 8) {
        if (error) *error = QStringLiteral("Mật khẩu hotspot phải tối thiểu 8 ký tự");
        return false;
    }

    QString localError;
    if (!ensureVirtualApInterface(config.parentInterfaceName, config.interfaceName, &localError)) {
        if (error) *error = localError;
        return false;
    }

    if (!connectionExists(config.connectionName)) {
        qInfo().noquote() << QStringLiteral("Hotspot: tạo connection %1").arg(config.connectionName);
        if (!runNmcli({QStringLiteral("connection"), QStringLiteral("add"),
                       QStringLiteral("type"), QStringLiteral("wifi"),
                       QStringLiteral("ifname"), config.interfaceName,
                       QStringLiteral("con-name"), config.connectionName,
                       QStringLiteral("autoconnect"), QStringLiteral("yes"),
                       QStringLiteral("ssid"), config.ssid},
                      nullptr, &localError)) {
            if (error) *error = localError;
            return false;
        }
    }

    const QStringList modifyArgs{
        QStringLiteral("connection"), QStringLiteral("modify"), config.connectionName,
        QStringLiteral("connection.interface-name"), config.interfaceName,
        QStringLiteral("802-11-wireless.mode"), QStringLiteral("ap"),
        QStringLiteral("802-11-wireless.band"), QStringLiteral("bg"),
        QStringLiteral("802-11-wireless.ssid"), config.ssid,
        QStringLiteral("wifi-sec.key-mgmt"), QStringLiteral("wpa-psk"),
        QStringLiteral("wifi-sec.psk"), config.password,
        QStringLiteral("ipv4.method"), QStringLiteral("shared"),
        QStringLiteral("ipv4.addresses"), config.addressCidr,
        QStringLiteral("ipv6.method"), QStringLiteral("ignore")
    };

    if (!runNmcli(modifyArgs, nullptr, &localError)) {
        if (error) *error = localError;
        return false;
    }

    if (!runNmcli({QStringLiteral("connection"), QStringLiteral("up"), config.connectionName},
                  nullptr, &localError)) {
        if (error) *error = localError;
        return false;
    }

    qInfo().noquote() << QStringLiteral("Hotspot ready: iface=%1 SSID=%2 IP=%3")
                             .arg(config.interfaceName, config.ssid, config.addressCidr);
    return true;
}
