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

}

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
                       "device_type TEXT NOT NULL DEFAULT 'generic',"
                       "metrics_json TEXT NOT NULL DEFAULT '{}',"
                       "state_json TEXT NOT NULL DEFAULT '{}',"
                       "first_seen_at TEXT NOT NULL,"
                       "last_seen_at TEXT NOT NULL)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_devices_owner "
                       "ON devices(owner_user_id)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS per_device_config ("
                       "device_id TEXT PRIMARY KEY COLLATE NOCASE,"
                       "config_json TEXT NOT NULL,updated_at TEXT NOT NULL,"
                       "FOREIGN KEY(device_id) REFERENCES devices(device_id) ON DELETE CASCADE)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_discovered_last_seen "
                       "ON discovered_devices(last_seen_at DESC)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS device_telemetry_log ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "device_id TEXT NOT NULL COLLATE NOCASE,"
                       "recorded_at TEXT NOT NULL,metrics_json TEXT NOT NULL)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_telemetry_device_time "
                       "ON device_telemetry_log(device_id,recorded_at DESC)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS pressure_log ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "measured_at TEXT NOT NULL, pressure_hpa REAL NOT NULL,"
                       "temperature_c REAL NOT NULL)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS distance_log ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "measured_at TEXT NOT NULL, distance_cm REAL NOT NULL)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS alert_log ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "created_at TEXT NOT NULL, type TEXT NOT NULL,"
                       "message TEXT NOT NULL, value REAL NOT NULL)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS device_config ("
                       "id INTEGER PRIMARY KEY CHECK(id = 1),"
                       "pressure_min_hpa REAL NOT NULL,"
                       "pressure_max_hpa REAL NOT NULL,"
                       "distance_min_cm REAL NOT NULL,"
                       "sampling_interval_seconds INTEGER NOT NULL,"
                       "updated_at TEXT NOT NULL)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_pressure_time "
                       "ON pressure_log(measured_at DESC)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_distance_time "
                       "ON distance_log(measured_at DESC)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_alert_time "
                       "ON alert_log(created_at DESC)")
    };
    if (!m_db.transaction()) {
        if (error)
            *error = m_db.lastError().text();
        return false;
    }
    for (const QString &statement : statements) {
        if (!execSql(query, statement, error)) {
            m_db.rollback();
            return false;
        }
    }

    // Migration from the first ownership model, which allowed one device per user.
    QSqlQuery schema(m_db);
    if (!schema.exec(QStringLiteral(
            "SELECT sql FROM sqlite_master WHERE type='table' AND name='devices'"))
        || !schema.next()) {
        if (error) *error = schema.lastError().text();
        m_db.rollback();
        return false;
    }
    const bool requiresOwnershipMigration = schema.value(0).toString().contains(
        QStringLiteral("owner_user_id INTEGER NOT NULL UNIQUE"), Qt::CaseInsensitive);
    schema.finish();
    if (requiresOwnershipMigration) {
        const QStringList migration = {
            QStringLiteral("ALTER TABLE devices RENAME TO devices_one_owner"),
            QStringLiteral("CREATE TABLE devices ("
                           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                           "device_id TEXT NOT NULL UNIQUE COLLATE NOCASE,"
                           "name TEXT NOT NULL,owner_user_id INTEGER NOT NULL,"
                           "created_at TEXT NOT NULL,"
                           "FOREIGN KEY(owner_user_id) REFERENCES users(id) ON DELETE RESTRICT)"),
            QStringLiteral("INSERT INTO devices(id,device_id,name,owner_user_id,created_at) "
                           "SELECT id,device_id,name,owner_user_id,created_at FROM devices_one_owner"),
            QStringLiteral("DROP TABLE devices_one_owner"),
            QStringLiteral("CREATE INDEX idx_devices_owner ON devices(owner_user_id)")
        };
        for (const QString &statement : migration) {
            if (!execSql(query, statement, error)) {
                m_db.rollback();
                return false;
            }
        }
    }

    // Add discovery profile columns to databases created by older builds.
    QSet<QString> discoveryColumns;
    QSqlQuery columns(m_db);
    if (!columns.exec(QStringLiteral("PRAGMA table_info(discovered_devices)"))) {
        if (error) *error = columns.lastError().text();
        m_db.rollback();
        return false;
    }
    while (columns.next())
        discoveryColumns.insert(columns.value(1).toString());
    columns.finish();
    if (!discoveryColumns.contains(QStringLiteral("device_type"))
        && !execSql(query, QStringLiteral(
            "ALTER TABLE discovered_devices ADD COLUMN device_type TEXT NOT NULL DEFAULT 'generic'"), error)) {
        m_db.rollback();
        return false;
    }
    if (!discoveryColumns.contains(QStringLiteral("metrics_json"))
        && !execSql(query, QStringLiteral(
            "ALTER TABLE discovered_devices ADD COLUMN metrics_json TEXT NOT NULL DEFAULT '{}'"), error)) {
        m_db.rollback();
        return false;
    }
    if (!discoveryColumns.contains(QStringLiteral("state_json"))
        && !execSql(query, QStringLiteral(
            "ALTER TABLE discovered_devices ADD COLUMN state_json TEXT NOT NULL DEFAULT '{}'"), error)) {
        m_db.rollback();
        return false;
    }
    if (!m_db.commit()) {
        if (error)
            *error = m_db.lastError().text();
        return false;
    }
    return true;
}

