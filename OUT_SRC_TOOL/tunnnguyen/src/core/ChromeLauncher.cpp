#include "core/ChromeLauncher.h"
#include "core/ProxyConfig.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

ChromeLauncher::ChromeLauncher(QObject *parent)
    : QObject(parent), m_process(new QProcess(this))
{
    connect(m_process, &QProcess::started, this, &ChromeLauncher::processStarted);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this](int exitCode, QProcess::ExitStatus) {
                emit processFinished(exitCode);
            });
}

ChromeLauncher::~ChromeLauncher()
{
    stop();
}

bool ChromeLauncher::launch(int port, const QString &profilePath, const QString &proxy,
                            int width, int height, int posX, int posY, double scale,
                            const QString &userAgent)
{
    QString program = "google-chrome";
#ifdef Q_OS_WIN
    program = "C:/Program Files/Google/Chrome/Application/chrome.exe";
#endif

    // Pre-seed window placement in Default/Preferences if profile path is provided
    if (!profilePath.isEmpty() && width > 0 && height > 0 && posX >= 0 && posY >= 0) {
        QString defaultDir = profilePath + "/Default";
        QDir().mkpath(defaultDir);
        QString prefPath = defaultDir + "/Preferences";
        QFile prefFile(prefPath);
        QJsonObject root;
        if (prefFile.exists() && prefFile.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(prefFile.readAll());
            if (doc.isObject()) {
                root = doc.object();
            }
            prefFile.close();
        }

        QJsonObject browserObj = root.value("browser").toObject();
        QJsonObject placementObj;
        placementObj["bottom"] = posY + height;
        placementObj["left"] = posX;
        placementObj["right"] = posX + width;
        placementObj["top"] = posY;
        placementObj["maximized"] = false;
        browserObj["window_placement"] = placementObj;
        root["browser"] = browserObj;

        if (prefFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            prefFile.write(QJsonDocument(root).toJson());
            prefFile.close();
        }
    }

    QStringList arguments;
    arguments << QString("--remote-debugging-port=%1").arg(port);
    if (!profilePath.isEmpty()) {
        arguments << QString("--user-data-dir=%1").arg(profilePath);
    }
    if (!proxy.trimmed().isEmpty()) {
        ProxyConfig pCfg = ProxyConfig::fromString(proxy);
        if (!pCfg.isEmpty()) {
            arguments << QString("--proxy-server=%1").arg(pCfg.toServerUrl());
            if (pCfg.hasAuth() && !profilePath.isEmpty()) {
                // Generate proxy auth extension to auto authenticate without prompt dialog
                QString extDir = profilePath + "/proxy_auth_ext";
                QDir().mkpath(extDir);

                QFile manifestFile(extDir + "/manifest.json");
                if (manifestFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    QString manifestContent = R"({
  "version": "1.0.0",
  "manifest_version": 2,
  "name": "Proxy Auth Helper",
  "permissions": [
    "proxy",
    "tabs",
    "unlimitedStorage",
    "storage",
    "<all_urls>",
    "webRequest",
    "webRequestBlocking"
  ],
  "background": {
    "scripts": ["background.js"]
  }
})";
                    manifestFile.write(manifestContent.toUtf8());
                    manifestFile.close();
                }

                QFile bgFile(extDir + "/background.js");
                if (bgFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    QString bgContent = QString(R"(chrome.webRequest.onAuthRequired.addListener(
    function(details) {
        return {
            authCredentials: {
                username: "%1",
                password: "%2"
            }
        };
    },
    {urls: ["<all_urls>"]},
    ['blocking']
);)").arg(pCfg.username, pCfg.password);
                    bgFile.write(bgContent.toUtf8());
                    bgFile.close();
                }

                arguments << QString("--load-extension=%1").arg(extDir);
            }
        } else {
            QString p = proxy.trimmed();
            if (!p.contains("://")) {
                p = "http://" + p;
            }
            arguments << QString("--proxy-server=%1").arg(p);
        }
    }
    if (!userAgent.trimmed().isEmpty()) {
        arguments << QString("--user-agent=%1").arg(userAgent.trimmed());
    }

#ifndef Q_OS_WIN
    // Essential for Linux (Wayland sessions): Chrome defaults to Wayland where
    // the Mutter compositor blocks manual window placement.
    // Specifying --ozone-platform=x11 routes Chrome through XWayland, which strictly
    // honors --window-position and --window-size for grid/tile window arrangement!
    arguments << "--ozone-platform=x11";
#endif

    if (width > 0 && height > 0) {
        arguments << QString("--window-size=%1,%2").arg(width).arg(height);
    }
    if (posX >= 0 && posY >= 0) {
        arguments << QString("--window-position=%1,%2").arg(posX).arg(posY);
    }
    if (scale > 0 && scale != 1.0) {
        arguments << QString("--force-device-scale-factor=%1").arg(scale);
    } else if (width > 0 && width < 480) {
        // Auto-scale compact windows so Chrome's minimum tab bar frame shrinks to match
        double autoScale = (double)width / 500.0;
        if (autoScale < 0.75) autoScale = 0.75;
        arguments << QString("--force-device-scale-factor=%1").arg(autoScale, 0, 'f', 2);
    }

    arguments << "--disable-blink-features=AutomationControlled";
    arguments << "--no-first-run";
    arguments << "--no-default-browser-check";

    m_process->start(program, arguments);
    return m_process->waitForStarted();
}

void ChromeLauncher::stop()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        if (!m_process->waitForFinished(2000)) {
            m_process->kill();
        }
    }
}
