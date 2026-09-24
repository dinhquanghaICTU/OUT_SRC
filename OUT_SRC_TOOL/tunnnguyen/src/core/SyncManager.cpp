#include "core/SyncManager.h"
#include "cdp/CdpClient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QTimer>
#include <QDebug>

SyncManager::SyncManager(QObject *parent)
    : QObject(parent),
      m_masterClient(new CdpClient(this)),
      m_nam(new QNetworkAccessManager(this))
{
    connect(m_masterClient, &CdpClient::connected, this, &SyncManager::onMasterConnected);
    connect(m_masterClient, &CdpClient::disconnected, this, &SyncManager::onMasterDisconnected);
    connect(m_masterClient, &CdpClient::eventReceived, this, &SyncManager::onMasterEvent);
    connect(m_masterClient, &CdpClient::errorOccurred, this, &SyncManager::onMasterError);
}

SyncManager::~SyncManager()
{
    stopSync();
}

void SyncManager::startSync(int masterPort, const QList<int> &workerPorts, const SyncOptions &options)
{
    stopSync();

    m_masterPort = masterPort;
    m_workerPorts = workerPorts;
    m_options = options;
    m_isSyncing = true;

    emit statusMessage(QString("Đang kết nối Profile Mẹ (Port %1)...").arg(masterPort));

    // 1. Connect Master
    fetchTargetWsUrl(masterPort, [this, masterPort](const QUrl &wsUrl) {
        if (!m_isSyncing || wsUrl.isEmpty()) {
            emit syncError(QString("Không thể kết nối Profile Mẹ tại Port %1").arg(masterPort));
            stopSync();
            return;
        }
        m_masterClient->connectToUrl(wsUrl);
    });

    // 2. Connect Workers
    for (int port : workerPorts) {
        if (port == masterPort) continue;

        fetchTargetWsUrl(port, [this, port](const QUrl &wsUrl) {
            if (!m_isSyncing || wsUrl.isEmpty()) return;

            auto *worker = new CdpClient(this);
            connect(worker, &CdpClient::connected, [worker]() {
                worker->sendCommand("Page.enable");
                worker->sendCommand("Runtime.enable");
            });
            worker->connectToUrl(wsUrl);
            m_workers.append(worker);
        });
    }

    emit syncStarted(m_masterPort, m_workerPorts.size());
}

void SyncManager::stopSync()
{
    if (!m_isSyncing && m_workers.isEmpty() && !m_masterClient->isConnected()) return;

    m_isSyncing = false;
    m_masterPort = 0;
    m_workerPorts.clear();

    if (m_masterClient) {
        m_masterClient->disconnectFromHost();
    }

    for (CdpClient *w : m_workers) {
        if (w) {
            w->disconnectFromHost();
            w->deleteLater();
        }
    }
    m_workers.clear();

    emit syncStopped();
}

void SyncManager::fetchTargetWsUrl(int port, std::function<void(const QUrl &wsUrl)> callback)
{
    QUrl url(QString("http://127.0.0.1:%1/json/list").arg(port));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, [reply, callback]() {
        QUrl wsUrl;
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isArray()) {
                QJsonArray arr = doc.array();
                for (const QJsonValue &val : arr) {
                    QJsonObject obj = val.toObject();
                    if (obj.value("type").toString() == "page") {
                        QString ws = obj.value("webSocketDebuggerUrl").toString();
                        if (!ws.isEmpty()) {
                            wsUrl = QUrl(ws);
                            break;
                        }
                    }
                }
            }
        }
        reply->deleteLater();
        callback(wsUrl);
    });
}

void SyncManager::onMasterConnected()
{
    emit statusMessage(QString("Đã kết nối Profile Mẹ thành công!"));

    // Enable Page and Runtime domains
    m_masterClient->sendCommand("Page.enable");
    m_masterClient->sendCommand("Runtime.enable");

    injectMasterTracker();
}

void SyncManager::injectMasterTracker()
{
    const char *trackerScript = R"JS(
(function() {
    if (window.__sync_injected_v2) return;
    window.__sync_injected_v2 = true;

    function emitSync(data) {
        try {
            console.debug('__SYNC_EVENT__:' + JSON.stringify(data));
        } catch(e) {}
    }

    window.addEventListener('mousedown', function(e) {
        emitSync({
            type: 'mousePressed',
            x: Math.round(e.clientX),
            y: Math.round(e.clientY),
            button: e.button === 2 ? 'right' : (e.button === 1 ? 'middle' : 'left'),
            clickCount: 1,
            modifiers: (e.ctrlKey ? 2 : 0) | (e.shiftKey ? 8 : 0) | (e.altKey ? 1 : 0) | (e.metaKey ? 4 : 0)
        });
    }, true);

    window.addEventListener('mouseup', function(e) {
        emitSync({
            type: 'mouseReleased',
            x: Math.round(e.clientX),
            y: Math.round(e.clientY),
            button: e.button === 2 ? 'right' : (e.button === 1 ? 'middle' : 'left'),
            clickCount: 1,
            modifiers: (e.ctrlKey ? 2 : 0) | (e.shiftKey ? 8 : 0) | (e.altKey ? 1 : 0) | (e.metaKey ? 4 : 0)
        });
    }, true);

    window.addEventListener('wheel', function(e) {
        emitSync({
            type: 'mouseWheel',
            x: Math.round(e.clientX),
            y: Math.round(e.clientY),
            deltaX: Math.round(e.deltaX),
            deltaY: Math.round(e.deltaY)
        });
    }, { passive: true, capture: true });

    window.addEventListener('keydown', function(e) {
        emitSync({
            type: 'rawKeyDown',
            key: e.key,
            code: e.code,
            keyCode: e.keyCode,
            text: e.key.length === 1 ? e.key : '',
            modifiers: (e.ctrlKey ? 2 : 0) | (e.shiftKey ? 8 : 0) | (e.altKey ? 1 : 0) | (e.metaKey ? 4 : 0)
        });
    }, true);

    window.addEventListener('keyup', function(e) {
        emitSync({
            type: 'keyUp',
            key: e.key,
            code: e.code,
            keyCode: e.keyCode,
            modifiers: (e.ctrlKey ? 2 : 0) | (e.shiftKey ? 8 : 0) | (e.altKey ? 1 : 0) | (e.metaKey ? 4 : 0)
        });
    }, true);
})();
)JS";

    // 1. Inject into all future documents
    QJsonObject addScriptParams;
    addScriptParams["source"] = QString::fromUtf8(trackerScript);
    m_masterClient->sendCommand("Page.addScriptToEvaluateOnNewDocument", addScriptParams);

    // 2. Inject immediately into current page
    QJsonObject evalParams;
    evalParams["expression"] = QString::fromUtf8(trackerScript);
    m_masterClient->sendCommand("Runtime.evaluate", evalParams);
}