bool Database::seedDefaults(QString *error)
{
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    QSqlQuery configQuery(m_db);
    configQuery.prepare(QStringLiteral(
        "INSERT OR IGNORE INTO device_config "
        "(id, pressure_min_hpa, pressure_max_hpa, distance_min_cm, "
        "sampling_interval_seconds, updated_at) VALUES (1, 990, 1030, 20, 5, ?)"));
    configQuery.addBindValue(now);
    if (!configQuery.exec()) {
        if (error)
            *error = configQuery.lastError().text();
        return false;
    }

    QSqlQuery count(m_db);
    if (!count.exec(QStringLiteral("SELECT COUNT(*) FROM users")) || !count.next()) {
        if (error)
            *error = count.lastError().text();
        return false;
    }

    // Clean up any foreign device records not belonging to THEANH
    QSqlQuery cleanup(m_db);
    cleanup.exec(QStringLiteral("DELETE FROM discovered_devices WHERE device_id NOT IN ('Theanh-190782', 'THEANH-ESP32-LAB01', 'THEANH-SMART-02', '190782')"));
    cleanup.exec(QStringLiteral("DELETE FROM devices WHERE device_id NOT IN ('Theanh-190782', 'THEANH-ESP32-LAB01', 'THEANH-SMART-02', '190782')"));
    cleanup.exec(QStringLiteral("DELETE FROM device_telemetry_log WHERE device_id NOT IN ('Theanh-190782', 'THEANH-ESP32-LAB01', 'THEANH-SMART-02', '190782')"));

    // Seed default users if not already present
    if (count.value(0).toInt() == 0) {
        struct UserSeed { const char *user; const char *pass; const char *role; };
        const UserSeed initialUsers[] = {
            {"admin", "1", "admin"},
            {"theanh", "123456", "admin"},
            {"kythuatvien", "123456", "viewer"},
            {"operator01", "123456", "viewer"}
        };
        for (const auto &u : initialUsers) {
            const QByteArray salt = makeSalt();
            QSqlQuery user(m_db);
            user.prepare(QStringLiteral(
                "INSERT INTO users(username,password_hash,password_salt,role,created_at) "
                "VALUES(?,?,?,?,?)"));
            user.addBindValue(QString::fromLatin1(u.user));
            user.addBindValue(QString::fromLatin1(hashPassword(QString::fromLatin1(u.pass), salt).toHex()));
            user.addBindValue(QString::fromLatin1(salt.toHex()));
            user.addBindValue(QString::fromLatin1(u.role));
            user.addBindValue(now);
            user.exec();
        }
    }

    // 1. Seed Discovered Online Devices
    QSqlQuery seedDiscovered(m_db);
    seedDiscovered.prepare(QStringLiteral(
        "INSERT INTO discovered_devices(device_id,online,device_type,metrics_json,state_json,first_seen_at,last_seen_at) "
        "VALUES(?,?,?,?,?,?,?) ON CONFLICT(device_id) DO UPDATE SET "
        "online=excluded.online,last_seen_at=excluded.last_seen_at,"
        "metrics_json=excluded.metrics_json,state_json=excluded.state_json"));

    // Device 1: Theanh-190782 (Main AC Power Monitor)
    seedDiscovered.addBindValue(QStringLiteral("Theanh-190782"));
    seedDiscovered.addBindValue(1);
    seedDiscovered.addBindValue(QStringLiteral("power_monitor"));
    seedDiscovered.addBindValue(QStringLiteral("{\"voltage_v\":221.8,\"current_a\":2.35,\"power_w\":521.23,\"frequency_hz\":50.02,\"power_factor\":0.98,\"relay_on\":true}"));
    seedDiscovered.addBindValue(QStringLiteral("{\"relay\":true,\"load_mode\":\"auto\",\"uptime\":86400}"));
    seedDiscovered.addBindValue(now);
    seedDiscovered.addBindValue(now);
    seedDiscovered.exec();

    // Device 2: THEANH-ESP32-LAB01
    seedDiscovered.addBindValue(QStringLiteral("THEANH-ESP32-LAB01"));
    seedDiscovered.addBindValue(1);
    seedDiscovered.addBindValue(QStringLiteral("power_monitor"));
    seedDiscovered.addBindValue(QStringLiteral("{\"voltage_v\":220.4,\"current_a\":1.15,\"power_w\":253.46,\"frequency_hz\":50.00,\"power_factor\":0.97,\"relay_on\":true}"));
    seedDiscovered.addBindValue(QStringLiteral("{\"relay\":true,\"load_mode\":\"manual\",\"uptime\":43200}"));
    seedDiscovered.addBindValue(now);
    seedDiscovered.addBindValue(now);
    seedDiscovered.exec();

    // Device 3: THEANH-SMART-02
    seedDiscovered.addBindValue(QStringLiteral("THEANH-SMART-02"));
    seedDiscovered.addBindValue(1);
    seedDiscovered.addBindValue(QStringLiteral("power_monitor"));
    seedDiscovered.addBindValue(QStringLiteral("{\"voltage_v\":222.1,\"current_a\":3.82,\"power_w\":848.42,\"frequency_hz\":50.01,\"power_factor\":0.99,\"relay_on\":false}"));
    seedDiscovered.addBindValue(QStringLiteral("{\"relay\":false,\"load_mode\":\"standby\",\"uptime\":129600}"));
    seedDiscovered.addBindValue(now);
    seedDiscovered.addBindValue(now);
    seedDiscovered.exec();

    // 2. Auto-claim primary device for Admin user
    QSqlQuery getAdminId(m_db);
    if (getAdminId.exec(QStringLiteral("SELECT id FROM users WHERE username='admin'")) && getAdminId.next()) {
        const int adminId = getAdminId.value(0).toInt();
        QSqlQuery claim(m_db);
        claim.prepare(QStringLiteral(
            "INSERT OR IGNORE INTO devices(device_id,name,owner_user_id,created_at) "
            "VALUES('Theanh-190782','Bộ Đo AC RMS & Công Suất Tải THEANH',?,?)"));
        claim.addBindValue(adminId);
        claim.addBindValue(now);
        claim.exec();
    }

    // 3. Seed Realistic Historical Telemetry Logs (last 24 hours) if table has few records
    QSqlQuery checkLogs(m_db);
    if (checkLogs.exec(QStringLiteral("SELECT COUNT(*) FROM device_telemetry_log WHERE device_id='Theanh-190782'")) && checkLogs.next()) {
        if (checkLogs.value(0).toInt() < 20) {
            const QDateTime baseTime = QDateTime::currentDateTimeUtc();
            for (int i = 60; i >= 0; --i) {
                const QDateTime recTime = baseTime.addSecs(-i * 300); // 5 min interval
                const double v = 220.0 + 2.2 * qSin(i / 6.0) + 0.6 * qCos(i / 2.0);
                const double a = 2.10 + 0.85 * qCos(i / 5.0) + 0.25 * qSin(i / 3.0);
                const double w = v * a * 0.98;
                const QString tStr = recTime.toString(Qt::ISODateWithMs);

                QSqlQuery insLog(m_db);
                insLog.prepare(QStringLiteral(
                    "INSERT INTO device_telemetry_log(device_id,recorded_at,metrics_json) VALUES(?,?,?)"));
                insLog.addBindValue(QStringLiteral("Theanh-190782"));
                insLog.addBindValue(tStr);
                insLog.addBindValue(QStringLiteral(
                    "{\"voltage_v\":%1,\"current_a\":%2,\"power_w\":%3,\"frequency_hz\":50.02,\"power_factor\":0.98,\"relay_on\":true}")
                    .arg(QString::number(v, 'f', 2), QString::number(a, 'f', 3), QString::number(w, 'f', 2)));
                insLog.exec();

                // Also seed standard readings table for backward compatibility
                QSqlQuery insRead(m_db);
                insRead.prepare(QStringLiteral(
                    "INSERT INTO pressure_log(measured_at,pressure_hpa,temperature_c) VALUES(?,?,?)"));
                insRead.addBindValue(tStr);
                insRead.addBindValue(v);
                insRead.addBindValue(w);
                insRead.exec();

                QSqlQuery insDist(m_db);
                insDist.prepare(QStringLiteral(
                    "INSERT INTO distance_log(measured_at,distance_cm) VALUES(?,?)"));
                insDist.addBindValue(tStr);
                insDist.addBindValue(a);
                insDist.exec();
            }
        }
    }

    // 4. Seed Alerts
    QSqlQuery checkAlerts(m_db);
    if (checkAlerts.exec(QStringLiteral("SELECT COUNT(*) FROM alert_log")) && checkAlerts.next()) {
        if (checkAlerts.value(0).toInt() == 0) {
            const QString alertTime1 = QDateTime::currentDateTimeUtc().addSecs(-1800).toString(Qt::ISODateWithMs);
            const QString alertTime2 = QDateTime::currentDateTimeUtc().addSecs(-3600).toString(Qt::ISODateWithMs);
            const QString alertTime3 = QDateTime::currentDateTimeUtc().addSecs(-7200).toString(Qt::ISODateWithMs);

            QSqlQuery insAlert(m_db);
            insAlert.exec(QStringLiteral("INSERT INTO alert_log(created_at,type,message,value) VALUES('%1','info','Khởi động hệ thống giám sát tải THEANH',221.8)").arg(alertTime3));
            insAlert.exec(QStringLiteral("INSERT INTO alert_log(created_at,type,message,value) VALUES('%1','info','Đóng Rơ-le tải chính thành công (Đang cấp nguồn)',521.2)").arg(alertTime2));
            insAlert.exec(QStringLiteral("INSERT INTO alert_log(created_at,type,message,value) VALUES('%1','warning','Điện áp lưới ổn định ở mức 221.8V (50.02Hz)',221.8)").arg(alertTime1));
        }
    }

    return true;
}

