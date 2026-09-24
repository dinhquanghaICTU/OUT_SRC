#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QElapsedTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QTimer>
#include "core/ProxyConfig.h"

struct ProxyCheckResult {
    QString rawProxy;
    bool isLive = false;
    int pingMs = -1;
    QString country;
    QString countryCode;
    QString publicIp;
    QString errorMsg;
    QString flagEmoji;
};

class SingleProxyChecker : public QObject {
    Q_OBJECT
public:
    explicit SingleProxyChecker(const QString &rawProxy, int timeoutMs = 8000, QObject *parent = nullptr);
    void start();

    static QString countryCodeToFlag(const QString &code);

signals:
    void finished(const ProxyCheckResult &result);

private slots:
    void onReplyFinished();
    void onTimeout();

private:
    QString m_rawProxy;
    int m_timeoutMs;
    QNetworkAccessManager *m_nam = nullptr;
    QNetworkReply *m_reply = nullptr;
    QTimer *m_timer = nullptr;
    QElapsedTimer m_elapsed;
    bool m_hasFinished = false;
};
