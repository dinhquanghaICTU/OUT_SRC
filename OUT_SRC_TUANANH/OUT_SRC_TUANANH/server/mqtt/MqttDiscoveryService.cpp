#include "MqttDiscoveryService.h"

#include "database/Database.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

MqttDiscoveryService::MqttDiscoveryService(Database *database, QObject *parent)
    : QObject(parent), m_database(database)
{
    m_reconnectTimer.setSingleShot(true);
    m_reconnectTimer.setInterval(3000);
    m_pingTimer.setInterval(20000);

    connect(&m_reconnectTimer, &QTimer::timeout, this, &MqttDiscoveryService::connectToBroker);
    connect(&m_pingTimer, &QTimer::timeout, this, [this] {
        if (m_socket.state() == QAbstractSocket::ConnectedState)
            m_socket.write(QByteArray::fromHex("c000"));
    });
    connect(&m_socket, &QTcpSocket::connected, this, &MqttDiscoveryService::sendConnect);
    connect(&m_socket, &QTcpSocket::readyRead, this, [this] {
        m_buffer.append(m_socket.readAll());
        processPackets();
    });
    connect(&m_socket, &QTcpSocket::disconnected, this, &MqttDiscoveryService::scheduleReconnect);
    connect(&m_socket, &QTcpSocket::errorOccurred, this,
            [this](QAbstractSocket::SocketError) { scheduleReconnect(); });
}

void MqttDiscoveryService::start(const QString &host, quint16 port)
{
    m_host = host;
    m_port = port;
    m_stopping = false;
    connectToBroker();
}

bool MqttDiscoveryService::publishRelayCommand(const QString &deviceId,
                                               const QString &commandId, bool state)
{
    if (m_socket.state() != QAbstractSocket::ConnectedState)
        return false;
    const QByteArray topic = QByteArrayLiteral("iot/v1/devices/")
        + deviceId.toUtf8() + QByteArrayLiteral("/commands");
    const QByteArray payload = QJsonDocument(QJsonObject{
        {"command_id", commandId},
        {"type", "relay.set"},
        {"params", QJsonObject{{"state", state}}}
    }).toJson(QJsonDocument::Compact);
    QByteArray body;
    appendUtf8(body, topic);
    const quint16 packetId = m_packetId++;
    body.append(char(packetId >> 8));
    body.append(char(packetId & 0xff));
    body.append(payload);
    QByteArray packet(1, char(0x32)); // PUBLISH QoS 1
    packet.append(encodeRemainingLength(body.size()));
    packet.append(body);
    return m_socket.write(packet) == packet.size();
}

bool MqttDiscoveryService::publishDeviceConfig(const QString &deviceId,
                                               const QJsonObject &config)
{
    if (m_socket.state() != QAbstractSocket::ConnectedState)
        return false;
    QJsonObject desired = config;
    desired.insert(QStringLiteral("config_version"),
                   QDateTime::currentMSecsSinceEpoch());
    const QByteArray topic = QByteArrayLiteral("iot/v1/devices/")
        + deviceId.toUtf8() + QByteArrayLiteral("/config/desired");
    const QByteArray payload = QJsonDocument(desired).toJson(QJsonDocument::Compact);
    QByteArray body;
    appendUtf8(body, topic);
    const quint16 packetId = m_packetId++;
    body.append(char(packetId >> 8));
    body.append(char(packetId & 0xff));
    body.append(payload);
    QByteArray packet(1, char(0x33)); // PUBLISH QoS 1, retained
    packet.append(encodeRemainingLength(body.size()));
    packet.append(body);
    return m_socket.write(packet) == packet.size();
}

void MqttDiscoveryService::connectToBroker()
{
    if (m_stopping || m_socket.state() != QAbstractSocket::UnconnectedState)
        return;
    qInfo().noquote() << QStringLiteral("MQTT discovery connecting to %1:%2").arg(m_host).arg(m_port);
    m_socket.connectToHost(m_host, m_port);
}

void MqttDiscoveryService::sendConnect()
{
    QByteArray body;
    appendUtf8(body, QByteArrayLiteral("MQTT"));
    body.append(char(4));       // MQTT 3.1.1
    body.append(char(2));       // clean session
    body.append(char(0));
    body.append(char(30));      // keepalive seconds
    const QByteArray clientId = QByteArrayLiteral("ictu-server-")
        + QByteArray::number(QRandomGenerator::global()->generate(), 16);
    appendUtf8(body, clientId);

    QByteArray packet(1, char(0x10));
    packet.append(encodeRemainingLength(body.size()));
    packet.append(body);
    m_socket.write(packet);
    m_pingTimer.start();
}

void MqttDiscoveryService::sendSubscribe()
{
    QByteArray body;
    const quint16 packetId = m_packetId++;
    body.append(char(packetId >> 8));
    body.append(char(packetId & 0xff));
    appendUtf8(body, QByteArrayLiteral("iot/v1/devices/+/telemetry"));
    body.append(char(1));
    appendUtf8(body, QByteArrayLiteral("iot/v1/devices/+/status"));
    body.append(char(1));
    appendUtf8(body, QByteArrayLiteral("iot/v1/devices/+/state"));
    body.append(char(1));

    QByteArray packet(1, char(0x82));
    packet.append(encodeRemainingLength(body.size()));
    packet.append(body);
    m_socket.write(packet);
}

