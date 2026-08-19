#include "Database.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonValue>
#include <QRandomGenerator>
#include <QSqlError>
#include <QSqlQuery>
#include <QSet>
#include <QUuid>

namespace {

bool execSql(QSqlQuery &query, const QString &sql, QString *error)
{
    if (query.exec(sql))
        return true;
    if (error)
        *error = query.lastError().text();
    return false;
}

int safeLimit(int value)
{
    return qBound(1, value, 1000);
}

} // namespace

Database::Database(QString path)
    : m_path(std::move(path)),
      m_connectionName(QStringLiteral("server-db-") + QUuid::createUuid().toString(QUuid::Id128))
{
}

Database::~Database()
{
    if (m_db.isValid())
        m_db.close();
    m_db = {};
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool Database::open(QString *error)
{
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_db.setDatabaseName(m_path);
    if (!m_db.open()) {
        if (error)
            *error = m_db.lastError().text();
        return false;
    }

    QSqlQuery query(m_db);
    if (!execSql(query, QStringLiteral("PRAGMA foreign_keys = ON"), error)
        || !execSql(query, QStringLiteral("PRAGMA journal_mode = WAL"), error)
        || !execSql(query, QStringLiteral("PRAGMA busy_timeout = 5000"), error))
        return false;

    return migrate(error) && seedDefaults(error);
}

bool Database::migrate(QString *error)
{
    QSqlQuery query(m_db);
    const QStringList statements = {
        QStringLiteral("CREATE TABLE IF NOT EXISTS schema_migrations ("
                       "version INTEGER PRIMARY KEY, applied_at TEXT NOT NULL)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS users ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "username TEXT NOT NULL UNIQUE COLLATE NOCASE,"
                       "password_hash TEXT NOT NULL, password_salt TEXT NOT NULL,"
                       "role TEXT NOT NULL DEFAULT 'viewer', enabled INTEGER NOT NULL DEFAULT 1,"
                       "created_at TEXT NOT NULL)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS devices ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "device_id TEXT NOT NULL UNIQUE COLLATE NOCASE,"
                       "name TEXT NOT NULL,"
                       "owner_user_id INTEGER NOT NULL,"
                       "created_at TEXT NOT NULL,"
                       "FOREIGN KEY(owner_user_id) REFERENCES users(id) ON DELETE RESTRICT)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS discovered_devices ("
                       "device_id TEXT PRIMARY KEY COLLATE NOCASE,"
                       "online INTEGER NOT NULL DEFAULT 1,"
                       "device_type TEXT NOT NULL DEFAULT 'smart_door',"
                       "metrics_json TEXT NOT NULL DEFAULT '{}',"
                       "state_json TEXT NOT NULL DEFAULT '{}',"
                       "first_seen_at TEXT NOT NULL,"
                       "last_seen_at TEXT NOT NULL)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_devices_owner "
                       "ON devices(owner_user_id)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS per_device_config ("
                       "device_id TEXT PRIMARY KEY COLLATE NOCASE,"
                       "config_json TEXT NOT NULL, updated_at TEXT NOT NULL,"
                       "FOREIGN KEY(device_id) REFERENCES devices(device_id) ON DELETE CASCADE)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_discovered_last_seen "
                       "ON discovered_devices(last_seen_at DESC)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS device_telemetry_log ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "device_id TEXT NOT NULL COLLATE NOCASE,"
                       "metrics_json TEXT NOT NULL,"
                       "recorded_at TEXT NOT NULL)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_device_telemetry_log_lookup "
                       "ON device_telemetry_log(device_id, recorded_at DESC)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS alerts ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "source TEXT NOT NULL,"
                       "severity TEXT NOT NULL,"
                       "title TEXT NOT NULL,"
                       "message TEXT NOT NULL,"
                       "created_at TEXT NOT NULL)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS app_config ("
                       "id INTEGER PRIMARY KEY CHECK (id = 1),"
                       "motor_speed_steps INTEGER NOT NULL DEFAULT 800,"
                       "max_travel_steps INTEGER NOT NULL DEFAULT 3200,"
                       "auto_close_delay_s INTEGER NOT NULL DEFAULT 5,"
                       "microstepping INTEGER NOT NULL DEFAULT 8,"
                       "sampling_interval_s INTEGER NOT NULL DEFAULT 1,"
                       "updated_at TEXT NOT NULL)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS sensor_readings ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "door_position_pct REAL NOT NULL,"
                       "motion_detected INTEGER NOT NULL,"
                       "ir_blocked INTEGER NOT NULL,"
                       "motor_speed_rpm REAL NOT NULL,"
                       "passage_count INTEGER NOT NULL,"
                       "temperature_c REAL NOT NULL,"
                       "measured_at TEXT NOT NULL)")
    };

    for (const auto &stmt : statements) {
        if (!execSql(query, stmt, error))
            return false;
    }
    return true;
}

bool Database::seedDefaults(QString *error)
{
    QSqlQuery countQuery(m_db);
    if (!countQuery.exec(QStringLiteral("SELECT COUNT(*) FROM users"))) {
        if (error) *error = countQuery.lastError().text();
        return false;
    }
    if (countQuery.next() && countQuery.value(0).toInt() == 0) {
        QString code;
        if (!createUser(QStringLiteral("admin"), QStringLiteral("admin123"), QStringLiteral("admin"), &code, error))
            return false;
        if (!createUser(QStringLiteral("admin1"), QStringLiteral("1"), QStringLiteral("admin"), &code, error))
            return false;
        if (!createUser(QStringLiteral("operator"), QStringLiteral("operator123"), QStringLiteral("operator"), &code, error))
            return false;
    }

    QSqlQuery cfgQuery(m_db);
    if (!cfgQuery.exec(QStringLiteral("SELECT COUNT(*) FROM app_config"))) {
        if (error) *error = cfgQuery.lastError().text();
        return false;
    }
    if (cfgQuery.next() && cfgQuery.value(0).toInt() == 0) {
        QSqlQuery insertCfg(m_db);
        insertCfg.prepare(QStringLiteral(
            "INSERT INTO app_config (id, motor_speed_steps, max_travel_steps, auto_close_delay_s, microstepping, sampling_interval_s, updated_at) "
            "VALUES (1, 800, 3200, 5, 8, 1, :updated_at)"));
        insertCfg.bindValue(QStringLiteral(":updated_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        if (!insertCfg.exec()) {
            if (error) *error = insertCfg.lastError().text();
            return false;
        }
    }

    // Seed default target device discovery if empty
    QSqlQuery devQuery(m_db);
    devQuery.prepare(QStringLiteral(
        "INSERT OR IGNORE INTO discovered_devices (device_id, online, device_type, metrics_json, state_json, first_seen_at, last_seen_at) "
        "VALUES (:id, 1, 'smart_door', '{\"motion_detected\":false,\"ir_blocked\":false,\"door_position_pct\":0,\"motor_speed_rpm\":0,\"passage_count\":0,\"door_state\":\"CLOSED\"}', "
        "'{\"relay\":false,\"auto_mode\":true}', :now, :now)"));
    devQuery.bindValue(QStringLiteral(":id"), QStringLiteral("manhquang-190782"));
    const QString nowIso = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    devQuery.bindValue(QStringLiteral(":now"), nowIso);
    devQuery.exec();

    return true;
}

QByteArray Database::makeSalt()
{
    QByteArray salt;
    salt.resize(16);
    for (int i = 0; i < 16; ++i)
        salt[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    return salt.toHex();
}

QByteArray Database::hashPassword(const QString &password, const QByteArray &salt)
{
    return QCryptographicHash::hash(password.toUtf8() + salt, QCryptographicHash::Sha256).toHex();
}

bool Database::verifyUser(const QString &username, const QString &password, QString *role)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT password_hash, password_salt, role, enabled FROM users WHERE username = :u"));
    query.bindValue(QStringLiteral(":u"), username.trimmed());
    if (!query.exec() || !query.next())
        return false;

    if (query.value(3).toInt() != 1)
        return false;

    const QByteArray expectedHash = query.value(0).toByteArray();
    const QByteArray salt = query.value(1).toByteArray();
    const QByteArray computedHash = hashPassword(password, salt);

    if (expectedHash != computedHash)
        return false;

    if (role)
        *role = query.value(2).toString();
    return true;
}

bool Database::createUser(const QString &username, const QString &password, const QString &role,
                          QString *errorCode, QString *error)
{
    const QString trimmed = username.trimmed();
    if (trimmed.isEmpty() || password.isEmpty()) {
        if (errorCode) *errorCode = QStringLiteral("VALIDATION_ERROR");
        if (error) *error = QStringLiteral("Tài khoản và mật khẩu không được rỗng");
        return false;
    }

    const QByteArray salt = makeSalt();
    const QByteArray hash = hashPassword(password, salt);

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO users (username, password_hash, password_salt, role, enabled, created_at) "
        "VALUES (:u, :h, :s, :r, 1, :c)"));
    query.bindValue(QStringLiteral(":u"), trimmed);
    query.bindValue(QStringLiteral(":h"), QString::fromLatin1(hash));
    query.bindValue(QStringLiteral(":s"), QString::fromLatin1(salt));
    query.bindValue(QStringLiteral(":r"), role.isEmpty() ? QStringLiteral("user") : role);
    query.bindValue(QStringLiteral(":c"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    if (!query.exec()) {
        if (errorCode) *errorCode = QStringLiteral("SQL_ERROR");
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

bool Database::updateUser(const QString &oldUsername, const QString &newUsername,
                          const QString &password, const QString &role, bool enabled,
                          QString *errorCode, QString *error)
{
    QSqlQuery check(m_db);
    check.prepare(QStringLiteral("SELECT id FROM users WHERE username = :u"));
    check.bindValue(QStringLiteral(":u"), oldUsername.trimmed());
    if (!check.exec() || !check.next()) {
        if (errorCode) *errorCode = QStringLiteral("NOT_FOUND");
        if (error) *error = QStringLiteral("Không tìm thấy người dùng");
        return false;
    }

    QSqlQuery query(m_db);
    if (password.isEmpty()) {
        query.prepare(QStringLiteral("UPDATE users SET username = :nu, role = :r, enabled = :e WHERE username = :ou"));
        query.bindValue(QStringLiteral(":nu"), newUsername.trimmed());
        query.bindValue(QStringLiteral(":r"), role);
        query.bindValue(QStringLiteral(":e"), enabled ? 1 : 0);
        query.bindValue(QStringLiteral(":ou"), oldUsername.trimmed());
    } else {
        const QByteArray salt = makeSalt();
        const QByteArray hash = hashPassword(password, salt);
        query.prepare(QStringLiteral(
            "UPDATE users SET username = :nu, password_hash = :h, password_salt = :s, role = :r, enabled = :e WHERE username = :ou"));
        query.bindValue(QStringLiteral(":nu"), newUsername.trimmed());
        query.bindValue(QStringLiteral(":h"), QString::fromLatin1(hash));
        query.bindValue(QStringLiteral(":s"), QString::fromLatin1(salt));
        query.bindValue(QStringLiteral(":r"), role);
        query.bindValue(QStringLiteral(":e"), enabled ? 1 : 0);
        query.bindValue(QStringLiteral(":ou"), oldUsername.trimmed());
    }

    if (!query.exec()) {
        if (errorCode) *errorCode = QStringLiteral("SQL_ERROR");
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

bool Database::deleteUser(const QString &username, QString *errorCode, QString *error)
{
    if (username.compare(QStringLiteral("admin"), Qt::CaseInsensitive) == 0) {
        if (errorCode) *errorCode = QStringLiteral("FORBIDDEN");
        if (error) *error = QStringLiteral("Không thể xóa tài khoản admin gốc");
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("DELETE FROM users WHERE username = :u"));
    query.bindValue(QStringLiteral(":u"), username.trimmed());
    if (!query.exec()) {
        if (errorCode) *errorCode = QStringLiteral("SQL_ERROR");
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

QJsonArray Database::users(QString *error) const
{
    QJsonArray result;
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral(
            "SELECT u.id, u.username, u.role, u.enabled, u.created_at, d.device_id "
            "FROM users u LEFT JOIN devices d ON d.owner_user_id = u.id ORDER BY u.id ASC"))) {
        if (error) *error = query.lastError().text();
        return result;
    }

    while (query.next()) {
        QJsonObject obj;
        obj.insert(QStringLiteral("id"), query.value(0).toInt());
        obj.insert(QStringLiteral("username"), query.value(1).toString());
        obj.insert(QStringLiteral("role"), query.value(2).toString());
        obj.insert(QStringLiteral("enabled"), query.value(3).toInt() == 1);
        obj.insert(QStringLiteral("created_at"), query.value(4).toString());
        obj.insert(QStringLiteral("device_id"), query.value(5).toString());
        result.append(obj);
    }
    return result;
}

bool Database::claimDevice(const QString &username, const QString &deviceId, const QString &name,
                           QString *errorCode, QString *error)
{
    const QString targetId = QStringLiteral("manhquang-190782");
    if (deviceId.trimmed().compare(targetId, Qt::CaseInsensitive) != 0) {
        if (errorCode) *errorCode = QStringLiteral("INVALID_DEVICE_ID");
        if (error) *error = QStringLiteral("Chỉ được phép thêm thiết bị Mạnh Quang (ID: manhquang-190782) theo firmware!");
        return false;
    }

    QSqlQuery userQuery(m_db);
    userQuery.prepare(QStringLiteral("SELECT id FROM users WHERE username = :u"));
    userQuery.bindValue(QStringLiteral(":u"), username.trimmed());
    if (!userQuery.exec() || !userQuery.next()) {
        if (errorCode) *errorCode = QStringLiteral("USER_NOT_FOUND");
        if (error) *error = QStringLiteral("Người dùng không tồn tại");
        return false;
    }
    const int userId = userQuery.value(0).toInt();

    QSqlQuery claimQuery(m_db);
    claimQuery.prepare(QStringLiteral(
        "INSERT INTO devices (device_id, name, owner_user_id, created_at) "
        "VALUES (:did, :name, :uid, :now)"));
    claimQuery.bindValue(QStringLiteral(":did"), deviceId.trimmed());
    claimQuery.bindValue(QStringLiteral(":name"), name.trimmed().isEmpty() ? deviceId : name.trimmed());
    claimQuery.bindValue(QStringLiteral(":uid"), userId);
    claimQuery.bindValue(QStringLiteral(":now"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    if (!claimQuery.exec()) {
        if (errorCode) *errorCode = QStringLiteral("CLAIM_FAILED");
        if (error) *error = claimQuery.lastError().text();
        return false;
    }
    return true;
}

bool Database::releaseDevice(const QString &username, const QString &deviceId,
                             QString *errorCode, QString *error)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "DELETE FROM devices WHERE device_id = :did AND owner_user_id = ("
        "SELECT id FROM users WHERE username = :u)"));
    query.bindValue(QStringLiteral(":did"), deviceId.trimmed());
    query.bindValue(QStringLiteral(":u"), username.trimmed());

    if (!query.exec()) {
        if (errorCode) *errorCode = QStringLiteral("RELEASE_FAILED");
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

QJsonArray Database::devicesForUser(const QString &username, int onlineWindowSeconds,
                                    QString *error) const
{
    QJsonArray result;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT d.device_id, d.name, disc.online, disc.device_type, disc.metrics_json, disc.state_json, "
        "disc.last_seen_at, cfg.config_json "
        "FROM devices d "
        "JOIN users u ON d.owner_user_id = u.id "
        "LEFT JOIN discovered_devices disc ON disc.device_id = d.device_id "
        "LEFT JOIN per_device_config cfg ON cfg.device_id = d.device_id "
        "WHERE u.username = :u ORDER BY d.id ASC"));
    query.bindValue(QStringLiteral(":u"), username.trimmed());

    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return result;
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    while (query.next()) {
        QJsonObject obj;
        const QString did = query.value(0).toString();
        obj.insert(QStringLiteral("device_id"), did);
        obj.insert(QStringLiteral("name"), query.value(1).toString());

        const QString lastSeenStr = query.value(6).toString();
        bool isOnline = query.value(2).toInt() == 1;
        if (!lastSeenStr.isEmpty() && onlineWindowSeconds > 0) {
            const QDateTime lastSeen = QDateTime::fromString(lastSeenStr, Qt::ISODateWithMs);
            if (lastSeen.isValid() && lastSeen.secsTo(now) > onlineWindowSeconds)
                isOnline = false;
        }

        obj.insert(QStringLiteral("online"), isOnline);
        obj.insert(QStringLiteral("device_type"), query.value(3).toString().isEmpty() ? QStringLiteral("smart_door") : query.value(3).toString());
        obj.insert(QStringLiteral("metrics"), QJsonDocument::fromJson(query.value(4).toByteArray()).object());
        obj.insert(QStringLiteral("state"), QJsonDocument::fromJson(query.value(5).toByteArray()).object());
        obj.insert(QStringLiteral("last_seen_at"), lastSeenStr);
        obj.insert(QStringLiteral("config"), QJsonDocument::fromJson(query.value(7).toByteArray()).object());
        result.append(obj);
    }
    return result;
}

bool Database::recordDevicePresence(const QString &deviceId, bool online,
                                    const QJsonObject &metrics, QString *error)
{
    const QString nowIso = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    const QByteArray metricsJson = QJsonDocument(metrics).toJson(QJsonDocument::Compact);

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO discovered_devices (device_id, online, device_type, metrics_json, state_json, first_seen_at, last_seen_at) "
        "VALUES (:did, :ol, 'smart_door', :m, '{}', :now, :now) "
        "ON CONFLICT(device_id) DO UPDATE SET "
        "online = :ol, metrics_json = CASE WHEN length(:m) > 2 THEN :m ELSE metrics_json END, last_seen_at = :now"));
    query.bindValue(QStringLiteral(":did"), deviceId);
    query.bindValue(QStringLiteral(":ol"), online ? 1 : 0);
    query.bindValue(QStringLiteral(":m"), QString::fromUtf8(metricsJson));
    query.bindValue(QStringLiteral(":now"), nowIso);

    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

bool Database::recordTelemetry(const QString &deviceId, const QJsonObject &metrics,
                               const QString &recordedAt, QString *error)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO device_telemetry_log (device_id, metrics_json, recorded_at) "
        "VALUES (:did, :m, :rec)"));
    query.bindValue(QStringLiteral(":did"), deviceId);
    query.bindValue(QStringLiteral(":m"), QString::fromUtf8(QJsonDocument(metrics).toJson(QJsonDocument::Compact)));
    query.bindValue(QStringLiteral(":rec"), recordedAt.isEmpty() ? QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs) : recordedAt);

    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

QJsonObject Database::deviceTelemetryHistory(const QString &username, const QString &deviceId,
                                             const QString &period, const QString &selectedDate,
                                             int limit, QString *error) const
{
    Q_UNUSED(username);
    Q_UNUSED(period);
    Q_UNUSED(selectedDate);

    QJsonObject root;
    QJsonArray rows;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT metrics_json, recorded_at FROM device_telemetry_log "
        "WHERE device_id = :did ORDER BY recorded_at DESC LIMIT :lim"));
    query.bindValue(QStringLiteral(":did"), deviceId);
    query.bindValue(QStringLiteral(":lim"), safeLimit(limit > 0 ? limit : 60));

    if (query.exec()) {
        while (query.next()) {
            QJsonObject row = QJsonDocument::fromJson(query.value(0).toByteArray()).object();
            row.insert(QStringLiteral("recorded_at"), query.value(1).toString());
            rows.append(row);
        }
    } else if (error) {
        *error = query.lastError().text();
    }

    root.insert(QStringLiteral("device_id"), deviceId);
    root.insert(QStringLiteral("rows"), rows);
    return root;
}

