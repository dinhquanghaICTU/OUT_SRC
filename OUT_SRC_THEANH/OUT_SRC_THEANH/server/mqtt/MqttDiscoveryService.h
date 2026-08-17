#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QHash>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>

class Database;

class MqttDiscoveryService final : public QObject
{
    Q_OBJECT
public:
    explicit MqttDiscoveryService(Database *database, QObject *parent = nullptr);
    void start(const QString &host, quint16 port);
    bool publishRelayCommand(const QString &deviceId, const QString &commandId, bool state);
    bool publishDeviceConfig(const QString &deviceId, const QJsonObject &config);

private:
    void connectToBroker();
    void sendConnect();
    void sendSubscribe();
    void processPackets();
    void processPublish(quint8 flags, const QByteArray &body);
    void scheduleReconnect();
    static QByteArray encodeRemainingLength(int length);
    static void appendUtf8(QByteArray &packet, const QByteArray &text);

    Database *m_database;
    QTcpSocket m_socket;
    QTimer m_reconnectTimer;
    QTimer m_pingTimer;
    QByteArray m_buffer;
    QString m_host;
    quint16 m_port = 1883;
    quint16 m_packetId = 1;
    bool m_stopping = false;
    QHash<QString, qint64> m_lastPresenceWriteMs;
    QHash<QString, qint64> m_lastTelemetryLogMs;
};
