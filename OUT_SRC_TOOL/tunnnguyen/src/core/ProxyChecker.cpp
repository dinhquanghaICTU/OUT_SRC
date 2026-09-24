#include "core/ProxyChecker.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QNetworkRequest>

SingleProxyChecker::SingleProxyChecker(const QString &rawProxy, int timeoutMs, QObject *parent)
    : QObject(parent), m_rawProxy(rawProxy.trimmed()), m_timeoutMs(timeoutMs)
{
    m_nam = new QNetworkAccessManager(this);
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &SingleProxyChecker::onTimeout);
}

void SingleProxyChecker::start()
{
    ProxyConfig cfg = ProxyConfig::fromString(m_rawProxy);
    if (!cfg.isEmpty()) {
        QNetworkProxy netProxy;
        if (cfg.protocol.toLower() == "socks5") {
            netProxy.setType(QNetworkProxy::Socks5Proxy);
        } else {
            netProxy.setType(QNetworkProxy::HttpProxy);
        }
        netProxy.setHostName(cfg.host);
        netProxy.setPort(cfg.port);
        if (!cfg.username.isEmpty()) {
            netProxy.setUser(cfg.username);
            netProxy.setPassword(cfg.password);
        }
        m_nam->setProxy(netProxy);
    } else {
        // Direct machine connection
        m_nam->setProxy(QNetworkProxy::NoProxy);
    }

    QNetworkRequest request(QUrl("http://ip-api.com/json?fields=status,message,country,countryCode,city,query"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    m_elapsed.start();
    m_reply = m_nam->get(request);
    connect(m_reply, &QNetworkReply::finished, this, &SingleProxyChecker::onReplyFinished);

    m_timer->start(m_timeoutMs);
}

void SingleProxyChecker::onReplyFinished()
{
    if (m_hasFinished) return;
    m_timer->stop();

    int ping = m_elapsed.elapsed();
    ProxyCheckResult result;
    result.rawProxy = m_rawProxy;

    if (m_reply->error() == QNetworkReply::NoError) {
        QByteArray data = m_reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            QString status = obj.value("status").toString();
            if (status == "success") {
                result.isLive = true;
                result.pingMs = ping;
                result.country = obj.value("country").toString();
                result.countryCode = obj.value("countryCode").toString().toUpper();
                result.publicIp = obj.value("query").toString();
                result.flagEmoji = countryCodeToFlag(result.countryCode);
            } else {
                result.isLive = false;
                result.errorMsg = obj.value("message").toString("Phản hồi lỗi");
            }
        } else {
            result.isLive = false;
            result.errorMsg = "Dữ liệu trả về không hợp lệ";
        }
    } else {
        result.isLive = false;
        result.errorMsg = m_reply->errorString();
    }

    m_hasFinished = true;
    m_reply->deleteLater();
    m_reply = nullptr;

    emit finished(result);
    deleteLater();
}

void SingleProxyChecker::onTimeout()
{
    if (m_hasFinished) return;
    m_hasFinished = true;

    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    ProxyCheckResult result;
    result.rawProxy = m_rawProxy;
    result.isLive = false;
    result.pingMs = -1;
    result.errorMsg = QString("Hết thời gian chờ (%1s) - Proxy Die").arg(m_timeoutMs / 1000);

    emit finished(result);
    deleteLater();
}

QString SingleProxyChecker::countryCodeToFlag(const QString &code)
{
    QString c = code.trimmed().toUpper();
    if (c.length() != 2) return "🌐";

    // Unicode regional indicator symbol logic
    char c1 = c[0].toLatin1();
    char c2 = c[1].toLatin1();

    if (c1 < 'A' || c1 > 'Z' || c2 < 'A' || c2 > 'Z') {
        return "🌐";
    }

    ushort high1 = 0xD83C;
    ushort low1 = 0xDDE6 + (c1 - 'A');
    ushort high2 = 0xD83C;
    ushort low2 = 0xDDE6 + (c2 - 'A');

    QString flag;
    flag.append(QChar(high1));
    flag.append(QChar(low1));
    flag.append(QChar(high2));
    flag.append(QChar(low2));

    return flag;
}