QByteArray Database::makeSalt()
{
    QByteArray salt(32, Qt::Uninitialized);
    for (char &byte : salt)
        byte = char(QRandomGenerator::system()->generate() & 0xff);
    return salt;
}

QByteArray Database::hashPassword(const QString &password, const QByteArray &salt)
{
    QByteArray value = salt + password.toUtf8();
    for (int i = 0; i < 120000; ++i)
        value = QCryptographicHash::hash(value + salt, QCryptographicHash::Sha256);
    return value;
}

bool Database::verifyUser(const QString &username, const QString &password, QString *role)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT password_hash,password_salt,role FROM users "
        "WHERE username = ? AND enabled = 1"));
    query.addBindValue(username.trimmed());
    if (!query.exec() || !query.next())
        return false;
    const QByteArray salt = QByteArray::fromHex(query.value(1).toByteArray());
    const QByteArray expected = QByteArray::fromHex(query.value(0).toByteArray());
    const QByteArray actual = hashPassword(password, salt);
    if (expected != actual)
        return false;
    if (role)
        *role = query.value(2).toString();
    return true;
}

bool Database::createUser(const QString &username, const QString &password, const QString &role,
                          QString *errorCode, QString *error)
{
    const QString normalizedUsername = username.trimmed();
    if (errorCode)
        errorCode->clear();

    QSqlQuery exists(m_db);
    exists.prepare(QStringLiteral("SELECT 1 FROM users WHERE username = ? COLLATE NOCASE"));
    exists.addBindValue(normalizedUsername);
    if (!exists.exec()) {
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = exists.lastError().text();
        return false;
    }
    if (exists.next()) {
        if (errorCode) *errorCode = QStringLiteral("username_taken");
        if (error) *error = QStringLiteral("Tài khoản đã tồn tại");
        return false;
    }

    const QByteArray salt = makeSalt();
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO users(username,password_hash,password_salt,role,created_at) "
        "VALUES(?,?,?,?,?)"));
    query.addBindValue(normalizedUsername);
    query.addBindValue(QString::fromLatin1(hashPassword(password, salt).toHex()));
    query.addBindValue(QString::fromLatin1(salt.toHex()));
    query.addBindValue(role);
    query.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    if (!query.exec()) {
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

bool Database::updateUser(const QString &oldUsername, const QString &newUsername,
                          const QString &password, const QString &role, bool enabled,
                          QString *errorCode, QString *error)
{
    const QString current = oldUsername.trimmed();
    const QString next = newUsername.trimmed();
    if (errorCode)
        errorCode->clear();
    if (!m_db.transaction()) {
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = m_db.lastError().text();
        return false;
    }

    QSqlQuery find(m_db);
    find.prepare(QStringLiteral("SELECT id FROM users WHERE username=? COLLATE NOCASE"));
    find.addBindValue(current);
    if (!find.exec() || !find.next()) {
        m_db.rollback();
        if (errorCode) *errorCode = QStringLiteral("user_not_found");
        if (error) *error = QStringLiteral("Tài khoản không tồn tại");
        return false;
    }
    const int userId = find.value(0).toInt();

    if (current.compare(next, Qt::CaseInsensitive) != 0) {
        QSqlQuery exists(m_db);
        exists.prepare(QStringLiteral("SELECT 1 FROM users WHERE username=? COLLATE NOCASE"));
        exists.addBindValue(next);
        if (!exists.exec()) {
            m_db.rollback();
            if (errorCode) *errorCode = QStringLiteral("database_error");
            if (error) *error = exists.lastError().text();
            return false;
        }
        if (exists.next()) {
            m_db.rollback();
            if (errorCode) *errorCode = QStringLiteral("username_taken");
            if (error) *error = QStringLiteral("Tài khoản đã tồn tại");
            return false;
        }
    }

    QSqlQuery update(m_db);
    if (password.isEmpty()) {
        update.prepare(QStringLiteral(
            "UPDATE users SET username=?,role=?,enabled=? WHERE id=?"));
        update.addBindValue(next);
        update.addBindValue(role);
        update.addBindValue(enabled ? 1 : 0);
        update.addBindValue(userId);
    } else {
        const QByteArray salt = makeSalt();
        update.prepare(QStringLiteral(
            "UPDATE users SET username=?,password_hash=?,password_salt=?,role=?,enabled=? "
            "WHERE id=?"));
        update.addBindValue(next);
        update.addBindValue(QString::fromLatin1(hashPassword(password, salt).toHex()));
        update.addBindValue(QString::fromLatin1(salt.toHex()));
        update.addBindValue(role);
        update.addBindValue(enabled ? 1 : 0);
        update.addBindValue(userId);
    }
    if (!update.exec() || !m_db.commit()) {
        const QString databaseMessage = update.lastError().text().isEmpty()
                                            ? m_db.lastError().text() : update.lastError().text();
        m_db.rollback();
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = databaseMessage;
        return false;
    }
    return true;
}

bool Database::deleteUser(const QString &username, QString *errorCode, QString *error)
{
    const QString normalizedUsername = username.trimmed();
    if (errorCode)
        errorCode->clear();
    if (!m_db.transaction()) {
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = m_db.lastError().text();
        return false;
    }

    QSqlQuery find(m_db);
    find.prepare(QStringLiteral("SELECT id FROM users WHERE username=? COLLATE NOCASE"));
    find.addBindValue(normalizedUsername);
    if (!find.exec() || !find.next()) {
        m_db.rollback();
        if (errorCode) *errorCode = QStringLiteral("user_not_found");
        if (error) *error = QStringLiteral("Tài khoản không tồn tại");
        return false;
    }
    const int userId = find.value(0).toInt();

    QSqlQuery config(m_db);
    config.prepare(QStringLiteral(
        "DELETE FROM per_device_config WHERE device_id IN "
        "(SELECT device_id FROM devices WHERE owner_user_id=?)"));
    config.addBindValue(userId);
    if (!config.exec()) {
        m_db.rollback();
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = config.lastError().text();
        return false;
    }

    QSqlQuery devices(m_db);
    devices.prepare(QStringLiteral("DELETE FROM devices WHERE owner_user_id=?"));
    devices.addBindValue(userId);
    if (!devices.exec()) {
        m_db.rollback();
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = devices.lastError().text();
        return false;
    }

    QSqlQuery remove(m_db);
    remove.prepare(QStringLiteral("DELETE FROM users WHERE id=?"));
    remove.addBindValue(userId);
    if (!remove.exec() || remove.numRowsAffected() != 1 || !m_db.commit()) {
        const QString databaseMessage = remove.lastError().text().isEmpty()
                                            ? m_db.lastError().text() : remove.lastError().text();
        m_db.rollback();
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = databaseMessage;
        return false;
    }
    return true;
}

QJsonArray Database::users(QString *error) const
{
    QJsonArray result;
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral(
            "SELECT u.id,u.username,u.role,u.enabled,u.created_at,"
            "GROUP_CONCAT(d.device_id, ',') "
            "FROM users u LEFT JOIN devices d ON d.owner_user_id=u.id "
            "GROUP BY u.id,u.username,u.role,u.enabled,u.created_at ORDER BY u.id"))) {
        if (error) *error = query.lastError().text();
        return result;
    }
    while (query.next()) {
        QJsonObject user{{"id", query.value(0).toInt()},
                         {"username", query.value(1).toString()},
                         {"role", query.value(2).toString()},
                         {"enabled", query.value(3).toBool()},
                         {"created_at", query.value(4).toString()}};
        QJsonArray deviceIds;
        if (!query.value(5).isNull()) {
            const QStringList ids = query.value(5).toString().split(',', Qt::SkipEmptyParts);
            for (const QString &id : ids)
                deviceIds.append(id);
        }
        user.insert(QStringLiteral("device_ids"), deviceIds);
        user.insert(QStringLiteral("device_count"), deviceIds.size());
        result.append(user);
    }
    return result;
}

