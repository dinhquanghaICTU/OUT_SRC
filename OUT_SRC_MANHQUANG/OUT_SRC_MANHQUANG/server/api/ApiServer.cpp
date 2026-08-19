#include "ApiServer.h"
#include "database/Database.h"
#include "mqtt/MqttDiscoveryService.h"

#include <QDateTime>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QUrlQuery>
#include <QUuid>

namespace {

using Status = QHttpServerResponse::StatusCode;

QHttpServerResponse jsonError(Status status, const QString &code, const QString &message)
{
    return QHttpServerResponse(
        QJsonObject{{"error", QJsonObject{{"code", code}, {"message", message}}}}, status);
}

QJsonObject parseObject(const QHttpServerRequest &request, bool *ok)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(request.body(), &parseError);
    *ok = parseError.error == QJsonParseError::NoError && document.isObject();
    return *ok ? document.object() : QJsonObject{};
}

} // namespace

ApiServer::ApiServer(Database *database, MqttDiscoveryService *mqtt, QObject *parent)
    : QObject(parent), m_database(database), m_mqtt(mqtt)
{
    registerRoutes();
}

bool ApiServer::listen(quint16 port, QString *error)
{
    const quint16 boundPort = m_server.listen(QHostAddress::Any, port);
    if (boundPort != 0)
        return true;
    if (error)
        *error = tr("Không thể lắng nghe cổng %1").arg(port);
    return false;
}