void MqttDiscoveryService::processPackets()
{
    while (m_buffer.size() >= 2) {
        int multiplier = 1;
        int remaining = 0;
        int index = 1;
        quint8 encoded = 0;
        do {
            if (index >= m_buffer.size())
                return;
            encoded = quint8(m_buffer.at(index++));
            remaining += (encoded & 127) * multiplier;
            multiplier *= 128;
            if (multiplier > 128 * 128 * 128 * 128) {
                m_socket.abort();
                return;
            }
        } while (encoded & 128);

        if (m_buffer.size() < index + remaining)
            return;
        const quint8 header = quint8(m_buffer.at(0));
        const QByteArray body = m_buffer.mid(index, remaining);
        m_buffer.remove(0, index + remaining);

        const quint8 type = header >> 4;
        if (type == 2) { // CONNACK
            if (body.size() == 2 && body.at(1) == 0) {
                qInfo() << "MQTT discovery connected";
                sendSubscribe();
            } else {
                qWarning() << "MQTT broker rejected discovery client";
                m_socket.disconnectFromHost();
            }
        } else if (type == 3) {
            processPublish(header & 0x0f, body);
        }
    }
}

void MqttDiscoveryService::processPublish(quint8 flags, const QByteArray &body)
{
    if (body.size() < 2)
        return;
    const int topicLength = (quint8(body.at(0)) << 8) | quint8(body.at(1));
    if (topicLength <= 0 || body.size() < 2 + topicLength)
        return;

    int cursor = 2;
    const QString topic = QString::fromUtf8(body.mid(cursor, topicLength));
    cursor += topicLength;
    const int qos = (flags >> 1) & 0x03;
    quint16 packetId = 0;
    if (qos > 0) {
        if (body.size() < cursor + 2)
            return;
        packetId = (quint8(body.at(cursor)) << 8) | quint8(body.at(cursor + 1));
        cursor += 2;
    }
    const QByteArray payload = body.mid(cursor);

    if (qos == 1) {
        QByteArray ack;
        ack.append(char(0x40));
        ack.append(char(0x02));
        ack.append(char(packetId >> 8));
        ack.append(char(packetId & 0xff));
        m_socket.write(ack);
    }

    const QStringList parts = topic.split('/');
    if (parts.size() != 5 || parts.at(0) != QStringLiteral("iot")
        || parts.at(1) != QStringLiteral("v1") || parts.at(2) != QStringLiteral("devices"))
        return;
    const QString deviceId = parts.at(3);
    const QString channel = parts.at(4);
    bool online = true;
    QJsonObject metrics;
    if (channel == QStringLiteral("status")) {
        const QJsonObject status = QJsonDocument::fromJson(payload).object();
        online = status.value(QStringLiteral("online")).toBool(false);
    } else if (channel == QStringLiteral("state")) {
        const QJsonObject state = QJsonDocument::fromJson(payload).object();
        QString error;
        if (!m_database->recordDeviceState(deviceId, state, &error))
            qWarning().noquote() << "Cannot update device state:" << error;
        return;
    } else if (channel != QStringLiteral("telemetry")) {
        return;
    } else {
        metrics = QJsonDocument::fromJson(payload).object()
                      .value(QStringLiteral("metrics")).toObject();
        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        // Một device lỗi có thể bắn nhiều gói trong cùng mili-giây. Giới hạn log
        // còn 1 mẫu/giây để SQLite không chặn event loop điều khiển MQTT/HTTP.
        if (nowMs - m_lastTelemetryLogMs.value(deviceId, 0) >= 1000) {
            QString logError;
            if (!m_database->recordTelemetry(
                    deviceId, metrics,
                    QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs), &logError))
                qWarning().noquote() << "Cannot store telemetry log:" << logError;
            else
                m_lastTelemetryLogMs.insert(deviceId, nowMs);
        }
    }

    const qint64 presenceNow = QDateTime::currentMSecsSinceEpoch();
    if (channel == QStringLiteral("telemetry")
        && presenceNow - m_lastPresenceWriteMs.value(deviceId, 0) < 500)
        return;
    QString error;
    if (!m_database->recordDevicePresence(deviceId, online, metrics, &error))
        qWarning().noquote() << "Cannot update device presence:" << error;
    else
        m_lastPresenceWriteMs.insert(deviceId, presenceNow);
}

void MqttDiscoveryService::scheduleReconnect()
{
    m_pingTimer.stop();
    m_buffer.clear();
    if (!m_stopping && !m_reconnectTimer.isActive())
        m_reconnectTimer.start();
}

QByteArray MqttDiscoveryService::encodeRemainingLength(int length)
{
    QByteArray encoded;
    do {
        quint8 byte = length % 128;
        length /= 128;
        if (length > 0)
            byte |= 128;
        encoded.append(char(byte));
    } while (length > 0);
    return encoded;
}

void MqttDiscoveryService::appendUtf8(QByteArray &packet, const QByteArray &text)
{
    packet.append(char((text.size() >> 8) & 0xff));
    packet.append(char(text.size() & 0xff));
    packet.append(text);
}