bool Database::claimDevice(const QString &username, const QString &deviceId, const QString &name,
                           QString *errorCode, QString *error)
{
    if (errorCode)
        errorCode->clear();
    if (!m_db.transaction()) {
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = m_db.lastError().text();
        return false;
    }

    QSqlQuery user(m_db);
    user.prepare(QStringLiteral("SELECT id FROM users WHERE username=? COLLATE NOCASE AND enabled=1"));
    user.addBindValue(username.trimmed());
    if (!user.exec() || !user.next()) {
        m_db.rollback();
        if (errorCode) *errorCode = QStringLiteral("user_not_found");
        if (error) *error = QStringLiteral("Tài khoản không tồn tại hoặc đã bị khóa");
        return false;
    }
    const int userId = user.value(0).toInt();

    QSqlQuery claimed(m_db);
    claimed.prepare(QStringLiteral("SELECT 1 FROM devices WHERE device_id=? COLLATE NOCASE"));
    claimed.addBindValue(deviceId.trimmed());
    if (!claimed.exec()) {
        m_db.rollback();
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = claimed.lastError().text();
        return false;
    }
    if (claimed.next()) {
        m_db.rollback();
        if (errorCode) *errorCode = QStringLiteral("device_claimed");
        if (error) *error = QStringLiteral("Thiết bị đã thuộc tài khoản khác");
        return false;
    }

    QSqlQuery insert(m_db);
    insert.prepare(QStringLiteral(
        "INSERT INTO devices(device_id,name,owner_user_id,created_at) VALUES(?,?,?,?)"));
    insert.addBindValue(deviceId.trimmed());
    insert.addBindValue(name.trimmed());
    insert.addBindValue(userId);
    insert.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    if (!insert.exec() || !m_db.commit()) {
        const QString databaseMessage = insert.lastError().text().isEmpty()
                                            ? m_db.lastError().text() : insert.lastError().text();
        m_db.rollback();
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = databaseMessage;
        return false;
    }
    return true;
}

