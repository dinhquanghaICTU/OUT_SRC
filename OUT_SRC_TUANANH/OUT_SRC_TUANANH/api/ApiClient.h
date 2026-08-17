#pragma once

#include "models/DeviceConfig.h"
#include "models/SensorReading.h"

#include <QNetworkAccessManager>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

class ApiClient : public QObject
{
    Q_OBJECT

public:
    explicit ApiClient(QObject *parent = nullptr);

    void login(const QString &username, const QString &password);
    void requestLatestReading();
    void requestPressureHistory();
    void requestDistanceHistory();
    void requestAlerts();
    void updateDeviceConfig(const DeviceConfig &config);
    void requestMyDevice();
    void claimDevice(const QString &deviceId, const QString &name);
    void releaseDevice(const QString &deviceId);
    void requestAvailableDevices();
    void requestUsers();
    void createUser(const QString &username, const QString &password, const QString &role);
    void updateUser(const QString &oldUsername, const QString &username,
                    const QString &password, const QString &role, bool enabled);
    void deleteUser(const QString &username);
    void releaseUserDevice(const QString &username, const QString &deviceId);
    void setRelayState(const QString &deviceId, bool state);
    void updatePerDeviceConfig(const QString &deviceId, const QJsonObject &config);
    void requestDeviceHistory(const QString &deviceId, const QString &period,
                              const QString &date);

signals:
    void loginSucceeded(const QString &role);
    void loginFailed(const QString &message);
    void latestReadingReceived(const SensorReading &reading);
    void devicesReceived(const QJsonArray &devices);
    void deviceClaimed(const QJsonObject &device);
    void deviceReleased(const QString &deviceId);
    void availableDevicesReceived(const QJsonArray &devices);
    void usersReceived(const QJsonArray &users);
    void userCreated();
    void userUpdated();
    void userDeleted();
    void userDeviceReleased(const QString &username, const QString &deviceId);
    void relayCommandAccepted(const QString &deviceId);
    void deviceConfigSaved(const QString &deviceId, bool mqttPublished);
    void deviceHistoryReceived(const QJsonObject &history);
    void operationFailed(const QString &message);
    void networkError(const QString &message);

private:
    QNetworkRequest makeRequest(const QString &path) const;
    void handleNetworkError(const QString &operation, QNetworkReply *reply);
    static QString responseError(const QByteArray &body, const QString &fallback);

    QNetworkAccessManager m_networkManager;
    QString m_accessToken;
    bool m_devicesRequestInFlight = false;
    bool m_availableRequestInFlight = false;
    bool m_usersRequestInFlight = false;
};
