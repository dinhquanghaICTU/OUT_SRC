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

}

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
                           {"service", "ictu-environment-server"},
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
        return QHttpServerResponse(QJsonObject{{"data", devices},
                                               {"count", devices.size()},
                                               {"online_window_seconds", 30}});
    });

    m_server.route(QStringLiteral("/api/devices/history"), QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        const QUrlQuery query(request.url());
        const QString deviceId = query.queryItemValue(QStringLiteral("device_id")).trimmed();
        const QString period = query.queryItemValue(QStringLiteral("period"));
        const QString selectedDate = query.queryItemValue(QStringLiteral("date"));
        bool limitOk = false;
        int limit = query.queryItemValue(QStringLiteral("limit")).toInt(&limitOk);
        if (!limitOk) limit = 500;
        const bool validPeriod = period == QStringLiteral("day")
                              || period == QStringLiteral("month")
                              || period == QStringLiteral("year");
        if (deviceId.isEmpty() || !validPeriod
            || !QDate::fromString(selectedDate, Qt::ISODate).isValid())
            return jsonError(Status::BadRequest, QStringLiteral("validation_error"),
                             tr("Thiết bị hoặc khoảng thời gian không hợp lệ"));
        QString error;
        const QJsonObject result = m_database->deviceTelemetryHistory(
            session.value(QStringLiteral("username")).toString(), deviceId,
            period, selectedDate, limit, &error);
        if (!error.isEmpty()) {
            const Status status = error == QStringLiteral("device_not_owned")
                                      ? Status::Forbidden : Status::InternalServerError;
            return jsonError(status, error == QStringLiteral("device_not_owned")
                                         ? error : QStringLiteral("database_error"), error);
        }
        return QHttpServerResponse(result);
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
        const QString deviceId = body.value(QStringLiteral("device_id")).toString().trimmed();
        QString name = body.value(QStringLiteral("name")).toString().trimmed();
        if (name.isEmpty())
            name = deviceId;
        static const QRegularExpression validDeviceId(QStringLiteral("^[A-Za-z0-9][A-Za-z0-9_-]{2,63}$"));
        if (!validDeviceId.match(deviceId).hasMatch() || name.size() > 100)
            return jsonError(Status::BadRequest, QStringLiteral("validation_error"),
                             tr("Device ID hoặc tên thiết bị không hợp lệ"));

        QString errorCode;
        QString error;
        if (!m_database->claimDevice(session.value(QStringLiteral("username")).toString(),
                                     deviceId, name, &errorCode, &error)) {
            if (errorCode == QStringLiteral("device_claimed"))
                return jsonError(Status::Conflict, errorCode, error);
            return jsonError(Status::InternalServerError, errorCode, error);
        }
        return QHttpServerResponse(
            QJsonObject{{"status", "claimed"}, {"device_id", deviceId}, {"name", name}},
            Status::Created);
    });

    m_server.route(QStringLiteral("/api/devices/relay"), QHttpServerRequest::Method::Post,
                   [this](const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        const QString deviceId = body.value(QStringLiteral("device_id")).toString().trimmed();
        if (!ok || deviceId.isEmpty() || !body.value(QStringLiteral("state")).isBool())
            return jsonError(Status::BadRequest, QStringLiteral("validation_error"),
                             tr("Device ID hoặc trạng thái relay không hợp lệ"));
        QString error;
        if (!m_database->userOwnsDevice(
                session.value(QStringLiteral("username")).toString(), deviceId, &error)) {
            if (!error.isEmpty())
                return jsonError(Status::InternalServerError,
                                 QStringLiteral("database_error"), error);
            return jsonError(Status::Forbidden, QStringLiteral("device_not_owned"),
                             tr("Bạn không sở hữu thiết bị này"));
        }
        const QString commandId = QStringLiteral("cmd-")
            + QUuid::createUuid().toString(QUuid::Id128);
        if (!m_mqtt->publishRelayCommand(
                deviceId, commandId, body.value(QStringLiteral("state")).toBool()))
            return jsonError(Status::ServiceUnavailable, QStringLiteral("mqtt_unavailable"),
                             tr("Server chưa kết nối MQTT broker"));
        return QHttpServerResponse(QJsonObject{{"status", "accepted"},
                                               {"command_id", commandId},
                                               {"device_id", deviceId}},
                                   Status::Accepted);
    });

    m_server.route(QStringLiteral("/api/devices/release"), QHttpServerRequest::Method::Post,
                   [this](const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        const QString deviceId = body.value(QStringLiteral("device_id")).toString().trimmed();
        if (!ok || deviceId.isEmpty())
            return jsonError(Status::BadRequest, QStringLiteral("validation_error"),
                             tr("Device ID không hợp lệ"));
        QString errorCode;
        QString error;
        if (!m_database->releaseDevice(
                session.value(QStringLiteral("username")).toString(), deviceId,
                &errorCode, &error)) {
            const Status status = errorCode == QStringLiteral("device_not_owned")
                                      ? Status::Forbidden : Status::InternalServerError;
            return jsonError(status, errorCode, error);
        }
        return QHttpServerResponse(QJsonObject{{"status", "released"},
                                               {"device_id", deviceId}});
    });

    m_server.route(QStringLiteral("/api/devices/config"), QHttpServerRequest::Method::Put,
                   [this](const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        const QString deviceId = body.value(QStringLiteral("device_id")).toString().trimmed();
        const QJsonObject config = body.value(QStringLiteral("config")).toObject();
        const int interval = config.value(QStringLiteral("sampling_interval_ms")).toInt(-1);
        const QJsonObject thresholds = config.value(QStringLiteral("thresholds")).toObject();
        bool thresholdsValid = !thresholds.isEmpty();
        for (auto sensor = thresholds.begin(); sensor != thresholds.end(); ++sensor) {
            const QJsonObject values = sensor.value().toObject();
            thresholdsValid = thresholdsValid && !values.isEmpty();
            for (auto value = values.begin(); value != values.end(); ++value)
                thresholdsValid = thresholdsValid && value.value().isDouble();
            if (values.contains(QStringLiteral("min")) && values.contains(QStringLiteral("max")))
                thresholdsValid = thresholdsValid
                    && values.value(QStringLiteral("min")).toDouble()
                       < values.value(QStringLiteral("max")).toDouble();
            if (values.contains(QStringLiteral("warning_above"))
                && values.contains(QStringLiteral("critical_above")))
                thresholdsValid = thresholdsValid
                    && values.value(QStringLiteral("warning_above")).toDouble()
                       < values.value(QStringLiteral("critical_above")).toDouble();
        }
        if (!ok || deviceId.isEmpty() || interval < 1000 || interval > 3600000
            || !thresholdsValid)
            return jsonError(Status::BadRequest, QStringLiteral("validation_error"),
                             tr("Cấu hình ngưỡng không hợp lệ"));

        QString errorCode;
        QString error;
        if (!m_database->updateDeviceConfig(
                session.value(QStringLiteral("username")).toString(), deviceId,
                config, &errorCode, &error)) {
            const Status status = errorCode == QStringLiteral("device_not_owned")
                                      ? Status::Forbidden : Status::InternalServerError;
            return jsonError(status, errorCode, error);
        }
        const bool published = m_mqtt->publishDeviceConfig(deviceId, config);
        return QHttpServerResponse(QJsonObject{{"status", "saved"},
                                               {"device_id", deviceId},
                                               {"mqtt_published", published}});
    });

    m_server.route(QStringLiteral("/api/admin/users"), QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        if (session.value(QStringLiteral("role")).toString() != QStringLiteral("admin"))
            return jsonError(Status::Forbidden, QStringLiteral("forbidden"),
                             tr("Chỉ admin được quản lý tài khoản"));
        QString error;
        const QJsonArray data = m_database->users(&error);
        if (!error.isEmpty())
            return jsonError(Status::InternalServerError, QStringLiteral("database_error"), error);
        return QHttpServerResponse(QJsonObject{{"data", data}, {"count", data.size()}});
    });

    m_server.route(QStringLiteral("/api/admin/users"), QHttpServerRequest::Method::Post,
                   [this](const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        if (session.value(QStringLiteral("role")).toString() != QStringLiteral("admin"))
            return jsonError(Status::Forbidden, QStringLiteral("forbidden"),
                             tr("Chỉ admin được thêm tài khoản"));
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        if (!ok)
            return jsonError(Status::BadRequest, QStringLiteral("invalid_json"),
                             tr("Nội dung JSON không hợp lệ"));
        const QString username = body.value(QStringLiteral("username")).toString().trimmed();
        const QString password = body.value(QStringLiteral("password")).toString();
        const QString role = body.value(QStringLiteral("role")).toString(QStringLiteral("viewer"));
        static const QRegularExpression validUsername(QStringLiteral("^[A-Za-z0-9_.-]{3,32}$"));
        if (!validUsername.match(username).hasMatch() || password.size() < 8
            || password.size() > 128
            || (role != QStringLiteral("viewer") && role != QStringLiteral("admin")))
            return jsonError(Status::BadRequest, QStringLiteral("validation_error"),
                             tr("Tài khoản, mật khẩu hoặc quyền không hợp lệ"));

        QString errorCode;
        QString error;
        if (!m_database->createUser(username, password, role, &errorCode, &error)) {
            if (errorCode == QStringLiteral("username_taken"))
                return jsonError(Status::Conflict, errorCode, error);
            return jsonError(Status::InternalServerError, errorCode, error);
        }
        return QHttpServerResponse(
            QJsonObject{{"status", "created"}, {"username", username}, {"role", role}},
            Status::Created);
    });

    m_server.route(QStringLiteral("/api/admin/users/<arg>"), QHttpServerRequest::Method::Put,
                   [this](const QString &oldUsername, const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        if (session.value(QStringLiteral("role")).toString() != QStringLiteral("admin"))
            return jsonError(Status::Forbidden, QStringLiteral("forbidden"),
                             tr("Chỉ admin được sửa tài khoản"));
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        if (!ok)
            return jsonError(Status::BadRequest, QStringLiteral("invalid_json"),
                             tr("Nội dung JSON không hợp lệ"));

        const QString username = body.value(QStringLiteral("username")).toString().trimmed();
        const QString password = body.value(QStringLiteral("password")).toString();
        const QString role = body.value(QStringLiteral("role")).toString(QStringLiteral("viewer"));
        const bool enabled = body.value(QStringLiteral("enabled")).toBool(true);
        static const QRegularExpression validUsername(QStringLiteral("^[A-Za-z0-9_.-]{3,32}$"));
        if (!validUsername.match(username).hasMatch()
            || (!password.isEmpty() && (password.size() < 8 || password.size() > 128))
            || (role != QStringLiteral("viewer") && role != QStringLiteral("admin")))
            return jsonError(Status::BadRequest, QStringLiteral("validation_error"),
                             tr("Tài khoản, mật khẩu hoặc quyền không hợp lệ"));

        QString errorCode;
        QString error;
        if (!m_database->updateUser(oldUsername, username, password, role, enabled,
                                    &errorCode, &error)) {
            if (errorCode == QStringLiteral("user_not_found"))
                return jsonError(Status::NotFound, errorCode, error);
            if (errorCode == QStringLiteral("username_taken"))
                return jsonError(Status::Conflict, errorCode, error);
            return jsonError(Status::InternalServerError, errorCode, error);
        }
        return QHttpServerResponse(QJsonObject{{"status", "updated"},
                                               {"username", username},
                                               {"role", role},
                                               {"enabled", enabled}});
    });

    m_server.route(QStringLiteral("/api/admin/users/<arg>"), QHttpServerRequest::Method::Delete,
                   [this](const QString &username, const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        if (session.value(QStringLiteral("role")).toString() != QStringLiteral("admin"))
            return jsonError(Status::Forbidden, QStringLiteral("forbidden"),
                             tr("Chỉ admin được xóa tài khoản"));
        if (session.value(QStringLiteral("username")).toString()
                .compare(username, Qt::CaseInsensitive) == 0)
            return jsonError(Status::BadRequest, QStringLiteral("cannot_delete_self"),
                             tr("Không thể xóa chính tài khoản đang đăng nhập"));

        QString errorCode;
        QString error;
        if (!m_database->deleteUser(username, &errorCode, &error)) {
            if (errorCode == QStringLiteral("user_not_found"))
                return jsonError(Status::NotFound, errorCode, error);
            return jsonError(Status::InternalServerError, errorCode, error);
        }
        return QHttpServerResponse(QJsonObject{{"status", "deleted"},
                                               {"username", username}});
    });

    m_server.route(QStringLiteral("/api/admin/users/<arg>/devices/<arg>"),
                   QHttpServerRequest::Method::Delete,
                   [this](const QString &username, const QString &deviceId,
                          const QHttpServerRequest &request) {
        const QJsonObject session = sessionForRequest(request);
        if (session.isEmpty())
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        if (session.value(QStringLiteral("role")).toString() != QStringLiteral("admin"))
            return jsonError(Status::Forbidden, QStringLiteral("forbidden"),
                             tr("Chỉ admin được gỡ thiết bị khỏi tài khoản"));

        QString errorCode;
        QString error;
        if (!m_database->releaseDevice(username, deviceId, &errorCode, &error)) {
            if (errorCode == QStringLiteral("device_not_owned"))
                return jsonError(Status::NotFound, errorCode, error);
            return jsonError(Status::InternalServerError, errorCode, error);
        }
        return QHttpServerResponse(QJsonObject{{"status", "released"},
                                               {"username", username},
                                               {"device_id", deviceId}});
    });

    m_server.route(QStringLiteral("/api/readings/latest"), QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest &request) {
        if (!authorized(request))
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        QString error;
        const QJsonObject result = m_database->latestReading(&error);
        if (result.isEmpty())
            return jsonError(Status::NotFound, QStringLiteral("reading_not_found"),
                             error.isEmpty() ? tr("Chưa có dữ liệu cảm biến") : error);
        return QHttpServerResponse(result);
    });

    m_server.route(QStringLiteral("/api/readings"), QHttpServerRequest::Method::Post,
                   [this](const QHttpServerRequest &request) {
        if (!authorized(request))
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        if (!ok)
            return jsonError(Status::BadRequest, QStringLiteral("invalid_json"),
                             tr("Nội dung JSON không hợp lệ"));
        const double pressure = body.value(QStringLiteral("pressure_hpa")).toDouble(-1);
        const double distance = body.value(QStringLiteral("distance_cm")).toDouble(-1);
        const double temperature = body.value(QStringLiteral("temperature_c")).toDouble(-999);
        const QString measuredAt = body.value(QStringLiteral("measured_at")).toString(
            QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        if (pressure <= 0 || distance < 0 || temperature < -100 || temperature > 150
            || !QDateTime::fromString(measuredAt, Qt::ISODate).isValid()) {
            return jsonError(Status::BadRequest, QStringLiteral("validation_error"),
                             tr("Giá trị cảm biến hoặc thời gian không hợp lệ"));
        }
        QString error;
        if (!m_database->insertReading(pressure, distance, temperature, measuredAt, &error))
            return jsonError(Status::InternalServerError, QStringLiteral("database_error"), error);
        return QHttpServerResponse(
            QJsonObject{{"status", "created"}, {"measured_at", measuredAt}}, Status::Created);
    });

    m_server.route(QStringLiteral("/api/pressure/history"), QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest &request) {
        if (!authorized(request))
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        QString error;
        const QJsonArray data = m_database->pressureHistory(requestedLimit(request), &error);
        if (!error.isEmpty())
            return jsonError(Status::InternalServerError, QStringLiteral("database_error"), error);
        return QHttpServerResponse(QJsonObject{{"data", data}, {"count", data.size()}});
    });

    m_server.route(QStringLiteral("/api/distance/history"), QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest &request) {
        if (!authorized(request))
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        QString error;
        const QJsonArray data = m_database->distanceHistory(requestedLimit(request), &error);
        if (!error.isEmpty())
            return jsonError(Status::InternalServerError, QStringLiteral("database_error"), error);
        return QHttpServerResponse(QJsonObject{{"data", data}, {"count", data.size()}});
    });

    m_server.route(QStringLiteral("/api/alerts"), QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest &request) {
        if (!authorized(request))
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        QString error;
        const QJsonArray data = m_database->alerts(requestedLimit(request), &error);
        if (!error.isEmpty())
            return jsonError(Status::InternalServerError, QStringLiteral("database_error"), error);
        return QHttpServerResponse(QJsonObject{{"data", data}, {"count", data.size()}});
    });

    m_server.route(QStringLiteral("/api/config"), QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest &request) {
        if (!authorized(request))
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        QString error;
        const QJsonObject data = m_database->config(&error);
        if (data.isEmpty())
            return jsonError(Status::InternalServerError, QStringLiteral("database_error"), error);
        return QHttpServerResponse(data);
    });

    m_server.route(QStringLiteral("/api/config"), QHttpServerRequest::Method::Put,
                   [this](const QHttpServerRequest &request) {
        if (!authorized(request))
            return jsonError(Status::Unauthorized, QStringLiteral("unauthorized"),
                             tr("Thiếu hoặc sai access token"));
        bool ok = false;
        const QJsonObject body = parseObject(request, &ok);
        if (!ok)
            return jsonError(Status::BadRequest, QStringLiteral("invalid_json"),
                             tr("Nội dung JSON không hợp lệ"));
        QString error;
        if (!m_database->updateConfig(body, &error))
            return jsonError(Status::BadRequest, QStringLiteral("validation_error"), error);
        return QHttpServerResponse(QJsonObject{{"status", "updated"}});
    });

    m_server.setMissingHandler([](const QHttpServerRequest &, QHttpServerResponder &&responder) {
        responder.write(QJsonDocument(
            QJsonObject{{"error", QJsonObject{{"code", "not_found"},
                                               {"message", "Endpoint không tồn tại"}}}}),
            QHttpServerResponder::StatusCode::NotFound);
    });
}