bool Database::recordDeviceState(const QString &deviceId, const QJsonObject &state, QString *error)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "UPDATE discovered_devices SET state_json = :s, last_seen_at = :now WHERE device_id = :did"));
    query.bindValue(QStringLiteral(":s"), QString::fromUtf8(QJsonDocument(state).toJson(QJsonDocument::Compact)));
    query.bindValue(QStringLiteral(":now"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    query.bindValue(QStringLiteral(":did"), deviceId);

    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

bool Database::userOwnsDevice(const QString &username, const QString &deviceId, QString *error) const
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM devices d JOIN users u ON d.owner_user_id = u.id "
        "WHERE u.username = :u AND d.device_id = :did"));
    query.bindValue(QStringLiteral(":u"), username.trimmed());
    query.bindValue(QStringLiteral(":did"), deviceId.trimmed());

    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return false;
    }
    return query.next() && query.value(0).toInt() > 0;
}

bool Database::updateDeviceConfig(const QString &username, const QString &deviceId,
                                  const QJsonObject &config, QString *errorCode, QString *error)
{
    Q_UNUSED(username);
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO per_device_config (device_id, config_json, updated_at) "
        "VALUES (:did, :cfg, :now) "
        "ON CONFLICT(device_id) DO UPDATE SET config_json = :cfg, updated_at = :now"));
    query.bindValue(QStringLiteral(":did"), deviceId.trimmed());
    query.bindValue(QStringLiteral(":cfg"), QString::fromUtf8(QJsonDocument(config).toJson(QJsonDocument::Compact)));
    query.bindValue(QStringLiteral(":now"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    if (!query.exec()) {
        if (errorCode) *errorCode = QStringLiteral("SQL_ERROR");
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

QJsonArray Database::availableDevices(int onlineWindowSeconds, QString *error) const
{
    QJsonArray result;
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral(
            "SELECT disc.device_id, disc.online, disc.device_type, disc.metrics_json, disc.state_json, "
            "disc.last_seen_at FROM discovered_devices disc "
            "WHERE disc.device_id NOT IN (SELECT device_id FROM devices)"))) {
        if (error) *error = query.lastError().text();
        return result;
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    while (query.next()) {
        QJsonObject obj;
        obj.insert(QStringLiteral("device_id"), query.value(0).toString());
        bool isOnline = query.value(1).toInt() == 1;
        const QString lastSeenStr = query.value(5).toString();
        if (!lastSeenStr.isEmpty() && onlineWindowSeconds > 0) {
            const QDateTime lastSeen = QDateTime::fromString(lastSeenStr, Qt::ISODateWithMs);
            if (lastSeen.isValid() && lastSeen.secsTo(now) > onlineWindowSeconds)
                isOnline = false;
        }
        obj.insert(QStringLiteral("online"), isOnline);
        obj.insert(QStringLiteral("device_type"), query.value(2).toString());
        obj.insert(QStringLiteral("metrics"), QJsonDocument::fromJson(query.value(3).toByteArray()).object());
        obj.insert(QStringLiteral("state"), QJsonDocument::fromJson(query.value(4).toByteArray()).object());
        obj.insert(QStringLiteral("last_seen_at"), lastSeenStr);
        result.append(obj);
    }
    return result;
}

bool Database::insertReading(double doorPositionPct, bool motionDetected, bool irBlocked,
                             double motorSpeedRpm, int passageCount, double temperatureC,
                             const QString &measuredAt, QString *error)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO sensor_readings (door_position_pct, motion_detected, ir_blocked, motor_speed_rpm, passage_count, temperature_c, measured_at) "
        "VALUES (:p, :m, :ir, :rpm, :cnt, :t, :dt)"));
    query.bindValue(QStringLiteral(":p"), doorPositionPct);
    query.bindValue(QStringLiteral(":m"), motionDetected ? 1 : 0);
    query.bindValue(QStringLiteral(":ir"), irBlocked ? 1 : 0);
    query.bindValue(QStringLiteral(":rpm"), motorSpeedRpm);
    query.bindValue(QStringLiteral(":cnt"), passageCount);
    query.bindValue(QStringLiteral(":t"), temperatureC);
    query.bindValue(QStringLiteral(":dt"), measuredAt.isEmpty() ? QDateTime::currentDateTime().toString(Qt::ISODateWithMs) : measuredAt);

    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

