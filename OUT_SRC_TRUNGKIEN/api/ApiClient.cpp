#include "ApiClient.h"

#include "config/AppConfig.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
{
}

QNetworkRequest ApiClient::makeRequest(const QString &path) const
{
    QNetworkRequest request(QUrl(AppConfig::ApiBaseUrl + path));
    request.setTransferTimeout(2500);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    if (!m_accessToken.isEmpty())
        request.setRawHeader("Authorization", "Bearer " + m_accessToken.toUtf8());
    return request;
}

void ApiClient::login(const QString &username, const QString &password)
{
    const QJsonObject body{{"username", username}, {"password", password}};
    QNetworkReply *reply = m_networkManager.post(
        makeRequest(QStringLiteral("/api/auth/login")), QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const QByteArray responseBody = reply->readAll();
        const QJsonDocument document = QJsonDocument::fromJson(responseBody);
        if (reply->error() != QNetworkReply::NoError) {
            emit loginFailed(responseError(responseBody, reply->errorString()));
        } else {
            m_accessToken = document.object().value(QStringLiteral("token")).toString();
            const QString role = document.object().value(QStringLiteral("user"))
                                     .toObject().value(QStringLiteral("role")).toString();
            emit loginSucceeded(role);
        }
        reply->deleteLater();
    });
}

void ApiClient::requestLatestReading()
{
    QNetworkReply *reply = m_networkManager.get(makeRequest(QStringLiteral("/api/readings/latest")));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        if (reply->error() != QNetworkReply::NoError) {
            handleNetworkError(QStringLiteral("latest reading"), reply);
            reply->deleteLater();
            return;
        }

        const QJsonObject object = QJsonDocument::fromJson(reply->readAll()).object();
        SensorReading reading;
        reading.pressureHpa = object.value(QStringLiteral("pressure_hpa")).toDouble();
        reading.distanceCm = object.value(QStringLiteral("distance_cm")).toDouble();
        reading.temperatureC = object.value(QStringLiteral("temperature_c")).toDouble();
        reading.measuredAt = QDateTime::fromString(
            object.value(QStringLiteral("measured_at")).toString(), Qt::ISODate);
        emit latestReadingReceived(reading);
        reply->deleteLater();
    });
}

void ApiClient::requestPressureHistory()
{
    m_networkManager.get(makeRequest(QStringLiteral("/api/pressure/history")))->deleteLater();
}

void ApiClient::requestDistanceHistory()
{
    m_networkManager.get(makeRequest(QStringLiteral("/api/distance/history")))->deleteLater();
}

void ApiClient::requestAlerts()
{
    m_networkManager.get(makeRequest(QStringLiteral("/api/alerts")))->deleteLater();
}

void ApiClient::updateDeviceConfig(const DeviceConfig &config)
{
    const QJsonObject body{
        {"pressure_min_hpa", config.minimumPressureHpa},
        {"pressure_max_hpa", config.maximumPressureHpa},
        {"distance_min_cm", config.minimumDistanceCm},
        {"sampling_interval_seconds", config.samplingIntervalSeconds}
    };
    m_networkManager.put(makeRequest(QStringLiteral("/api/config")),
                         QJsonDocument(body).toJson())->deleteLater();
}

void ApiClient::requestMyDevice()
{
    if (m_devicesRequestInFlight)
        return;
    m_devicesRequestInFlight = true;
    QNetworkReply *reply = m_networkManager.get(makeRequest(QStringLiteral("/api/devices/me")));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        m_devicesRequestInFlight = false;
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emit operationFailed(responseError(body, reply->errorString()));
        } else {
            emit devicesReceived(QJsonDocument::fromJson(body).object()
                                     .value(QStringLiteral("data")).toArray());
        }
        reply->deleteLater();
    });
}

void ApiClient::claimDevice(const QString &deviceId, const QString &name)
{
    const QJsonObject payload{{"device_id", deviceId.trimmed()}, {"name", name.trimmed()}};
    QNetworkReply *reply = m_networkManager.post(
        makeRequest(QStringLiteral("/api/devices/claim")), QJsonDocument(payload).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError)
            emit operationFailed(responseError(body, reply->errorString()));
        else
            emit deviceClaimed(QJsonDocument::fromJson(body).object());
        reply->deleteLater();
    });
}

void ApiClient::requestAvailableDevices()
{
    if (m_availableRequestInFlight)
        return;
    m_availableRequestInFlight = true;
    QNetworkReply *reply = m_networkManager.get(
        makeRequest(QStringLiteral("/api/devices/available")));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        m_availableRequestInFlight = false;
        const QByteArray body = reply->readAll();
        if (reply->error() == QNetworkReply::NoError)
            emit availableDevicesReceived(QJsonDocument::fromJson(body).object()
                                              .value(QStringLiteral("data")).toArray());
        reply->deleteLater();
    });
}

void ApiClient::requestUsers()
{
    if (m_usersRequestInFlight)
        return;
    m_usersRequestInFlight = true;
    QNetworkReply *reply = m_networkManager.get(makeRequest(QStringLiteral("/api/admin/users")));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        m_usersRequestInFlight = false;
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError)
            emit operationFailed(responseError(body, reply->errorString()));
        else
            emit usersReceived(QJsonDocument::fromJson(body).object()
                                   .value(QStringLiteral("data")).toArray());
        reply->deleteLater();
    });
}