bool ApiServer::authorized(const QHttpServerRequest &request) const
{
    return !sessionForRequest(request).isEmpty();
}

QJsonObject ApiServer::sessionForRequest(const QHttpServerRequest &request) const
{
    const QByteArray header = request.value(QByteArrayLiteral("Authorization"));
    if (!header.startsWith("Bearer "))
        return {};
    const QString token = QString::fromUtf8(header.mid(7));
    const auto session = m_sessions.constFind(token);
    if (session == m_sessions.cend())
        return {};
    if (QDateTime::fromString(session->value(QStringLiteral("expires_at")).toString(), Qt::ISODate)
        <= QDateTime::currentDateTimeUtc())
        return {};
    return *session;
}

QString ApiServer::createToken(const QString &username, const QString &role)
{
    QByteArray bytes(32, Qt::Uninitialized);
    for (char &byte : bytes)
        byte = char(QRandomGenerator::system()->generate() & 0xff);
    const QString token = QString::fromLatin1(bytes.toHex());
    m_sessions.insert(token, QJsonObject{
        {"username", username}, {"role", role},
        {"expires_at", QDateTime::currentDateTimeUtc().addSecs(28800).toString(Qt::ISODateWithMs)}
    });
    return token;
}

int ApiServer::requestedLimit(const QHttpServerRequest &request)
{
    bool ok = false;
    const int value = request.query().queryItemValue(QStringLiteral("limit")).toInt(&ok);
    return ok ? qBound(1, value, 1000) : 100;
}
