#include "ApiClient.h"
#include "config/AppConfig.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

namespace {

QString urlFor(const QString &path)
{
    return QStringLiteral("http://%1:%2%3")
        .arg(AppConfig::DefaultApiHost)
        .arg(AppConfig::DefaultApiPort)
        .arg(path);
}

} // namespace

ApiClient::ApiClient(QObject *parent) : QObject(parent) {}

QNetworkRequest ApiClient::makeRequest(const QString &path) const
{
    QNetworkRequest request(QUrl(urlFor(path)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    if (!m_accessToken.isEmpty())
        request.setRawHeader("Authorization", "Bearer " + m_accessToken.toUtf8());
    return request;
}

QString ApiClient::responseError(const QByteArray &body, const QString &fallback)
{
    const QJsonObject root = QJsonDocument::fromJson(body).object();
    const QJsonObject error = root.value(QStringLiteral("error")).toObject();
    const QString msg = error.value(QStringLiteral("message")).toString();
    return msg.isEmpty() ? fallback : msg;
}

void ApiClient::handleNetworkError(const QString &operation, QNetworkReply *reply)
{
    const QString body = QString::fromUtf8(reply->readAll());
    const QString message = responseError(body.toUtf8(), reply->errorString());
    emit networkError(QStringLiteral("%1: %2").arg(operation, message));
}

void ApiClient::login(const QString &username, const QString &password)
{
    auto *reply = m_networkManager.post(
        makeRequest(QStringLiteral("/api/auth/login")),
        QJsonDocument(QJsonObject{{"username", username}, {"password", password}}).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emit loginFailed(responseError(body, tr("Tài khoản hoặc mật khẩu không đúng")));
            return;
        }
        const QJsonObject root = QJsonDocument::fromJson(body).object();
        m_accessToken = root.value(QStringLiteral("token")).toString();
        const QString role = root.value(QStringLiteral("user")).toObject().value(QStringLiteral("role")).toString();
        emit loginSucceeded(role);
    });
}

void ApiClient::requestLatestReading()
{
    auto *reply = m_networkManager.get(makeRequest(QStringLiteral("/api/readings/latest")));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;
        const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        SensorReading r;
        r.soilMoisturePct = obj.value(QStringLiteral("soil_moisture")).toDouble(55.0);
        r.temperatureC = obj.value(QStringLiteral("temperature_c")).toDouble(
            obj.value(QStringLiteral("temperature")).toDouble(27.5));
        r.humidityPct = obj.value(QStringLiteral("humidity")).toDouble(
            obj.value(QStringLiteral("humidity_pct")).toDouble(65.0));
        r.pumpActive = obj.value(QStringLiteral("pump_active")).toBool(
            obj.value(QStringLiteral("relay")).toBool(false));
        r.waterTankLevelPct = obj.value(QStringLiteral("water_tank_level")).toDouble(85.0);
        r.measuredAt = QDateTime::currentDateTime();
        emit latestReadingReceived(r);
    });
}

void ApiClient::requestAlerts()
{
    auto *reply = m_networkManager.get(makeRequest(QStringLiteral("/api/alerts")));
    connect(reply, &QNetworkReply::finished, reply, &QObject::deleteLater);
}

void ApiClient::updateDeviceConfig(const DeviceConfig &config)
{
    QJsonObject obj{
        {"min_soil_moisture", config.minSoilMoistureTriggerPct},
        {"max_soil_moisture", config.maxSoilMoistureStopPct},
        {"max_pump_runtime_m", config.maxPumpRuntimeMinutes},
        {"auto_watering", config.autoWateringEnabled},
        {"sampling_interval_s", config.samplingIntervalSeconds}
    };
    auto *reply = m_networkManager.put(makeRequest(QStringLiteral("/api/config")), QJsonDocument(obj).toJson());
    connect(reply, &QNetworkReply::finished, reply, &QObject::deleteLater);
}

void ApiClient::requestMyDevice()
{
    if (m_devicesRequestInFlight) return;
    m_devicesRequestInFlight = true;

    auto *reply = m_networkManager.get(makeRequest(QStringLiteral("/api/devices/me")));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        m_devicesRequestInFlight = false;
        if (reply->error() != QNetworkReply::NoError) {
            handleNetworkError(tr("Tải danh sách thiết bị"), reply);
            return;
        }
        const QJsonObject root = QJsonDocument::fromJson(reply->readAll()).object();
        emit devicesReceived(root.value(QStringLiteral("data")).toArray());
    });
}

void ApiClient::claimDevice(const QString &deviceId, const QString &name)
{
    auto *reply = m_networkManager.post(
        makeRequest(QStringLiteral("/api/devices/claim")),
        QJsonDocument(QJsonObject{{"device_id", deviceId}, {"name", name}}).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const QByteArray body = reply->readAll();
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit operationFailed(responseError(body, tr("Không thể thêm thiết bị")));
            return;
        }
        m_devicesRequestInFlight = false;
        m_availableRequestInFlight = false;
        emit deviceClaimed(QJsonDocument::fromJson(body).object());
    });
}