void ApiClient::createUser(const QString &username, const QString &password, const QString &role)
{
    const QJsonObject payload{{"username", username.trimmed()},
                              {"password", password},
                              {"role", role}};
    QNetworkReply *reply = m_networkManager.post(
        makeRequest(QStringLiteral("/api/admin/users")), QJsonDocument(payload).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError)
            emit operationFailed(responseError(body, reply->errorString()));
        else
            emit userCreated();
        reply->deleteLater();
    });
}

void ApiClient::updateUser(const QString &oldUsername, const QString &username,
                           const QString &password, const QString &role, bool enabled)
{
    const QJsonObject payload{{"username", username.trimmed()},
                              {"password", password},
                              {"role", role},
                              {"enabled", enabled}};
    QNetworkReply *reply = m_networkManager.put(
        makeRequest(QStringLiteral("/api/admin/users/%1")
                        .arg(QString::fromUtf8(QUrl::toPercentEncoding(oldUsername.trimmed())))),
        QJsonDocument(payload).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError)
            emit operationFailed(responseError(body, reply->errorString()));
        else
            emit userUpdated();
        reply->deleteLater();
    });
}

void ApiClient::deleteUser(const QString &username)
{
    QNetworkReply *reply = m_networkManager.deleteResource(
        makeRequest(QStringLiteral("/api/admin/users/%1")
                        .arg(QString::fromUtf8(QUrl::toPercentEncoding(username.trimmed())))));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError)
            emit operationFailed(responseError(body, reply->errorString()));
        else
            emit userDeleted();
        reply->deleteLater();
    });
}

void ApiClient::releaseUserDevice(const QString &username, const QString &deviceId)
{
    QNetworkReply *reply = m_networkManager.deleteResource(
        makeRequest(QStringLiteral("/api/admin/users/%1/devices/%2")
                        .arg(QString::fromUtf8(QUrl::toPercentEncoding(username.trimmed())),
                             QString::fromUtf8(QUrl::toPercentEncoding(deviceId.trimmed())))));
    connect(reply, &QNetworkReply::finished, this, [this, reply, username, deviceId] {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError)
            emit operationFailed(responseError(body, reply->errorString()));
        else
            emit userDeviceReleased(username, deviceId);
        reply->deleteLater();
    });
}

void ApiClient::setRelayState(const QString &deviceId, bool state)
{
    const QJsonObject payload{{"device_id", deviceId}, {"state", state}};
    QNetworkReply *reply = m_networkManager.post(
        makeRequest(QStringLiteral("/api/devices/relay")), QJsonDocument(payload).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, deviceId] {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError)
            emit operationFailed(responseError(body, reply->errorString()));
        else
            emit relayCommandAccepted(deviceId);
        reply->deleteLater();
    });
}

void ApiClient::releaseDevice(const QString &deviceId)
{
    const QJsonObject payload{{"device_id", deviceId}};
    QNetworkReply *reply = m_networkManager.post(
        makeRequest(QStringLiteral("/api/devices/release")), QJsonDocument(payload).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, deviceId] {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError)
            emit operationFailed(responseError(body, reply->errorString()));
        else
            emit deviceReleased(deviceId);
        reply->deleteLater();
    });
}

void ApiClient::updatePerDeviceConfig(const QString &deviceId, const QJsonObject &config)
{
    const QJsonObject payload{{"device_id", deviceId}, {"config", config}};
    QNetworkReply *reply = m_networkManager.put(
        makeRequest(QStringLiteral("/api/devices/config")), QJsonDocument(payload).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, deviceId] {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emit operationFailed(responseError(body, reply->errorString()));
        } else {
            const bool published = QJsonDocument::fromJson(body).object()
                                       .value(QStringLiteral("mqtt_published")).toBool();
            emit deviceConfigSaved(deviceId, published);
        }
        reply->deleteLater();
    });
}

void ApiClient::requestDeviceHistory(const QString &deviceId, const QString &period,
                                     const QString &date)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("device_id"), deviceId);
    query.addQueryItem(QStringLiteral("period"), period);
    query.addQueryItem(QStringLiteral("date"), date);
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("500"));
    QNetworkReply *reply = m_networkManager.get(makeRequest(
        QStringLiteral("/api/devices/history?") + query.toString(QUrl::FullyEncoded)));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError)
            emit operationFailed(responseError(body, reply->errorString()));
        else
            emit deviceHistoryReceived(QJsonDocument::fromJson(body).object());
        reply->deleteLater();
    });
}

void ApiClient::handleNetworkError(const QString &operation, QNetworkReply *reply)
{
    emit networkError(operation + QStringLiteral(": ") + reply->errorString());
}

QString ApiClient::responseError(const QByteArray &body, const QString &fallback)
{
    const QJsonObject error = QJsonDocument::fromJson(body).object()
                                  .value(QStringLiteral("error")).toObject();
    const QString message = error.value(QStringLiteral("message")).toString();
    return message.isEmpty() ? fallback : message;
}
