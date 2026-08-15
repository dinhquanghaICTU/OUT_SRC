#include "api/ApiServer.h"
#include "database/Database.h"
#include "mqtt/MqttDiscoveryService.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("ICTU Environment Server"));
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
    parser.addOption(portOption);
    parser.addOption(dbOption);
    parser.addOption(mqttHostOption);
    parser.addOption(mqttPortOption);
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

    MqttDiscoveryService mqttDiscovery(&database);
    ApiServer server(&database, &mqttDiscovery);
    if (!server.listen(quint16(portValue), &error)) {
        qCritical().noquote() << error;
        return 5;
    }

    mqttDiscovery.start(parser.value(mqttHostOption), quint16(mqttPortValue));

    qInfo().noquote() << QStringLiteral("ICTU server listening on 0.0.0.0:%1").arg(portValue);
    qInfo().noquote() << QStringLiteral("SQLite: %1").arg(databasePath);
    qInfo().noquote() << QStringLiteral("MQTT discovery: %1:%2")
                            .arg(parser.value(mqttHostOption)).arg(mqttPortValue);
    return app.exec();
}
