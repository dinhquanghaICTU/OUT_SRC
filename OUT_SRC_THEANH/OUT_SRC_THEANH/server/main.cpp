#include "api/ApiServer.h"
#include "database/Database.h"
#include "mqtt/MqttDiscoveryService.h"
#include "system/HotspotManager.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("HoangAnh IoT Server"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral(
        "Raspberry Pi sensor API and SQLite service"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption portOption({QStringLiteral("p"), QStringLiteral("port")},
                                  QStringLiteral("HTTP listen port"), QStringLiteral("port"),
                                  QStringLiteral("8080"));
    QCommandLineOption dbOption({QStringLiteral("d"), QStringLiteral("database")},
                                QStringLiteral("SQLite database path"), QStringLiteral("path"));
    QCommandLineOption mqttHostOption(QStringLiteral("mqtt-host"),
                                      QStringLiteral("MQTT broker host"),
                                      QStringLiteral("host"), QStringLiteral("127.0.0.1"));
    QCommandLineOption mqttPortOption(QStringLiteral("mqtt-port"),
                                      QStringLiteral("MQTT broker port"),
                                      QStringLiteral("port"), QStringLiteral("1883"));
    QCommandLineOption noHotspotOption(QStringLiteral("no-hotspot"),
                                       QStringLiteral("Không bật WiFi AP bằng nmcli"));
    QCommandLineOption hotspotIfaceOption(QStringLiteral("hotspot-iface"),
                                          QStringLiteral("WiFi interface phát AP"),
                                          QStringLiteral("iface"), QStringLiteral("wlan1"));
    QCommandLineOption hotspotNameOption(QStringLiteral("hotspot-name"),
                                         QStringLiteral("Tên NetworkManager connection"),
                                         QStringLiteral("name"), QStringLiteral("ICTU_IOT_AP"));
    QCommandLineOption hotspotSsidOption(QStringLiteral("hotspot-ssid"),
                                         QStringLiteral("SSID phát cho ESP"),
                                         QStringLiteral("ssid"), QStringLiteral("ICTU_IOT_AP"));
    QCommandLineOption hotspotPassOption(QStringLiteral("hotspot-pass"),
                                         QStringLiteral("Mật khẩu WiFi AP"),
                                         QStringLiteral("password"), QStringLiteral("12345678"));
    QCommandLineOption hotspotIpOption(QStringLiteral("hotspot-ip"),
                                       QStringLiteral("IP/CIDR của Pi trong mạng AP"),
                                       QStringLiteral("cidr"), QStringLiteral("192.168.4.1/24"));
    parser.addOption(portOption);
    parser.addOption(dbOption);
    parser.addOption(mqttHostOption);
    parser.addOption(mqttPortOption);
    parser.addOption(noHotspotOption);
    parser.addOption(hotspotIfaceOption);
    parser.addOption(hotspotNameOption);
    parser.addOption(hotspotSsidOption);
    parser.addOption(hotspotPassOption);
    parser.addOption(hotspotIpOption);
    parser.process(app);

    bool portOk = false;
    const int portValue = parser.value(portOption).toInt(&portOk);
    if (!portOk || portValue < 1 || portValue > 65535) {
        qCritical() << "Invalid port";
        return 2;
    }
    bool mqttPortOk = false;
    const int mqttPortValue = parser.value(mqttPortOption).toInt(&mqttPortOk);
    if (!mqttPortOk || mqttPortValue < 1 || mqttPortValue > 65535) {
        qCritical() << "Invalid MQTT port";
        return 2;
    }

    QString databasePath = parser.value(dbOption);
    if (databasePath.isEmpty()) {
        const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        if (!QDir().mkpath(dataDir)) {
            qCritical() << "Cannot create data directory:" << dataDir;
            return 3;
        }
        databasePath = dataDir + QStringLiteral("/environment.db");
    }

    Database database(databasePath);
    QString error;
    if (!database.open(&error)) {
        qCritical().noquote() << "Database startup failed:" << error;
        return 4;
    }

    if (!parser.isSet(noHotspotOption)) {
        HotspotManager::Config hotspot;
        hotspot.interfaceName = parser.value(hotspotIfaceOption);
        hotspot.connectionName = parser.value(hotspotNameOption);
        hotspot.ssid = parser.value(hotspotSsidOption);
        hotspot.password = parser.value(hotspotPassOption);
        hotspot.addressCidr = parser.value(hotspotIpOption);

        QString hotspotError;
        if (!HotspotManager::ensureStarted(hotspot, &hotspotError)) {
            qWarning().noquote() << QStringLiteral("Hotspot startup warning: %1").arg(hotspotError);
            qWarning().noquote() << QStringLiteral("Server vẫn chạy. Nếu muốn bỏ qua AP: --no-hotspot");
        }
    }

    MqttDiscoveryService mqttDiscovery(&database);
    ApiServer server(&database, &mqttDiscovery);
    if (!server.listen(quint16(portValue), &error)) {
        qCritical().noquote() << error;
        return 5;
    }

    mqttDiscovery.start(parser.value(mqttHostOption), quint16(mqttPortValue));

    qInfo().noquote() << QStringLiteral("HoangAnh server listening on 0.0.0.0:%1").arg(portValue);
    qInfo().noquote() << QStringLiteral("SQLite: %1").arg(databasePath);
    qInfo().noquote() << QStringLiteral("MQTT discovery local broker: %1:%2")
                            .arg(parser.value(mqttHostOption)).arg(mqttPortValue);
    return app.exec();
}