QJsonObject Database::latestReading(QString *error) const
{
    QJsonObject obj;
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral(
            "SELECT door_position_pct, motion_detected, ir_blocked, motor_speed_rpm, passage_count, temperature_c, measured_at "
            "FROM sensor_readings ORDER BY id DESC LIMIT 1"))) {
        if (error) *error = query.lastError().text();
        return obj;
    }
    if (query.next()) {
        obj.insert(QStringLiteral("door_position_pct"), query.value(0).toDouble());
        obj.insert(QStringLiteral("motion_detected"), query.value(1).toInt() == 1);
        obj.insert(QStringLiteral("ir_blocked"), query.value(2).toInt() == 1);
        obj.insert(QStringLiteral("motor_speed_rpm"), query.value(3).toDouble());
        obj.insert(QStringLiteral("passage_count"), query.value(4).toInt());
        obj.insert(QStringLiteral("temperature_c"), query.value(5).toDouble());
        obj.insert(QStringLiteral("measured_at"), query.value(6).toString());
    }
    return obj;
}

QJsonArray Database::alerts(int limit, QString *error) const
{
    QJsonArray result;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT id, source, severity, title, message, created_at FROM alerts ORDER BY id DESC LIMIT :lim"));
    query.bindValue(QStringLiteral(":lim"), safeLimit(limit > 0 ? limit : 50));

    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return result;
    }
    while (query.next()) {
        QJsonObject obj;
        obj.insert(QStringLiteral("id"), query.value(0).toInt());
        obj.insert(QStringLiteral("source"), query.value(1).toString());
        obj.insert(QStringLiteral("severity"), query.value(2).toString());
        obj.insert(QStringLiteral("title"), query.value(3).toString());
        obj.insert(QStringLiteral("message"), query.value(4).toString());
        obj.insert(QStringLiteral("created_at"), query.value(5).toString());
        result.append(obj);
    }
    return result;
}

