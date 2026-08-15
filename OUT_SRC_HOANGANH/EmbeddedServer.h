#ifndef EMBEDDEDSERVER_H
#define EMBEDDEDSERVER_H

#include <QObject>
#include <QString>
#include <memory>

class ApiServer;
class Database;
class MqttDiscoveryService;

class EmbeddedServer final : public QObject
{
    Q_OBJECT
public:
    struct Config
    {
        quint16 httpPort = 8080;
        QString databasePath;
        QString mqttHost = QStringLiteral("127.0.0.1");
        quint16 mqttPort = 1883;
        bool enableHotspot = true;
        QString hotspotInterface = QStringLiteral("wlan1");
        QString hotspotName = QStringLiteral("ICTU_IOT_AP");
        QString hotspotSsid = QStringLiteral("ICTU_IOT_AP");
        QString hotspotPassword = QStringLiteral("12345678");
        QString hotspotAddressCidr = QStringLiteral("192.168.4.1/24");
    };

    explicit EmbeddedServer(QObject *parent = nullptr);
    ~EmbeddedServer() override;

    bool start(const Config &config, QString *error = nullptr);
    bool start(QString *error = nullptr);
    QString baseUrl() const;
    QString databasePath() const;

private:
    std::unique_ptr<Database> m_database;
    std::unique_ptr<MqttDiscoveryService> m_mqtt;
    std::unique_ptr<ApiServer> m_api;
    Config m_config;
    QString m_databasePath;
    bool m_started = false;
};

#endif // EMBEDDEDSERVER_H