bool Database::releaseDevice(const QString &username, const QString &deviceId,
                             QString *errorCode, QString *error)
{
    if (errorCode)
        errorCode->clear();
    if (!m_db.transaction()) {
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = m_db.lastError().text();
        return false;
    }
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "DELETE FROM devices WHERE device_id=? COLLATE NOCASE AND owner_user_id=("
        "SELECT id FROM users WHERE username=? COLLATE NOCASE)"));
    query.addBindValue(deviceId.trimmed());
    query.addBindValue(username.trimmed());
    if (!query.exec()) {
        m_db.rollback();
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = query.lastError().text();
        return false;
    }
    if (query.numRowsAffected() != 1) {
        m_db.rollback();
        if (errorCode) *errorCode = QStringLiteral("device_not_owned");
        if (error) *error = QStringLiteral("Thiết bị không thuộc tài khoản của bạn");
        return false;
    }
    QSqlQuery config(m_db);
    config.prepare(QStringLiteral("DELETE FROM per_device_config WHERE device_id=? COLLATE NOCASE"));
    config.addBindValue(deviceId.trimmed());
    if (!config.exec() || !m_db.commit()) {
        const QString databaseMessage = config.lastError().text().isEmpty()
                                            ? m_db.lastError().text() : config.lastError().text();
        m_db.rollback();
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = databaseMessage;
        return false;
    }
    return true;
}

QJsonArray Database::devicesForUser(const QString &username, int onlineWindowSeconds,
                                    QString *error) const
{
    QJsonArray result;
    const QString cutoff = QDateTime::currentDateTimeUtc()
                               .addSecs(-qBound(5, onlineWindowSeconds, 300))
                               .toString(Qt::ISODateWithMs);
    const bool isAdmin = (username.trimmed().compare(QStringLiteral("admin"), Qt::CaseInsensitive) == 0);
    QSqlQuery query(m_db);
    if (isAdmin) {
        query.prepare(QStringLiteral(
            "SELECT d.device_id,d.name,d.created_at,p.online,p.last_seen_at,"
            "p.device_type,p.metrics_json,p.state_json,c.config_json,u.username "
            "FROM devices d JOIN users u ON u.id=d.owner_user_id "
            "LEFT JOIN discovered_devices p ON p.device_id=d.device_id COLLATE NOCASE "
            "LEFT JOIN per_device_config c ON c.device_id=d.device_id COLLATE NOCASE "
            "ORDER BY d.created_at DESC"));
    } else {
        query.prepare(QStringLiteral(
            "SELECT d.device_id,d.name,d.created_at,p.online,p.last_seen_at,"
            "p.device_type,p.metrics_json,p.state_json,c.config_json,u.username "
            "FROM devices d JOIN users u ON u.id=d.owner_user_id "
            "LEFT JOIN discovered_devices p ON p.device_id=d.device_id COLLATE NOCASE "
            "LEFT JOIN per_device_config c ON c.device_id=d.device_id COLLATE NOCASE "
            "WHERE u.username=? COLLATE NOCASE "
            "ORDER BY d.created_at DESC"));
        query.addBindValue(username.trimmed());
    }
    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return result;
    }
    while (query.next()) {
        const bool online = query.value(3).toBool()
            && query.value(4).toString() >= cutoff;
        const QJsonObject metrics = QJsonDocument::fromJson(
            query.value(6).toByteArray()).object();
        const QJsonObject state = QJsonDocument::fromJson(
            query.value(7).toByteArray()).object();
        const QJsonObject config = QJsonDocument::fromJson(
            query.value(8).toByteArray()).object();
        const QString addedBy = query.value(9).toString();
        QJsonArray capabilities;
        if (state.contains(QStringLiteral("relay"))
            || query.value(5).toString() == QStringLiteral("temperature_sound")
            || query.value(5).toString() == QStringLiteral("pump_distance")
            || query.value(5).toString() == QStringLiteral("water_flow_pump")
            || metrics.contains(QStringLiteral("pump_on")))
            capabilities.append(QStringLiteral("relay"));
        result.append(QJsonObject{{"device_id", query.value(0).toString()},
                                  {"name", query.value(1).toString()},
                                  {"created_at", query.value(2).toString()},
                                  {"online", online},
                                  {"last_seen_at", query.value(4).toString()},
                                  {"device_type", query.value(5).toString()},
                                  {"metrics", metrics},
                                  {"state", state},
                                  {"config", config},
                                  {"capabilities", capabilities},
                                  {"added_by", addedBy},
                                  {"owner", addedBy}});
    }
    return result;
}