void ApiServer::registerRoutes()
{
    m_server.route(QStringLiteral("/api/health"), QHttpServerRequest::Method::Get,
                   [this] {
        return QJsonObject{{"status", m_database->isOpen() ? "ok" : "degraded"},
                           {"service", "manhquang-door-server"},
                           {"time", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}};
    });

    m_server.route(QStringLiteral("/api/auth/login"), QHttpServerRequest::Method::Post,
                   [this](const QHttpServerRequest &request) {
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        if (!ok)
            return jsonError(Status::BadRequest, QStringLiteral("invalid_json"),
                             tr("Nội dung JSON không hợp lệ"));
        const QString username = body.value(QStringLiteral("username")).toString();
        const QString password = body.value(QStringLiteral("password")).toString();
        if (username.isEmpty() || password.isEmpty())
            return jsonError(Status::BadRequest, QStringLiteral("validation_error"),
                             tr("Thiếu tài khoản hoặc mật khẩu"));
        QString role;
        if (!m_database->verifyUser(username, password, &role))
            return jsonError(Status::Unauthorized, QStringLiteral("invalid_credentials"),
                             tr("Tài khoản hoặc mật khẩu không đúng"));
        const QString token = createToken(username, role);
        return QHttpServerResponse(QJsonObject{{"token", token},
                                               {"token_type", "Bearer"},
                                               {"expires_in", 28800},
                                               {"user", QJsonObject{{"username", username},
                                                                    {"role", role}}}});
    });

    m_server.route(QStringLiteral("/api/devices/me"), QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        QString error;
        const QJsonArray devices = m_database->devicesForUser(
            session.value(QStringLiteral("username")).toString(), 30, &error);
        if (!error.isEmpty())
            return jsonError(Status::InternalServerError, QStringLiteral("database_error"), error);
        return QHttpServerResponse(QJsonObject{{"data", devices}, {"count", devices.size()}});
    });

    m_server.route(QStringLiteral("/api/devices/available"), QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest &request) {
        if (!authorized(request))
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        QString error;
        const QJsonArray devices = m_database->availableDevices(30, &error);
        if (!error.isEmpty())
            return jsonError(Status::InternalServerError, QStringLiteral("database_error"), error);
        return QHttpServerResponse(QJsonObject{{"data", devices}, {"count", devices.size()}});
    });

    m_server.route(QStringLiteral("/api/devices/claim"), QHttpServerRequest::Method::Post,
                   [this](const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        if (!ok)
            return jsonError(Status::BadRequest, QStringLiteral("invalid_json"),
                             tr("Nội dung JSON không hợp lệ"));
        const QString deviceId = body.value(QStringLiteral("device_id")).toString();
        const QString name = body.value(QStringLiteral("name")).toString(deviceId);
        if (deviceId.isEmpty())
            return jsonError(Status::BadRequest, QStringLiteral("validation_error"),
                             tr("Thiếu device_id"));

        QString code, error;
        if (!m_database->claimDevice(session.value(QStringLiteral("username")).toString(),
                                     deviceId, name, &code, &error))
            return jsonError(Status::BadRequest, code, error);

        return QHttpServerResponse(QJsonObject{{"status", "claimed"}, {"device_id", deviceId}, {"name", name}});
    });

    m_server.route(QStringLiteral("/api/devices/release"), QHttpServerRequest::Method::Post,
                   [this](const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        if (!ok)
            return jsonError(Status::BadRequest, QStringLiteral("invalid_json"),
                             tr("Nội dung JSON không hợp lệ"));
        const QString deviceId = body.value(QStringLiteral("device_id")).toString();
        if (deviceId.isEmpty())
            return jsonError(Status::BadRequest, QStringLiteral("validation_error"),
                             tr("Thiếu device_id"));

        QString code, error;
        if (!m_database->releaseDevice(session.value(QStringLiteral("username")).toString(),
                                       deviceId, &code, &error))
            return jsonError(Status::BadRequest, code, error);

        return QHttpServerResponse(QJsonObject{{"status", "released"}, {"device_id", deviceId}});
    });

    m_server.route(QStringLiteral("/api/devices/<arg>/relay"), QHttpServerRequest::Method::Post,
                   [this](const QString &deviceId, const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        if (!ok)
            return jsonError(Status::BadRequest, QStringLiteral("invalid_json"),
                             tr("Nội dung JSON không hợp lệ"));
        const bool state = body.value(QStringLiteral("state")).toBool(false);
        const QString commandId = body.value(QStringLiteral("command_id"))
                                      .toString(QUuid::createUuid().toString(QUuid::WithoutBraces));

        if (!m_mqtt->publishRelayCommand(deviceId, commandId, state))
            return jsonError(Status::ServiceUnavailable, QStringLiteral("mqtt_unavailable"),
                             tr("MQTT broker chưa kết nối hoặc gửi lệnh thất bại"));

        return QHttpServerResponse(QJsonObject{{"command_id", commandId}, {"status", "accepted"}});
    });

    m_server.route(QStringLiteral("/api/devices/<arg>/config"), QHttpServerRequest::Method::Post,
                   [this](const QString &deviceId, const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        if (!ok)
            return jsonError(Status::BadRequest, QStringLiteral("invalid_json"),
                             tr("Nội dung JSON không hợp lệ"));

        QString code, error;
        if (!m_database->updateDeviceConfig(session.value(QStringLiteral("username")).toString(),
                                            deviceId, body, &code, &error))
            return jsonError(Status::BadRequest, code, error);

        const bool mqttPublished = m_mqtt->publishDeviceConfig(deviceId, body);
        return QHttpServerResponse(QJsonObject{{"status", "saved"},
                                               {"device_id", deviceId},
                                               {"mqtt_published", mqttPublished}});
    });

    m_server.route(QStringLiteral("/api/devices/<arg>/history"), QHttpServerRequest::Method::Get,
                   [this](const QString &deviceId, const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));

        const QUrlQuery query(request.url().query());
        const QString period = query.queryItemValue(QStringLiteral("period"));
        const QString date = query.queryItemValue(QStringLiteral("date"));
        const int limit = requestedLimit(request);

        QString error;
        const QJsonObject history = m_database->deviceTelemetryHistory(
            session.value(QStringLiteral("username")).toString(), deviceId, period, date, limit, &error);
        if (!error.isEmpty())
            return jsonError(Status::InternalServerError, QStringLiteral("database_error"), error);

        return QHttpServerResponse(history);
    });

    m_server.route(QStringLiteral("/api/users"), QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty() || session.value(QStringLiteral("role")).toString() != QStringLiteral("admin"))
            return jsonError(Status::Forbidden, QStringLiteral("forbidden"), tr("Yêu cầu quyền admin"));
        QString error;
        const QJsonArray list = m_database->users(&error);
        if (!error.isEmpty())
            return jsonError(Status::InternalServerError, QStringLiteral("database_error"), error);
        return QHttpServerResponse(QJsonObject{{"data", list}, {"count", list.size()}});
    });

    m_server.route(QStringLiteral("/api/users"), QHttpServerRequest::Method::Post,
                   [this](const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty() || session.value(QStringLiteral("role")).toString() != QStringLiteral("admin"))
            return jsonError(Status::Forbidden, QStringLiteral("forbidden"), tr("Yêu cầu quyền admin"));
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        if (!ok)
            return jsonError(Status::BadRequest, QStringLiteral("invalid_json"), tr("JSON không hợp lệ"));

        const QString username = body.value(QStringLiteral("username")).toString();
        const QString password = body.value(QStringLiteral("password")).toString();
        const QString role = body.value(QStringLiteral("role")).toString(QStringLiteral("user"));

        QString code, error;
        if (!m_database->createUser(username, password, role, &code, &error))
            return jsonError(Status::BadRequest, code, error);

        return QHttpServerResponse(QJsonObject{{"status", "created"}, {"username", username}});
    });

    m_server.route(QStringLiteral("/api/users/<arg>"), QHttpServerRequest::Method::Put,
                   [this](const QString &username, const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty() || session.value(QStringLiteral("role")).toString() != QStringLiteral("admin"))
            return jsonError(Status::Forbidden, QStringLiteral("forbidden"), tr("Yêu cầu quyền admin"));
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        if (!ok)
            return jsonError(Status::BadRequest, QStringLiteral("invalid_json"), tr("JSON không hợp lệ"));

        const QString newUsername = body.value(QStringLiteral("username")).toString(username);
        const QString password = body.value(QStringLiteral("password")).toString();
        const QString role = body.value(QStringLiteral("role")).toString(QStringLiteral("user"));
        const bool enabled = body.value(QStringLiteral("enabled")).toBool(true);

        QString code, error;
        if (!m_database->updateUser(username, newUsername, password, role, enabled, &code, &error))
            return jsonError(Status::BadRequest, code, error);

        return QHttpServerResponse(QJsonObject{{"status", "updated"}, {"username", newUsername}});
    });

    m_server.route(QStringLiteral("/api/users/<arg>"), QHttpServerRequest::Method::Delete,
                   [this](const QString &username, const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty() || session.value(QStringLiteral("role")).toString() != QStringLiteral("admin"))
            return jsonError(Status::Forbidden, QStringLiteral("forbidden"), tr("Yêu cầu quyền admin"));

        QString code, error;
        if (!m_database->deleteUser(username, &code, &error))
            return jsonError(Status::BadRequest, code, error);

        return QHttpServerResponse(QJsonObject{{"status", "deleted"}, {"username", username}});
    });

    m_server.route(QStringLiteral("/api/alerts"), QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest &request) {
        if (!authorized(request))
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"), tr("Chưa đăng nhập"));
        QString error;
        const QJsonArray alerts = m_database->alerts(requestedLimit(request), &error);
        return QHttpServerResponse(QJsonObject{{"data", alerts}});
    });

    m_server.route(QStringLiteral("/api/readings/latest"), QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest &request) {
        if (!authorized(request))
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"), tr("Chưa đăng nhập"));
        QString error;
        const QJsonObject reading = m_database->latestReading(&error);
        return QHttpServerResponse(reading);
    });

    m_server.route(QStringLiteral("/api/config"), QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest &request) {
        if (!authorized(request))
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"), tr("Chưa đăng nhập"));
        QString error;
        return QHttpServerResponse(m_database->config(&error));
    });

    m_server.route(QStringLiteral("/api/config"), QHttpServerRequest::Method::Put,
                   [this](const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty() || session.value(QStringLiteral("role")).toString() != QStringLiteral("admin"))
            return jsonError(Status::Forbidden, QStringLiteral("forbidden"), tr("Yêu cầu quyền admin"));
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        if (!ok)
            return jsonError(Status::BadRequest, QStringLiteral("invalid_json"), tr("JSON không hợp lệ"));
        QString error;
        if (!m_database->updateConfig(body, &error))
            return jsonError(Status::InternalServerError, QStringLiteral("database_error"), error);
        return QHttpServerResponse(QJsonObject{{"status", "updated"}});
    });
}