void ApiClient::releaseDevice(const QString &deviceId)
{
    auto *reply = m_networkManager.post(
        makeRequest(QStringLiteral("/api/devices/release")),
        QJsonDocument(QJsonObject{{"device_id", deviceId}}).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, deviceId, reply] {
        const QByteArray body = reply->readAll();
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit operationFailed(responseError(body, tr("Không thể gỡ thiết bị")));
            return;
        }
        m_devicesRequestInFlight = false;
        m_availableRequestInFlight = false;
        emit deviceReleased(deviceId);
    });
}

void ApiClient::requestAvailableDevices()
{
    if (m_availableRequestInFlight) return;
    m_availableRequestInFlight = true;

    auto *reply = m_networkManager.get(makeRequest(QStringLiteral("/api/devices/available")));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        m_availableRequestInFlight = false;
        if (reply->error() != QNetworkReply::NoError) return;
        const QJsonObject root = QJsonDocument::fromJson(reply->readAll()).object();
        emit availableDevicesReceived(root.value(QStringLiteral("data")).toArray());
    });
}

void ApiClient::requestUsers()
{
    if (m_usersRequestInFlight) return;
    m_usersRequestInFlight = true;

    auto *reply = m_networkManager.get(makeRequest(QStringLiteral("/api/users")));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        m_usersRequestInFlight = false;
        if (reply->error() != QNetworkReply::NoError) {
            handleNetworkError(tr("Tải danh sách người dùng"), reply);
            return;
        }
        const QJsonObject root = QJsonDocument::fromJson(reply->readAll()).object();
        emit usersReceived(root.value(QStringLiteral("data")).toArray());
    });
}

void ApiClient::createUser(const QString &username, const QString &password, const QString &role)
{
    auto *reply = m_networkManager.post(
        makeRequest(QStringLiteral("/api/users")),
        QJsonDocument(QJsonObject{{"username", username}, {"password", password}, {"role", role}}).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit operationFailed(responseError(reply->readAll(), tr("Không thể tạo người dùng")));
            return;
        }
        emit userCreated();
    });
}

void ApiClient::updateUser(const QString &oldUsername, const QString &username,
                           const QString &password, const QString &role, bool enabled)
{
    QJsonObject body{{"username", username}, {"role", role}, {"enabled", enabled}};
    if (!password.isEmpty()) body.insert("password", password);

    auto *reply = m_networkManager.put(
        makeRequest(QStringLiteral("/api/users/%1").arg(oldUsername)),
        QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit operationFailed(responseError(reply->readAll(), tr("Không thể cập nhật người dùng")));
            return;
        }
        emit userUpdated();
    });
}

void ApiClient::deleteUser(const QString &username)
{
    auto *reply = m_networkManager.deleteResource(makeRequest(QStringLiteral("/api/users/%1").arg(username)));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit operationFailed(responseError(reply->readAll(), tr("Không thể xóa người dùng")));
            return;
        }
        emit userDeleted();
    });
}

void ApiClient::releaseUserDevice(const QString &username, const QString &deviceId)
{
    Q_UNUSED(username);
    releaseDevice(deviceId);
}

void ApiClient::setRelayState(const QString &deviceId, bool state)
{
    auto *reply = m_networkManager.post(
        makeRequest(QStringLiteral("/api/devices/%1/relay").arg(deviceId)),
        QJsonDocument(QJsonObject{{"state", state}}).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, deviceId, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit operationFailed(responseError(reply->readAll(), tr("Không thể gửi lệnh")));
            return;
        }
        emit relayCommandAccepted(deviceId);
    });
}

void ApiClient::sendPumpCommand(const QString &deviceId, bool state)
{
    setRelayState(deviceId, state);
}

void ApiClient::updatePerDeviceConfig(const QString &deviceId, const QJsonObject &config)
{
    auto *reply = m_networkManager.post(
        makeRequest(QStringLiteral("/api/devices/%1/config").arg(deviceId)),
        QJsonDocument(config).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, deviceId, reply] {
        reply->deleteLater();
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emit operationFailed(responseError(body, tr("Không thể lưu cấu hình")));
            return;
        }
        const QJsonObject res = QJsonDocument::fromJson(body).object();
        emit deviceConfigSaved(deviceId, res.value(QStringLiteral("mqtt_published")).toBool());
    });
}

void ApiClient::requestDeviceHistory(const QString &deviceId, const QString &period,
                                     const QString &date)
{
    QUrl url(urlFor(QStringLiteral("/api/devices/%1/history").arg(deviceId)));
    QUrlQuery q;
    if (!period.isEmpty()) q.addQueryItem(QStringLiteral("period"), period);
    if (!date.isEmpty()) q.addQueryItem(QStringLiteral("date"), date);
    url.setQuery(q);

    QNetworkRequest req(url);
    if (!m_accessToken.isEmpty())
        req.setRawHeader("Authorization", "Bearer " + m_accessToken.toUtf8());

    auto *reply = m_networkManager.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            handleNetworkError(tr("Tải lịch sử thiết bị"), reply);
            return;
        }
        emit deviceHistoryReceived(QJsonDocument::fromJson(reply->readAll()).object());
    });
}