void SyncManager::onMasterEvent(const QString &method, const QJsonObject &params)
{
    if (method == "Runtime.consoleAPICalled") {
        QJsonArray args = params.value("args").toArray();
        if (!args.isEmpty()) {
            QString val = args[0].toObject().value("value").toString();
            if (val.startsWith("__SYNC_EVENT__:")) {
                QString jsonStr = val.mid(15);
                QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
                if (doc.isObject()) {
                    QJsonObject ev = doc.object();
                    dispatchToWorkers(ev.value("type").toString(), ev);
                }
            }
        }
    } else if (method == "Page.frameNavigated") {
        if (m_options.syncNavigation) {
            QJsonObject frame = params.value("frame").toObject();
            if (!frame.contains("parentId")) {
                QString url = frame.value("url").toString();
                if (!url.isEmpty() && !url.startsWith("chrome://") && !url.startsWith("devtools://") && url != "about:blank") {
                    QJsonObject p;
                    p["url"] = url;
                    broadcastCommand("Page.navigate", p);
                }
            }
        }
    }
}

void SyncManager::dispatchToWorkers(const QString &type, const QJsonObject &ev)
{
    if (type == "mousePressed" || type == "mouseReleased") {
        if (!m_options.syncMouse) return;

        QJsonObject p;
        p["type"] = type;
        p["x"] = ev.value("x").toDouble();
        p["y"] = ev.value("y").toDouble();
        p["button"] = ev.value("button").toString("left");
        p["clickCount"] = ev.value("clickCount").toInt(1);
        if (ev.contains("modifiers")) p["modifiers"] = ev.value("modifiers").toInt();
        broadcastCommand("Input.dispatchMouseEvent", p);

    } else if (type == "mouseWheel") {
        if (!m_options.syncScroll) return;

        QJsonObject p;
        p["type"] = "mouseWheel";
        p["x"] = ev.value("x").toDouble();
        p["y"] = ev.value("y").toDouble();
        p["deltaX"] = ev.value("deltaX").toDouble();
        p["deltaY"] = ev.value("deltaY").toDouble();
        broadcastCommand("Input.dispatchMouseEvent", p);

    } else if (type == "rawKeyDown") {
        if (!m_options.syncKeyboard) return;

        QJsonObject p;
        p["type"] = "rawKeyDown";
        p["key"] = ev.value("key").toString();
        p["code"] = ev.value("code").toString();
        p["windowsVirtualKeyCode"] = ev.value("keyCode").toInt();
        if (ev.contains("modifiers")) p["modifiers"] = ev.value("modifiers").toInt();
        QString text = ev.value("text").toString();
        if (!text.isEmpty()) {
            p["text"] = text;
            p["unmodifiedText"] = text;
        }
        broadcastCommand("Input.dispatchKeyEvent", p);

    } else if (type == "keyUp") {
        if (!m_options.syncKeyboard) return;

        QJsonObject p;
        p["type"] = "keyUp";
        p["key"] = ev.value("key").toString();
        p["code"] = ev.value("code").toString();
        p["windowsVirtualKeyCode"] = ev.value("keyCode").toInt();
        if (ev.contains("modifiers")) p["modifiers"] = ev.value("modifiers").toInt();
        broadcastCommand("Input.dispatchKeyEvent", p);
    }
}

void SyncManager::broadcastCommand(const QString &method, const QJsonObject &params)
{
    if (!m_isSyncing) return;
    int delayBase = m_options.randomDelayMs;

    for (CdpClient *worker : m_workers) {
        if (!worker || !worker->isConnected()) continue;

        if (delayBase <= 0) {
            worker->sendCommand(method, params);
        } else {
            int delay = QRandomGenerator::global()->bounded(delayBase + 1);
            QPointer<CdpClient> safeWorker = worker;
            QTimer::singleShot(delay, this, [safeWorker, method, params]() {
                if (safeWorker && safeWorker->isConnected()) {
                    safeWorker->sendCommand(method, params);
                }
            });
        }
    }
}

void SyncManager::onMasterDisconnected()
{
    if (m_isSyncing) {
        emit statusMessage("Profile Mẹ đã bị ngắt kết nối.");
        stopSync();
    }
}

void SyncManager::onMasterError(const QString &err)
{
    emit syncError("Lỗi kết nối Profile Mẹ: " + err);
}
