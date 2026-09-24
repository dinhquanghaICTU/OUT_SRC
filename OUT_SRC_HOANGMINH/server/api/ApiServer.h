#pragma once

#include <QHash>
#include <QHttpServer>
#include <QJsonObject>
#include <QObject>

class Database;
class MqttDiscoveryService;
class QHttpServerRequest;
class QHttpServerResponse;

class ApiServer final : public QObject
{
    Q_OBJECT
public:
    explicit ApiServer(Database *database, MqttDiscoveryService *mqtt,
                       QObject *parent = nullptr);
    bool listen(quint16 port, QString *error);

private:
    void registerRoutes();
    bool authorized(const QHttpServerRequest &request) const;
    QJsonObject sessionForRequest(const QHttpServerRequest &request) const;
    QString createToken(const QString &username, const QString &role);
    static int requestedLimit(const QHttpServerRequest &request);

    Database *m_database;
    MqttDiscoveryService *m_mqtt;
    QHttpServer m_server;
    QHash<QString, QJsonObject> m_sessions;
};
