#pragma once

#include <QObject>
#include <QString>

class CdpClient : public QObject {
    Q_OBJECT
public:
    explicit CdpClient(QObject *parent = nullptr);
    ~CdpClient();

    void connectToBrowser(const QString &wsUrl);
    void disconnectBrowser();
    void sendCommand(const QString &method, const QString &paramsJson = "{}");

signals:
    void connected();
    void disconnected();
    void messageReceived(const QString &message);

private:
    bool m_isConnected = false;
};
