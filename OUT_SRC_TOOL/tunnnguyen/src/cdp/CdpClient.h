#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QTcpSocket>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>

class CdpClient : public QObject {
    Q_OBJECT
public:
    explicit CdpClient(QObject *parent = nullptr);
    ~CdpClient() override;

    void connectToUrl(const QUrl &wsUrl);
    void disconnectFromHost();
    bool isConnected() const { return m_isConnected; }
    int targetPort() const { return m_port; }

    int sendCommand(const QString &method, const QJsonObject &params = QJsonObject());

signals:
    void connected();
    void disconnected();
    void messageReceived(const QJsonObject &message);
    void eventReceived(const QString &method, const QJsonObject &params);
    void errorOccurred(const QString &error);

private slots:
    void onSocketConnected();
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);

private:
    void sendHandshake(const QString &host, int port, const QString &path);
    void processIncomingData();
    void sendFrame(quint8 opcode, const QByteArray &payload);

    QTcpSocket *m_socket = nullptr;
    QUrl m_url;
    int m_port = 0;
    bool m_handshakeDone = false;
    bool m_isConnected = false;
    QByteArray m_secWebSocketKey;
    QByteArray m_readBuffer;
    int m_commandId = 1;
};