bool Database::addAlert(const QString &source, const QString &severity, const QString &title,
                        const QString &message, QString *error)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO alerts (source, severity, title, message, created_at) "
        "VALUES (:src, :sev, :title, :msg, :dt)"));
    query.bindValue(QStringLiteral(":src"), source);
    query.bindValue(QStringLiteral(":sev"), severity);
    query.bindValue(QStringLiteral(":title"), title);
    query.bindValue(QStringLiteral(":msg"), message);
    query.bindValue(QStringLiteral(":dt"), QDateTime::currentDateTime().toString(QStringLiteral("dd/MM HH:mm:ss")));

    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

QJsonObject Database::config(QString *error) const
{
    QJsonObject obj;
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral("SELECT motor_speed_steps, max_travel_steps, auto_close_delay_s, microstepping, sampling_interval_s FROM app_config WHERE id = 1"))) {
        if (error) *error = query.lastError().text();
        return obj;
    }
    if (query.next()) {
        obj.insert(QStringLiteral("motor_speed_steps"), query.value(0).toInt());
        obj.insert(QStringLiteral("max_travel_steps"), query.value(1).toInt());
        obj.insert(QStringLiteral("auto_close_delay_s"), query.value(2).toInt());
        obj.insert(QStringLiteral("microstepping"), query.value(3).toInt());
        obj.insert(QStringLiteral("sampling_interval_s"), query.value(4).toInt());
    }
    return obj;
}

bool Database::updateConfig(const QJsonObject &config, QString *error)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "UPDATE app_config SET motor_speed_steps = :s, max_travel_steps = :m, auto_close_delay_s = :d, "
        "microstepping = :ms, sampling_interval_s = :si, updated_at = :now WHERE id = 1"));
    query.bindValue(QStringLiteral(":s"), config.value(QStringLiteral("motor_speed_steps")).toInt(800));
    query.bindValue(QStringLiteral(":m"), config.value(QStringLiteral("max_travel_steps")).toInt(3200));
    query.bindValue(QStringLiteral(":d"), config.value(QStringLiteral("auto_close_delay_s")).toInt(5));
    query.bindValue(QStringLiteral(":ms"), config.value(QStringLiteral("microstepping")).toInt(8));
    query.bindValue(QStringLiteral(":si"), config.value(QStringLiteral("sampling_interval_s")).toInt(1));
    query.bindValue(QStringLiteral(":now"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

bool Database::isOpen() const
{
    return m_db.isOpen();
}
