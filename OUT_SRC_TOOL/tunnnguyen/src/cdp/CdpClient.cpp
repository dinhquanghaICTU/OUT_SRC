#include "cdp/CdpClient.h"
#include <QRandomGenerator>
#include <QDebug>

CdpClient::CdpClient(QObject *parent)
    : QObject(parent), m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &CdpClient::onSocketConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &CdpClient::onSocketReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &CdpClient::onSocketDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &CdpClient::onSocketError);
}

CdpClient::~CdpClient()
{
    disconnectFromHost();
}

void CdpClient::connectToUrl(const QUrl &wsUrl)
{
    disconnectFromHost();
    m_url = wsUrl;
    m_port = wsUrl.port(80);
    m_handshakeDone = false;
    m_isConnected = false;
    m_readBuffer.clear();

    QString host = wsUrl.host();
    if (host.isEmpty()) host = "127.0.0.1";

    m_socket->connectToHost(host, m_port);
}

void CdpClient::disconnectFromHost()
{
    if (m_socket && m_socket->state() != QAbstractSocket::UnconnectedState) {
        if (m_handshakeDone && m_isConnected) {
            sendFrame(0x08, QByteArray()); // Close frame
        }
        m_socket->disconnectFromHost();
    }
    m_isConnected = false;
    m_handshakeDone = false;
    m_readBuffer.clear();
}

void CdpClient::onSocketConnected()
{
    QString host = m_url.host();
    if (host.isEmpty()) host = "127.0.0.1";
    QString path = m_url.path();
    if (path.isEmpty()) path = "/";
    sendHandshake(host, m_port, path);
}

void CdpClient::sendHandshake(const QString &host, int port, const QString &path)
{
    // 16 random bytes base64 encoded
    QByteArray rawKey(16, 0);
    for (int i = 0; i < 16; ++i) {
        rawKey[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    m_secWebSocketKey = rawKey.toBase64();

    QByteArray request;
    request.append(QString("GET %1 HTTP/1.1\r\n").arg(path).toUtf8());
    request.append(QString("Host: %1:%2\r\n").arg(host).arg(port).toUtf8());
    request.append("Upgrade: websocket\r\n");
    request.append("Connection: Upgrade\r\n");
    request.append("Sec-WebSocket-Key: " + m_secWebSocketKey + "\r\n");
    request.append("Sec-WebSocket-Version: 13\r\n");
    request.append("\r\n");

    m_socket->write(request);
}

void CdpClient::onSocketReadyRead()
{
    m_readBuffer.append(m_socket->readAll());

    if (!m_handshakeDone) {
        int idx = m_readBuffer.indexOf("\r\n\r\n");
        if (idx != -1) {
            QByteArray headerBytes = m_readBuffer.left(idx);
            m_readBuffer.remove(0, idx + 4);

            if (headerBytes.contains(" 101 ") || headerBytes.contains("101 WebSocket") || headerBytes.contains("101 Switching Protocols")) {
                m_handshakeDone = true;
                m_isConnected = true;
                emit connected();
            } else {
                emit errorOccurred("Handshake failed: " + QString::fromUtf8(headerBytes));
                disconnectFromHost();
                return;
            }
        } else {
            return; // Wait for full HTTP headers
        }
    }

    if (m_handshakeDone) {
        processIncomingData();
    }
}

void CdpClient::processIncomingData()
{
    while (m_readBuffer.size() >= 2) {
        const quint8 *data = reinterpret_cast<const quint8*>(m_readBuffer.constData());
        quint8 b0 = data[0];
        quint8 b1 = data[1];

        quint8 opcode = b0 & 0x0F;
        bool masked = (b1 & 0x80) != 0;
        quint64 payloadLen = b1 & 0x7F;
        int headerLen = 2;

        if (payloadLen == 126) {
            if (m_readBuffer.size() < 4) return;
            payloadLen = (quint8(data[2]) << 8) | quint8(data[3]);
            headerLen = 4;
        } else if (payloadLen == 127) {
            if (m_readBuffer.size() < 10) return;
            payloadLen = 0;
            for (int i = 0; i < 8; ++i) {
                payloadLen = (payloadLen << 8) | quint8(data[2 + i]);
            }
            headerLen = 10;
        }

        if (masked) {
            if (m_readBuffer.size() < headerLen + 4) return;
            headerLen += 4;
        }

        if (static_cast<quint64>(m_readBuffer.size()) < static_cast<quint64>(headerLen + payloadLen)) {
            return; // Incomplete frame, wait for more data
        }

        QByteArray payload = m_readBuffer.mid(headerLen, static_cast<int>(payloadLen));
        if (masked) {
            const char *mask = m_readBuffer.constData() + (headerLen - 4);
            for (int i = 0; i < payload.size(); ++i) {
                payload[i] = payload[i] ^ mask[i % 4];
            }
        }

        m_readBuffer.remove(0, headerLen + static_cast<int>(payloadLen));

        if (opcode == 0x01) { // Text frame
            QJsonDocument doc = QJsonDocument::fromJson(payload);
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                emit messageReceived(obj);
                if (obj.contains("method")) {
                    emit eventReceived(obj.value("method").toString(), obj.value("params").toObject());
                }
            }
        } else if (opcode == 0x09) { // Ping
            sendFrame(0x0A, payload); // Pong
        } else if (opcode == 0x08) { // Close
            disconnectFromHost();
            return;
        }
    }
}

void CdpClient::sendFrame(quint8 opcode, const QByteArray &payload)
{
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) return;

    QByteArray frame;
    frame.append(static_cast<char>(0x80 | (opcode & 0x0F))); // FIN = 1

    quint64 len = static_cast<quint64>(payload.size());
    if (len <= 125) {
        frame.append(static_cast<char>(0x80 | len)); // MASK = 1
    } else if (len <= 65535) {
        frame.append(static_cast<char>(0x80 | 126));
        frame.append(static_cast<char>((len >> 8) & 0xFF));
        frame.append(static_cast<char>(len & 0xFF));
    } else {
        frame.append(static_cast<char>(0x80 | 127));
        for (int i = 7; i >= 0; --i) {
            frame.append(static_cast<char>((len >> (i * 8)) & 0xFF));
        }
    }

    // 4 masking bytes (RFC 6455 requires masking from client to server)
    char mask[4];
    for (int i = 0; i < 4; ++i) {
        mask[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    frame.append(mask, 4);

    QByteArray maskedPayload = payload;
    for (int i = 0; i < maskedPayload.size(); ++i) {
        maskedPayload[i] = maskedPayload[i] ^ mask[i % 4];
    }
    frame.append(maskedPayload);

    m_socket->write(frame);
}

int CdpClient::sendCommand(const QString &method, const QJsonObject &params)
{
    int id = m_commandId++;
    QJsonObject cmd;
    cmd["id"] = id;
    cmd["method"] = method;
    if (!params.isEmpty()) {
        cmd["params"] = params;
    }

    QByteArray json = QJsonDocument(cmd).toJson(QJsonDocument::Compact);
    sendFrame(0x01, json);
    return id;
}

void CdpClient::onSocketDisconnected()
{
    m_isConnected = false;
    m_handshakeDone = false;
    emit disconnected();
}

void CdpClient::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit errorOccurred(m_socket->errorString());
}