bool Database::recordDevicePresence(const QString &deviceId, bool online,
                                    const QJsonObject &metrics, QString *error)
{
    const QString normalizedId = deviceId.trimmed();
    if (normalizedId.isEmpty() || normalizedId.size() > 64)
        return false;
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    QString deviceType = QStringLiteral("power_monitor");
    if (metrics.contains(QStringLiteral("voltage_v")) || metrics.contains(QStringLiteral("current_a")) || metrics.contains(QStringLiteral("power_w")))
        deviceType = QStringLiteral("power_monitor");
    else if (metrics.contains(QStringLiteral("sound_vpp")))
        deviceType = QStringLiteral("temperature_sound");
    else if (metrics.contains(QStringLiteral("weather_pressure")) || metrics.contains(QStringLiteral("pressure_hpa")))
        deviceType = QStringLiteral("uv_pressure");
    else if (metrics.contains(QStringLiteral("pump_on")) || metrics.contains(QStringLiteral("distance_cm")))
        deviceType = QStringLiteral("pump_distance");
    const QString metricsJson = QString::fromUtf8(
        QJsonDocument(metrics).toJson(QJsonDocument::Compact));
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO discovered_devices(device_id,online,device_type,metrics_json,first_seen_at,last_seen_at) "
        "VALUES(?,?,?,?,?,?) ON CONFLICT(device_id) DO UPDATE SET "
        "online=excluded.online,last_seen_at=excluded.last_seen_at,"
        "device_type=CASE WHEN excluded.metrics_json='{}' THEN discovered_devices.device_type "
        "ELSE excluded.device_type END,"
        "metrics_json=CASE WHEN excluded.metrics_json='{}' THEN discovered_devices.metrics_json "
        "ELSE excluded.metrics_json END"));
    query.addBindValue(normalizedId);
    query.addBindValue(online ? 1 : 0);
    query.addBindValue(deviceType);
    query.addBindValue(metricsJson);
    query.addBindValue(now);
    query.addBindValue(now);
    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

bool Database::recordTelemetry(const QString &deviceId, const QJsonObject &metrics,
                               const QString &recordedAt, QString *error)
{
    const QString normalizedId = deviceId.trimmed();
    if (normalizedId.isEmpty() || metrics.isEmpty())
        return false;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO device_telemetry_log(device_id,recorded_at,metrics_json) VALUES(?,?,?)"));
    query.addBindValue(normalizedId);
    query.addBindValue(recordedAt);
    query.addBindValue(QString::fromUtf8(QJsonDocument(metrics).toJson(QJsonDocument::Compact)));
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
    QString ownershipError;
    const bool isAdmin = (username.trimmed().compare(QStringLiteral("admin"), Qt::CaseInsensitive) == 0);
    if (!isAdmin && !userOwnsDevice(username, deviceId, &ownershipError)) {
        if (error) *error = ownershipError.isEmpty()
                                ? QStringLiteral("device_not_owned") : ownershipError;
        return {};
    }

    QString ownerUsername = QStringLiteral("Chưa gán");
    QString deviceName = deviceId;
    QString deviceCreatedAt = QStringLiteral("--");
    QSqlQuery devInfo(m_db);
    devInfo.prepare(QStringLiteral(
        "SELECT d.name, u.username, d.created_at FROM devices d JOIN users u ON u.id=d.owner_user_id WHERE d.device_id=? COLLATE NOCASE"));
    devInfo.addBindValue(deviceId.trimmed());
    if (devInfo.exec() && devInfo.next()) {
        deviceName = devInfo.value(0).toString();
        ownerUsername = devInfo.value(1).toString();
        deviceCreatedAt = devInfo.value(2).toString();
    }

    const int prefixLength = period == QStringLiteral("year") ? 4
                           : period == QStringLiteral("month") ? 7 : 10;
    const QString prefix = selectedDate.left(prefixLength);
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT recorded_at,metrics_json FROM device_telemetry_log "
        "WHERE device_id=? COLLATE NOCASE AND substr(recorded_at,1,?)=? "
        "ORDER BY recorded_at DESC LIMIT ?"));
    query.addBindValue(deviceId.trimmed());
    query.addBindValue(prefixLength);
    query.addBindValue(prefix);
    query.addBindValue(safeLimit(limit));
    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return {};
    }
    QJsonArray rows;
    QSet<QString> metricKeys;
    while (query.next()) {
        const QJsonObject metrics = QJsonDocument::fromJson(query.value(1).toByteArray()).object();
        for (auto it = metrics.begin(); it != metrics.end(); ++it)
            if (it.value().isDouble()) metricKeys.insert(it.key());
        rows.append(QJsonObject{{"recorded_at", query.value(0).toString()},
                                {"metrics", metrics}});
    }

    QHash<QString, double> sums;
    QHash<QString, int> counts;
    QSqlQuery aggregate(m_db);
    aggregate.prepare(QStringLiteral(
        "SELECT metrics_json FROM device_telemetry_log "
        "WHERE device_id=? COLLATE NOCASE AND substr(recorded_at,1,?)=?"));
    aggregate.addBindValue(deviceId.trimmed());
    aggregate.addBindValue(prefixLength);
    aggregate.addBindValue(prefix);
    if (!aggregate.exec()) {
        if (error) *error = aggregate.lastError().text();
        return {};
    }
    int total = 0;
    while (aggregate.next()) {
        ++total;
        const QJsonObject metrics = QJsonDocument::fromJson(aggregate.value(0).toByteArray()).object();
        for (auto it = metrics.begin(); it != metrics.end(); ++it) {
            if (!it.value().isDouble()) continue;
            sums[it.key()] += it.value().toDouble();
            counts[it.key()] += 1;
            metricKeys.insert(it.key());
        }
    }
    QStringList sortedKeys(metricKeys.begin(), metricKeys.end());
    sortedKeys.sort();
    QJsonArray keys;
    QJsonObject averages;
    for (const QString &key : sortedKeys) {
        keys.append(key);
        if (counts.value(key) > 0)
            averages.insert(key, sums.value(key) / counts.value(key));
    }
    return QJsonObject{{"device_id", deviceId},
                       {"device_name", deviceName},
                       {"added_by", ownerUsername},
                       {"added_at", deviceCreatedAt},
                       {"period", period},
                       {"selected_date", selectedDate}, {"total", total},
                       {"metric_keys", keys}, {"averages", averages}, {"data", rows}};
}

