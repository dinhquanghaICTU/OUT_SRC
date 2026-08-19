#pragma once

#include <QObject>
#include <QString>
#include <memory>

class Database;
class MqttDiscoveryService;
class ApiServer;

class EmbeddedServer final : public QObject
{
    Q_OBJECT
public:
    struct Config
    {
        QString databasePath;
        quint16 httpPort = 8080;
        QString mqttHost = QStringLiteral("127.0.0.1");
        quint16 mqttPort = 1883;

        bool enableHotspot = false;
        QString hotspotInterface = QStringLiteral("wlan0");
        QString hotspotName = QStringLiteral("MANHQUANG-AP");
        QString hotspotSsid = QStringLiteral("MANHQUANG-AP");
        QString hotspotPassword = QStringLiteral("12345678");
        QString hotspotAddressCidr = QStringLiteral("192.168.4.1/24");
    };

    explicit EmbeddedServer(QObject *parent = nullptr);
    ~EmbeddedServer() override;

    bool start(QString *error = nullptr);
    bool start(const Config &config, QString *error = nullptr);

    QString baseUrl() const;
    QString databasePath() const;

private:
    Config m_config;
    bool m_started = false;
    QString m_databasePath;
    std::unique_ptr<Database> m_database;
    std::unique_ptr<MqttDiscoveryService> m_mqtt;
    std::unique_ptr<ApiServer> m_api;
};
