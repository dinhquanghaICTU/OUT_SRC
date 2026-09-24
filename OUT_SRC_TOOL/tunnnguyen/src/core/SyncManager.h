#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QUrl>

class CdpClient;

struct SyncOptions {
    bool syncMouse = true;
    bool syncKeyboard = true;
    bool syncScroll = true;
    bool syncNavigation = true;
    int randomDelayMs = 0; // 0 to 200 ms
};

class SyncManager : public QObject {
    Q_OBJECT
public:
    explicit SyncManager(QObject *parent = nullptr);
    ~SyncManager() override;

    bool isSyncing() const { return m_isSyncing; }
    int masterPort() const { return m_masterPort; }
    QList<int> workerPorts() const { return m_workerPorts; }
    SyncOptions options() const { return m_options; }

    void setOptions(const SyncOptions &opts) { m_options = opts; }

    void startSync(int masterPort, const QList<int> &workerPorts, const SyncOptions &options);
    void stopSync();

signals:
    void syncStarted(int masterPort, int workerCount);
    void syncStopped();
    void syncError(const QString &error);
    void statusMessage(const QString &msg);

private slots:
    void onMasterConnected();
    void onMasterDisconnected();
    void onMasterEvent(const QString &method, const QJsonObject &params);
    void onMasterError(const QString &err);

private:
    void fetchTargetWsUrl(int port, std::function<void(const QUrl &wsUrl)> callback);
    void injectMasterTracker();
    void dispatchToWorkers(const QString &type, const QJsonObject &eventObj);
    void broadcastCommand(const QString &method, const QJsonObject &params);

    bool m_isSyncing = false;
    int m_masterPort = 0;
    QList<int> m_workerPorts;
    SyncOptions m_options;

    CdpClient *m_masterClient = nullptr;
    QList<CdpClient*> m_workers;
    QNetworkAccessManager *m_nam = nullptr;
};