bool Database::recordDeviceState(const QString &deviceId, const QJsonObject &state, QString *error)
{
    if (deviceId.trimmed().isEmpty() || state.isEmpty())
        return false;
    const QString stateJson = QString::fromUtf8(
        QJsonDocument(state).toJson(QJsonDocument::Compact));
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO discovered_devices(device_id,online,state_json,first_seen_at,last_seen_at) "
        "VALUES(?,1,?,?,?) ON CONFLICT(device_id) DO UPDATE SET "
        "online=1,state_json=excluded.state_json,last_seen_at=excluded.last_seen_at"));
    query.addBindValue(deviceId.trimmed());
    query.addBindValue(stateJson);
    query.addBindValue(now);
    query.addBindValue(now);
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
        "SELECT 1 FROM devices d JOIN users u ON u.id=d.owner_user_id "
        "WHERE u.username=? COLLATE NOCASE AND d.device_id=? COLLATE NOCASE"));
    query.addBindValue(username.trimmed());
    query.addBindValue(deviceId.trimmed());
    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return false;
    }
    return query.next();
}

bool Database::updateDeviceConfig(const QString &username, const QString &deviceId,
                                  const QJsonObject &config, QString *errorCode, QString *error)
{
    if (errorCode) errorCode->clear();
    QString ownershipError;
    if (!userOwnsDevice(username, deviceId, &ownershipError)) {
        if (!ownershipError.isEmpty()) {
            if (errorCode) *errorCode = QStringLiteral("database_error");
            if (error) *error = ownershipError;
        } else {
            if (errorCode) *errorCode = QStringLiteral("device_not_owned");
            if (error) *error = QStringLiteral("Bạn không sở hữu thiết bị này");
        }
        return false;
    }
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO per_device_config(device_id,config_json,updated_at) VALUES(?,?,?) "
        "ON CONFLICT(device_id) DO UPDATE SET config_json=excluded.config_json,"
        "updated_at=excluded.updated_at"));
    query.addBindValue(deviceId.trimmed());
    query.addBindValue(QString::fromUtf8(QJsonDocument(config).toJson(QJsonDocument::Compact)));
    query.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    if (!query.exec()) {
        if (errorCode) *errorCode = QStringLiteral("database_error");
        if (error) *error = query.lastError().text();
        return false;
    }
    return true;
}

QJsonArray Database::availableDevices(int onlineWindowSeconds, QString *error) const
{
    QJsonArray result;
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QString cutoff = now.addSecs(-qBound(5, onlineWindowSeconds, 300))
                               .toString(Qt::ISODateWithMs);
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT d.device_id,d.last_seen_at,d.device_type,d.metrics_json "
        "FROM discovered_devices d "
        "WHERE d.online=1 AND d.last_seen_at>=? "
        "AND NOT EXISTS(SELECT 1 FROM devices c WHERE c.device_id=d.device_id COLLATE NOCASE) "
        "ORDER BY d.last_seen_at DESC"));
    query.addBindValue(cutoff);
    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return result;
    }
    while (query.next()) {
        const QDateTime lastSeen = QDateTime::fromString(query.value(1).toString(), Qt::ISODate);
        result.append(QJsonObject{{"device_id", query.value(0).toString()},
                                  {"online", true},
                                  {"last_seen_at", query.value(1).toString()},
                                  {"last_seen_seconds", lastSeen.secsTo(now)},
                                  {"device_type", query.value(2).toString()},
                                  {"metrics", QJsonDocument::fromJson(
                                       query.value(3).toByteArray()).object()}});
    }
    return result;
}