bool ApiServer::authorized(const QHttpServerRequest &request) const
{
    return !sessionForRequest(request).isEmpty();
}

QJsonObject ApiServer::sessionForRequest(const QHttpServerRequest &request) const
{
    const auto headers = request.headers();
    for (const auto &pair : headers) {
        if (QString::fromUtf8(pair.first).compare(QStringLiteral("Authorization"), Qt::CaseInsensitive) == 0) {
            const QString val = QString::fromUtf8(pair.second);
            if (val.startsWith(QStringLiteral("Bearer "), Qt::CaseInsensitive)) {
                const QString token = val.mid(7).trimmed();
                return m_sessions.value(token);
            }
        }
    }
    return {};
}

QString ApiServer::createToken(const QString &username, const QString &role)
{
    const QString token = QUuid::createUuid().toString(QUuid::WithoutBraces)
        + QString::number(QRandomGenerator::global()->generate64(), 16);
    m_sessions.insert(token, QJsonObject{{"username", username},
                                         {"role", role},
                                         {"created_at", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}});
    return token;
}

int ApiServer::requestedLimit(const QHttpServerRequest &request)
{
    const QUrlQuery query(request.url().query());
    const int lim = query.queryItemValue(QStringLiteral("limit")).toInt();
    return lim > 0 ? lim : 50;
}
