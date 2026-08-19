#pragma once

#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>

class Database;

class MqttDiscoveryService final : public QObject
{
    Q_OBJECT
public:
    explicit MqttDiscoveryService(Database *database, QObject *parent = nullptr);

    void start(const QString &host = QStringLiteral("127.0.0.1"), quint16 port = 1883);
    void stop();
    bool isConnected() const;

    bool publishRelayCommand(const QString &deviceId, const QString &commandId, bool state);
    bool publishDoorCommand(const QString &deviceId, const QString &commandId, const QString &action, const QJsonObject &params = {});
    bool publishDeviceConfig(const QString &deviceId, const QJsonObject &config);

signals:
    void brokerConnected();
    void brokerDisconnected();
    void telemetryReceived(const QString &deviceId, const QJsonObject &metrics);

private slots:
    void connectToBroker();
    void sendConnect();
    void sendSubscribe();
    void processPackets();
    void scheduleReconnect();

private:
    void processPublish(quint8 flags, const QByteArray &body);
    static QByteArray encodeRemainingLength(int length);
    static void appendUtf8(QByteArray &packet, const QByteArray &text);

    Database *m_database = nullptr;
    QTcpSocket m_socket;
    QTimer m_reconnectTimer;
    QTimer m_pingTimer;
    QString m_host = QStringLiteral("127.0.0.1");
    quint16 m_port = 1883;
    quint16 m_packetId = 1;
    QByteArray m_buffer;
    bool m_stopping = false;
    QHash<QString, qint64> m_lastTelemetryLogMs;
    QHash<QString, qint64> m_lastPresenceWriteMs;
};