bool Database::insertReading(double pressure, double distance, double temperature,
                             const QString &measuredAt, QString *error)
{
    const QJsonObject cfg = config(error);
    if (cfg.isEmpty())
        return false;
    if (!m_db.transaction()) {
        if (error)
            *error = m_db.lastError().text();
        return false;
    }
    QSqlQuery p(m_db);
    p.prepare(QStringLiteral(
        "INSERT INTO pressure_log(measured_at,pressure_hpa,temperature_c) VALUES(?,?,?)"));
    p.addBindValue(measuredAt); p.addBindValue(pressure); p.addBindValue(temperature);
    QSqlQuery d(m_db);
    d.prepare(QStringLiteral(
        "INSERT INTO distance_log(measured_at,distance_cm) VALUES(?,?)"));
    d.addBindValue(measuredAt); d.addBindValue(distance);
    if (!p.exec() || !d.exec()) {
        if (error)
            *error = !p.lastError().text().isEmpty() ? p.lastError().text() : d.lastError().text();
        m_db.rollback();
        return false;
    }
    auto insertAlert = [&](const QString &type, const QString &message, double value) {
        QSqlQuery a(m_db);
        a.prepare(QStringLiteral(
            "INSERT INTO alert_log(created_at,type,message,value) VALUES(?,?,?,?)"));
        a.addBindValue(measuredAt); a.addBindValue(type); a.addBindValue(message); a.addBindValue(value);
        return a.exec();
    };
    const double minP = cfg.value(QStringLiteral("pressure_min_hpa")).toDouble();
    const double maxP = cfg.value(QStringLiteral("pressure_max_hpa")).toDouble();
    const double minD = cfg.value(QStringLiteral("distance_min_cm")).toDouble();
    bool alertsOk = true;
    if (pressure < minP || pressure > maxP)
        alertsOk = insertAlert(QStringLiteral("pressure"), QStringLiteral("Áp suất vượt ngưỡng"), pressure);
    if (distance < minD)
        alertsOk = alertsOk && insertAlert(QStringLiteral("distance"), QStringLiteral("Khoảng cách dưới ngưỡng"), distance);
    if (!alertsOk || !m_db.commit()) {
        if (error)
            *error = m_db.lastError().text();
        m_db.rollback();
        return false;
    }
    return true;
}

QJsonObject Database::latestReading(QString *error) const
{
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral(
            "SELECT p.pressure_hpa,p.temperature_c,d.distance_cm,p.measured_at "
            "FROM pressure_log p JOIN distance_log d ON d.measured_at=p.measured_at "
            "ORDER BY p.measured_at DESC LIMIT 1")) || !query.next()) {
        if (error)
            *error = query.lastError().text();
        return {};
    }
    return {{QStringLiteral("pressure_hpa"), query.value(0).toDouble()},
            {QStringLiteral("temperature_c"), query.value(1).toDouble()},
            {QStringLiteral("distance_cm"), query.value(2).toDouble()},
            {QStringLiteral("measured_at"), query.value(3).toString()}};
}

QJsonArray Database::pressureHistory(int limit, QString *error) const
{
    QJsonArray result;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT measured_at,pressure_hpa,temperature_c FROM pressure_log "
        "ORDER BY measured_at DESC LIMIT ?"));
    query.addBindValue(safeLimit(limit));
    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return result;
    }
    while (query.next())
        result.append(QJsonObject{{"measured_at",query.value(0).toString()},
                                  {"pressure_hpa",query.value(1).toDouble()},
                                  {"temperature_c",query.value(2).toDouble()}});
    return result;
}

QJsonArray Database::distanceHistory(int limit, QString *error) const
{
    QJsonArray result;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT measured_at,distance_cm FROM distance_log ORDER BY measured_at DESC LIMIT ?"));
    query.addBindValue(safeLimit(limit));
    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return result;
    }
    while (query.next())
        result.append(QJsonObject{{"measured_at",query.value(0).toString()},
                                  {"distance_cm",query.value(1).toDouble()}});
    return result;
}

QJsonArray Database::alerts(int limit, QString *error) const
{
    QJsonArray result;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT id,created_at,type,message,value FROM alert_log "
        "ORDER BY created_at DESC LIMIT ?"));
    query.addBindValue(safeLimit(limit));
    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return result;
    }
    while (query.next())
        result.append(QJsonObject{{"id",query.value(0).toInt()},
                                  {"created_at",query.value(1).toString()},
                                  {"type",query.value(2).toString()},
                                  {"message",query.value(3).toString()},
                                  {"value",query.value(4).toDouble()}});
    return result;
}

QJsonObject Database::config(QString *error) const
{
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral(
            "SELECT pressure_min_hpa,pressure_max_hpa,distance_min_cm,"
            "sampling_interval_seconds,updated_at FROM device_config WHERE id=1"))
        || !query.next()) {
        if (error) *error = query.lastError().text();
        return {};
    }
    return {{"pressure_min_hpa",query.value(0).toDouble()},
            {"pressure_max_hpa",query.value(1).toDouble()},
            {"distance_min_cm",query.value(2).toDouble()},
            {"sampling_interval_seconds",query.value(3).toInt()},
            {"updated_at",query.value(4).toString()}};
}

bool Database::updateConfig(const QJsonObject &cfg, QString *error)
{
    const double minP = cfg.value(QStringLiteral("pressure_min_hpa")).toDouble(-1);
    const double maxP = cfg.value(QStringLiteral("pressure_max_hpa")).toDouble(-1);
    const double minD = cfg.value(QStringLiteral("distance_min_cm")).toDouble(-1);
    const int interval = cfg.value(QStringLiteral("sampling_interval_seconds")).toInt(-1);
    if (minP <= 0 || maxP <= minP || minD < 0 || interval < 1 || interval > 3600) {
        if (error) *error = QStringLiteral("Cấu hình không hợp lệ");
        return false;
    }
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "UPDATE device_config SET pressure_min_hpa=?,pressure_max_hpa=?,"
        "distance_min_cm=?,sampling_interval_seconds=?,updated_at=? WHERE id=1"));
    query.addBindValue(minP); query.addBindValue(maxP); query.addBindValue(minD);
    query.addBindValue(interval);
    query.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
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
