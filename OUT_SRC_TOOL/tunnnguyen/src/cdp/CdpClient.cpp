#include "cdp/CdpClient.h"
#include <QDebug>

CdpClient::CdpClient(QObject *parent)
    : QObject(parent)
{
}

CdpClient::~CdpClient()
{
    disconnectBrowser();
}

void CdpClient::connectToBrowser(const QString &wsUrl)
{
    Q_UNUSED(wsUrl);
    // TODO: Connect via QWebSocket
    m_isConnected = true;
    emit connected();
}

void CdpClient::disconnectBrowser()
{
    if (m_isConnected) {
        m_isConnected = false;
        emit disconnected();
    }
}

void CdpClient::sendCommand(const QString &method, const QString &paramsJson)
{
    Q_UNUSED(method);
    Q_UNUSED(paramsJson);
    // TODO: Send JSON-RPC via WebSocket
}
