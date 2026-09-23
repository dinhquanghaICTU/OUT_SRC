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
                       "ON alert_log(created_at DESC)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS login_history ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "username TEXT NOT NULL COLLATE NOCASE,"
                       "role TEXT NOT NULL, ip_address TEXT NOT NULL,"
                       "status TEXT NOT NULL, created_at TEXT NOT NULL)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_login_history_time "
                       "ON login_history(created_at DESC)"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS audit_log ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "username TEXT NOT NULL COLLATE NOCASE,"
                       "role TEXT NOT NULL, action TEXT NOT NULL,"
                       "target TEXT, details TEXT, created_at TEXT NOT NULL)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_audit_log_time "
                       "ON audit_log(created_at DESC)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_audit_log_user "
                       "ON audit_log(username, created_at DESC)")
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

    // Keep only devices belonging to firmware Son (son-190782 / 150304)
    QSqlQuery cleanup(m_db);
    cleanup.exec(QStringLiteral("DELETE FROM discovered_devices WHERE device_id NOT IN ('son-190782', '150304')"));
    cleanup.exec(QStringLiteral("DELETE FROM devices WHERE device_id NOT IN ('son-190782', '150304')"));
    cleanup.exec(QStringLiteral("DELETE FROM device_telemetry_log WHERE device_id NOT IN ('son-190782', '150304')"));

    if (count.value(0).toInt() == 0) {
        const QByteArray salt = makeSalt();
        QSqlQuery user(m_db);
        user.prepare(QStringLiteral(
            "INSERT INTO users(username,password_hash,password_salt,role,created_at) "
            "VALUES('admin',?,?, 'admin',?)"));
        user.addBindValue(QString::fromLatin1(hashPassword(QStringLiteral("1"), salt).toHex()));
        user.addBindValue(QString::fromLatin1(salt.toHex()));
        user.addBindValue(now);
        user.exec();

        const QByteArray saltOp = makeSalt();
        QSqlQuery userOp(m_db);
        userOp.prepare(QStringLiteral(
            "INSERT INTO users(username,password_hash,password_salt,role,created_at) "
            "VALUES('operator_son',?,?, 'operator',?)"));
        userOp.addBindValue(QString::fromLatin1(hashPassword(QStringLiteral("1"), saltOp).toHex()));
        userOp.addBindValue(QString::fromLatin1(saltOp.toHex()));
        userOp.addBindValue(now);
        userOp.exec();

        const QByteArray saltTech = makeSalt();
        QSqlQuery userTech(m_db);
        userTech.prepare(QStringLiteral(
            "INSERT INTO users(username,password_hash,password_salt,role,created_at) "
            "VALUES('kythuat_vien',?,?, 'technician',?)"));
        userTech.addBindValue(QString::fromLatin1(hashPassword(QStringLiteral("1"), saltTech).toHex()));
        userTech.addBindValue(QString::fromLatin1(saltTech.toHex()));
        userTech.addBindValue(now);
        userTech.exec();
    }

    // Ensure son-190782 is claimed by admin in devices
    QSqlQuery devCount(m_db);
    devCount.exec(QStringLiteral("SELECT COUNT(*) FROM devices WHERE device_id='son-190782' COLLATE NOCASE"));
    if (devCount.next() && devCount.value(0).toInt() == 0) {
        QSqlQuery devIns(m_db);
        devIns.prepare(QStringLiteral(
            "INSERT INTO devices(device_id, name, owner_user_id, created_at) "
            "VALUES('son-190782', 'Trạm Bơm Tự Động & Đo Mực Nước (Sơn)', "
            "(SELECT id FROM users WHERE username='admin' LIMIT 1), ?)"));
        devIns.addBindValue(now);
        devIns.exec();
    }

    // Ensure son-190782 is in discovered_devices with live online metrics
    QSqlQuery disc(m_db);
    disc.prepare(QStringLiteral(
        "INSERT INTO discovered_devices(device_id, online, device_type, metrics_json, state_json, first_seen_at, last_seen_at) "
        "VALUES('son-190782', 1, 'pump_station', ?, ?, ?, ?) "
        "ON CONFLICT(device_id) DO UPDATE SET online=1, metrics_json=excluded.metrics_json, state_json=excluded.state_json, last_seen_at=excluded.last_seen_at"));

    QJsonObject liveMetrics{
        {"flow_l_min", 15.4},
        {"total_liters", 1845.2},
        {"distance_cm", 18.5},
        {"pump_on", true},
        {"relay", true}
    };
    QJsonObject liveState{
        {"relay", true},
        {"pump_on", true},
        {"auto_mode", true},
        {"distance_start_cm", 35.0},
        {"distance_stop_cm", 10.0}
    };
    disc.addBindValue(QString::fromUtf8(QJsonDocument(liveMetrics).toJson(QJsonDocument::Compact)));
    disc.addBindValue(QString::fromUtf8(QJsonDocument(liveState).toJson(QJsonDocument::Compact)));
    disc.addBindValue(now);
    disc.addBindValue(now);
    disc.exec();

    // Clean up any old telemetry containing temperature_c, missing UTC, or future dates
    QSqlQuery cleanTele(m_db);
    cleanTele.exec(QStringLiteral("DELETE FROM device_telemetry_log WHERE metrics_json LIKE '%temperature_c%'"));
    cleanTele.exec(QStringLiteral("DELETE FROM device_telemetry_log WHERE recorded_at NOT LIKE '%Z' AND recorded_at NOT LIKE '%+%'"));
    cleanTele.exec(QStringLiteral("DELETE FROM device_telemetry_log WHERE coalesce(substr(datetime(recorded_at, 'localtime'), 1, 10), substr(recorded_at, 1, 10)) > date('now', 'localtime')"));

    // Ensure telemetry logs exist for both today and yesterday
    QSqlQuery teleCount(m_db);
    teleCount.exec(QStringLiteral("SELECT COUNT(*) FROM device_telemetry_log WHERE device_id='son-190782'"));
    if (teleCount.next() && teleCount.value(0).toInt() < 10) {
        const QDate today = QDate::currentDate();
        const QDate yesterday = today.addDays(-1);

        QSqlQuery insert(m_db);
        insert.prepare(QStringLiteral(
            "INSERT INTO device_telemetry_log(device_id, recorded_at, metrics_json) VALUES(?,?,?)"));

        // Seed yesterday
        for (int hour = 6; hour <= 18; ++hour) {
            for (int min = 0; min < 60; min += 30) {
                const QDateTime dt(yesterday, QTime(hour, min, 0));
                const double progress = (hour + min / 60.0 - 6.0) / 12.0;
                const bool pumpState = (hour % 3 != 0);
                const double flow = pumpState ? (14.2 + (hour % 3) * 0.8 + ((min == 30) ? 0.4 : -0.2)) : 0.0;
                const double dist = 14.0 + std::sin(progress * 3.14159265 * 2.0) * 8.0 + (pumpState ? -2.0 : 4.0);
                const double distClamped = qMax(8.0, qMin(38.0, dist));
                const double liters = 750.0 + (hour - 6) * 80.0 + min * 1.3;

                QJsonObject m{
                    {"distance_cm", std::round(distClamped * 10.0) / 10.0},
                    {"flow_l_min", std::round(flow * 10.0) / 10.0},
                    {"total_liters", std::round(liters * 10.0) / 10.0},
                    {"pump_on", pumpState}
                };

                insert.addBindValue(QStringLiteral("son-190782"));
                insert.addBindValue(dt.toUTC().toString(Qt::ISODateWithMs));
                insert.addBindValue(QString::fromUtf8(QJsonDocument(m).toJson(QJsonDocument::Compact)));
                insert.exec();
            }
        }

        // Seed today (only up to current time, never in future)
        const int curH = qMax(6, qMin(18, QTime::currentTime().hour()));
        for (int hour = 6; hour <= curH; ++hour) {
            const int maxMin = (hour == curH) ? QTime::currentTime().minute() : 59;
            for (int min = 0; min <= maxMin; min += 30) {
                const QDateTime dt(today, QTime(hour, min, 0));
                const double progress = (hour + min / 60.0 - 6.0) / 12.0;
                const bool pumpState = (hour % 2 == 0);
                const double flow = pumpState ? (15.2 + ((min == 30) ? 0.6 : -0.3)) : 0.0;
                const double dist = 16.0 + std::sin(progress * 3.14159265 * 2.0) * 6.0;
                const double distClamped = qMax(8.0, qMin(38.0, dist));
                const double liters = 1350.0 + (hour - 6) * 65.0 + min * 1.1;

                QJsonObject m{
                    {"distance_cm", std::round(distClamped * 10.0) / 10.0},
                    {"flow_l_min", std::round(flow * 10.0) / 10.0},
                    {"total_liters", std::round(liters * 10.0) / 10.0},
                    {"pump_on", pumpState}
                };

                insert.addBindValue(QStringLiteral("son-190782"));
                insert.addBindValue(dt.toUTC().toString(Qt::ISODateWithMs));
                insert.addBindValue(QString::fromUtf8(QJsonDocument(m).toJson(QJsonDocument::Compact)));
                insert.exec();
            }
        }
    }

    // 5. Seed Login History
    QSqlQuery checkLoginHist(m_db);
    if (checkLoginHist.exec(QStringLiteral("SELECT COUNT(*) FROM login_history")) && checkLoginHist.next()) {
        if (checkLoginHist.value(0).toInt() == 0) {
            const QString t1 = QDateTime::currentDateTime().addSecs(-7200).toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            const QString t2 = QDateTime::currentDateTime().addSecs(-3600).toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            const QString t3 = QDateTime::currentDateTime().addSecs(-600).toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            QSqlQuery q(m_db);
            q.exec(QStringLiteral("INSERT INTO login_history(username,role,ip_address,status,created_at) VALUES('admin','admin','127.0.0.1','success','%1')").arg(t1));
            q.exec(QStringLiteral("INSERT INTO login_history(username,role,ip_address,status,created_at) VALUES('son','user','192.168.1.110','success','%1')").arg(t2));
            q.exec(QStringLiteral("INSERT INTO login_history(username,role,ip_address,status,created_at) VALUES('guest','user','192.168.1.116','failed','%1')").arg(t3));
        }
    }

    // 6. Seed Audit Logs
    QSqlQuery checkAudit(m_db);
    if (checkAudit.exec(QStringLiteral("SELECT COUNT(*) FROM audit_log")) && checkAudit.next()) {
        if (checkAudit.value(0).toInt() == 0) {
            const QString t1 = QDateTime::currentDateTime().addSecs(-7100).toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            const QString t2 = QDateTime::currentDateTime().addSecs(-3500).toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            const QString t3 = QDateTime::currentDateTime().addSecs(-1200).toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            QSqlQuery q(m_db);
            q.exec(QStringLiteral("INSERT INTO audit_log(username,role,action,target,details,created_at) VALUES('admin','admin','ĐIỀU KHIỂN RƠ-LE','son-190782','Bật máy bơm tự động','%1')").arg(t1));
            q.exec(QStringLiteral("INSERT INTO audit_log(username,role,action,target,details,created_at) VALUES('son','user','CÀI ĐẶT CẤU HÌNH','son-190782','Cập nhật ngưỡng lưu lượng & mực nước','%1')").arg(t2));
            q.exec(QStringLiteral("INSERT INTO audit_log(username,role,action,target,details,created_at) VALUES('admin','admin','GHÉP NỐI THIẾT BỊ','son-190782','Gán trạm bơm tự động','%1')").arg(t3));
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
    const QString normalizedId = deviceId.trimmed();
    if (normalizedId.compare(QStringLiteral("son-190782"), Qt::CaseInsensitive) != 0 &&
        normalizedId.compare(QStringLiteral("150304"), Qt::CaseInsensitive) != 0) {
        if (errorCode) *errorCode = QStringLiteral("invalid_device");
        if (error) *error = QStringLiteral("Chỉ cho phép thêm thiết bị ID 'son-190782' hoặc '150304' (Firmware Sơn)");
        return false;
    }
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
    if (normalizedId.compare(QStringLiteral("son-190782"), Qt::CaseInsensitive) != 0 &&
        normalizedId.compare(QStringLiteral("150304"), Qt::CaseInsensitive) != 0)
        return false;
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    QString deviceType = QStringLiteral("uv_pressure");
    if (metrics.contains(QStringLiteral("sound_vpp")))
        deviceType = QStringLiteral("temperature_sound");
    else if (metrics.contains(QStringLiteral("weather_pressure")) || metrics.contains(QStringLiteral("pressure_hpa")))
        deviceType = QStringLiteral("uv_pressure");
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
    if (normalizedId.compare(QStringLiteral("son-190782"), Qt::CaseInsensitive) != 0 &&
        normalizedId.compare(QStringLiteral("150304"), Qt::CaseInsensitive) != 0)
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

    if (period == QStringLiteral("month")) {
        const QString prefix = selectedDate.left(7); // "yyyy-MM"
        QSqlQuery query(m_db);
        query.prepare(QStringLiteral(
            "SELECT coalesce(substr(datetime(recorded_at, 'localtime'), 1, 10), substr(recorded_at, 1, 10)) as day_str, metrics_json FROM device_telemetry_log "
            "WHERE device_id=? COLLATE NOCASE AND ("
            "    coalesce(substr(datetime(recorded_at, 'localtime'), 1, 7), substr(recorded_at, 1, 7)) = ? "
            "    OR substr(recorded_at, 1, 7) = ?"
            ") "
            "ORDER BY recorded_at ASC"));
        query.addBindValue(deviceId.trimmed());
        query.addBindValue(prefix);
        query.addBindValue(prefix);
        if (!query.exec()) {
            if (error) *error = query.lastError().text();
            return {};
        }

        QMap<QString, QHash<QString, double>> daySums;
        QMap<QString, QHash<QString, int>> dayCounts;
        QMap<QString, int> daySampleCount;
        QSet<QString> metricKeys;
        QHash<QString, double> overallSums;
        QHash<QString, int> overallCounts;

        while (query.next()) {
            const QString dayStr = query.value(0).toString();
            const QJsonObject metrics = QJsonDocument::fromJson(query.value(1).toByteArray()).object();
            daySampleCount[dayStr] += 1;
            for (auto it = metrics.begin(); it != metrics.end(); ++it) {
                if (it.value().isDouble()) {
                    const QString k = it.key();
                    const double val = it.value().toDouble();
                    metricKeys.insert(k);
                    daySums[dayStr][k] += val;
                    dayCounts[dayStr][k] += 1;
                    overallSums[k] += val;
                    overallCounts[k] += 1;
                }
            }
        }

        QStringList sortedKeys(metricKeys.begin(), metricKeys.end());
        sortedKeys.sort();
        QJsonArray keys;
        for (const QString &k : sortedKeys)
            keys.append(k);

        QJsonObject averages;
        for (const QString &k : sortedKeys) {
            if (overallCounts.value(k) > 0)
                averages.insert(k, std::round((overallSums.value(k) / overallCounts.value(k)) * 100.0) / 100.0);
        }

        QJsonArray rows;
        QStringList dayList = daySums.keys();
        std::sort(dayList.begin(), dayList.end(), std::greater<QString>());

        const QDate today = QDate::currentDate();
        for (const QString &dayStr : dayList) {
            const QDate d = QDate::fromString(dayStr, QStringLiteral("yyyy-MM-dd"));
            if (d.isValid() && d > today) {
                continue; // Do not include future days
            }
            QJsonObject dayMetrics;
            for (const QString &k : sortedKeys) {
                if (dayCounts[dayStr].value(k) > 0) {
                    const double avg = daySums[dayStr].value(k) / dayCounts[dayStr].value(k);
                    dayMetrics.insert(k, std::round(avg * 100.0) / 100.0);
                }
            }
            const QString label = d.isValid() ? QStringLiteral("Ngày %1").arg(d.toString(QStringLiteral("dd/MM"))) : dayStr;
            rows.append(QJsonObject{
                {QStringLiteral("recorded_at"), dayStr},
                {QStringLiteral("label"), label},
                {QStringLiteral("day"), d.isValid() ? d.day() : 0},
                {QStringLiteral("sample_count"), daySampleCount.value(dayStr, 0)},
                {QStringLiteral("metrics"), dayMetrics}
            });
        }

        return QJsonObject{
            {QStringLiteral("device_id"), deviceId},
            {QStringLiteral("device_name"), deviceName},
            {QStringLiteral("added_by"), ownerUsername},
            {QStringLiteral("added_at"), deviceCreatedAt},
            {QStringLiteral("period"), period},
            {QStringLiteral("selected_date"), selectedDate},
            {QStringLiteral("total"), rows.size()},
            {QStringLiteral("metric_keys"), keys},
            {QStringLiteral("averages"), averages},
            {QStringLiteral("data"), rows}
        };
    }

    if (period == QStringLiteral("year")) {
        const QString prefix = selectedDate.left(4); // "yyyy"
        QSqlQuery query(m_db);
        query.prepare(QStringLiteral(
            "SELECT coalesce(substr(datetime(recorded_at, 'localtime'), 1, 7), substr(recorded_at, 1, 7)) as month_str, metrics_json FROM device_telemetry_log "
            "WHERE device_id=? COLLATE NOCASE AND ("
            "    coalesce(substr(datetime(recorded_at, 'localtime'), 1, 4), substr(recorded_at, 1, 4)) = ? "
            "    OR substr(recorded_at, 1, 4) = ?"
            ") "
            "ORDER BY recorded_at ASC"));
        query.addBindValue(deviceId.trimmed());
        query.addBindValue(prefix);
        query.addBindValue(prefix);
        if (!query.exec()) {
            if (error) *error = query.lastError().text();
            return {};
        }

        QMap<QString, QHash<QString, double>> monthSums;
        QMap<QString, QHash<QString, int>> monthCounts;
        QMap<QString, int> monthSampleCount;
        QSet<QString> metricKeys;
        QHash<QString, double> overallSums;
        QHash<QString, int> overallCounts;

        while (query.next()) {
            const QString monthStr = query.value(0).toString();
            const QJsonObject metrics = QJsonDocument::fromJson(query.value(1).toByteArray()).object();
            monthSampleCount[monthStr] += 1;
            for (auto it = metrics.begin(); it != metrics.end(); ++it) {
                if (it.value().isDouble()) {
                    const QString k = it.key();
                    const double val = it.value().toDouble();
                    metricKeys.insert(k);
                    monthSums[monthStr][k] += val;
                    monthCounts[monthStr][k] += 1;
                    overallSums[k] += val;
                    overallCounts[k] += 1;
                }
            }
        }

        QStringList sortedKeys(metricKeys.begin(), metricKeys.end());
        sortedKeys.sort();
        QJsonArray keys;
        for (const QString &k : sortedKeys)
            keys.append(k);

        QJsonObject averages;
        for (const QString &k : sortedKeys) {
            if (overallCounts.value(k) > 0)
                averages.insert(k, std::round((overallSums.value(k) / overallCounts.value(k)) * 100.0) / 100.0);
        }

        QJsonArray rows;
        QStringList monthList = monthSums.keys();
        std::sort(monthList.begin(), monthList.end(), std::greater<QString>());

        for (const QString &monthStr : monthList) {
            QJsonObject monthMetrics;
            for (const QString &k : sortedKeys) {
                if (monthCounts[monthStr].value(k) > 0) {
                    const double avg = monthSums[monthStr].value(k) / monthCounts[monthStr].value(k);
                    monthMetrics.insert(k, std::round(avg * 100.0) / 100.0);
                }
            }
            const int monthNum = monthStr.mid(5, 2).toInt();
            const QString label = QStringLiteral("Tháng %1").arg(monthNum, 2, 10, QChar('0'));
            rows.append(QJsonObject{
                {QStringLiteral("recorded_at"), monthStr},
                {QStringLiteral("label"), label},
                {QStringLiteral("month"), monthNum},
                {QStringLiteral("sample_count"), monthSampleCount.value(monthStr, 0)},
                {QStringLiteral("metrics"), monthMetrics}
            });
        }

        return QJsonObject{
            {QStringLiteral("device_id"), deviceId},
            {QStringLiteral("device_name"), deviceName},
            {QStringLiteral("added_by"), ownerUsername},
            {QStringLiteral("added_at"), deviceCreatedAt},
            {QStringLiteral("period"), period},
            {QStringLiteral("selected_date"), selectedDate},
            {QStringLiteral("total"), rows.size()},
            {QStringLiteral("metric_keys"), keys},
            {QStringLiteral("averages"), averages},
            {QStringLiteral("data"), rows}
        };
    }

    // Default period: "day" (second-level resolution)
    const QString prefix = selectedDate.left(10);
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT recorded_at,metrics_json FROM device_telemetry_log "
        "WHERE device_id=? COLLATE NOCASE AND ("
        "    coalesce(substr(datetime(recorded_at, 'localtime'), 1, 10), substr(recorded_at, 1, 10)) = ? "
        "    OR substr(recorded_at, 1, 10) = ?"
        ") "
        "ORDER BY recorded_at DESC LIMIT ?"));
    query.addBindValue(deviceId.trimmed());
    query.addBindValue(prefix);
    query.addBindValue(prefix);
    query.addBindValue(safeLimit(limit));
    if (!query.exec()) {
        if (error) *error = query.lastError().text();
        return {};
    }
    QJsonArray rows;
    QSet<QString> metricKeys;
    QHash<QString, double> sums;
    QHash<QString, int> counts;

    while (query.next()) {
        const QString recTime = query.value(0).toString();
        const QJsonObject metrics = QJsonDocument::fromJson(query.value(1).toByteArray()).object();
        for (auto it = metrics.begin(); it != metrics.end(); ++it) {
            if (it.value().isDouble()) {
                metricKeys.insert(it.key());
                sums[it.key()] += it.value().toDouble();
                counts[it.key()] += 1;
            }
        }
        rows.append(QJsonObject{{"recorded_at", recTime},
                                {"metrics", metrics}});
    }

    QStringList sortedKeys(metricKeys.begin(), metricKeys.end());
    sortedKeys.sort();
    QJsonArray keys;
    QJsonObject averages;
    for (const QString &key : sortedKeys) {
        keys.append(key);
        if (counts.value(key) > 0)
            averages.insert(key, std::round((sums.value(key) / counts.value(key)) * 100.0) / 100.0);
    }
    return QJsonObject{{"device_id", deviceId},
                       {"device_name", deviceName},
                       {"added_by", ownerUsername},
                       {"added_at", deviceCreatedAt},
                       {"period", period},
                       {"selected_date", selectedDate}, {"total", rows.size()},
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
        "WHERE d.online=1 AND d.last_seen_at>=? AND (d.device_id='son-190782' COLLATE NOCASE OR d.device_id='150304' COLLATE NOCASE) "
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

bool Database::recordLoginHistory(const QString &username, const QString &role, const QString &status,
                                  const QString &ip)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO login_history(username,role,ip_address,status,created_at) "
        "VALUES(?,?,?,?,?)"));
    query.addBindValue(username.trimmed());
    query.addBindValue(role);
    query.addBindValue(ip.isEmpty() ? QStringLiteral("127.0.0.1") : ip);
    query.addBindValue(status);
    query.addBindValue(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    return query.exec();
}

QJsonArray Database::loginHistory(int limit) const
{
    QJsonArray result;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT id, username, role, ip_address, status, created_at "
        "FROM login_history ORDER BY id DESC LIMIT ?"));
    query.addBindValue(qBound(1, limit, 500));
    if (query.exec()) {
        while (query.next()) {
            result.append(QJsonObject{
                {QStringLiteral("id"), query.value(0).toInt()},
                {QStringLiteral("username"), query.value(1).toString()},
                {QStringLiteral("role"), query.value(2).toString()},
                {QStringLiteral("ip_address"), query.value(3).toString()},
                {QStringLiteral("status"), query.value(4).toString()},
                {QStringLiteral("created_at"), query.value(5).toString()}
            });
        }
    }
    return result;
}

bool Database::recordAuditLog(const QString &username, const QString &role, const QString &action,
                              const QString &target, const QString &details)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO audit_log(username,role,action,target,details,created_at) "
        "VALUES(?,?,?,?,?,?)"));
    query.addBindValue(username.trimmed());
    query.addBindValue(role);
    query.addBindValue(action);
    query.addBindValue(target);
    query.addBindValue(details);
    query.addBindValue(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    return query.exec();
}

QJsonArray Database::auditLogs(const QString &usernameFilter, int limit) const
{
    QJsonArray result;
    QSqlQuery query(m_db);
    if (!usernameFilter.trimmed().isEmpty()) {
        query.prepare(QStringLiteral(
            "SELECT id, username, role, action, target, details, created_at "
            "FROM audit_log WHERE username = ? COLLATE NOCASE "
            "ORDER BY id DESC LIMIT ?"));
        query.addBindValue(usernameFilter.trimmed());
        query.addBindValue(qBound(1, limit, 500));
    } else {
        query.prepare(QStringLiteral(
            "SELECT id, username, role, action, target, details, created_at "
            "FROM audit_log ORDER BY id DESC LIMIT ?"));
        query.addBindValue(qBound(1, limit, 500));
    }
    if (query.exec()) {
        while (query.next()) {
            result.append(QJsonObject{
                {QStringLiteral("id"), query.value(0).toInt()},
                {QStringLiteral("username"), query.value(1).toString()},
                {QStringLiteral("role"), query.value(2).toString()},
                {QStringLiteral("action"), query.value(3).toString()},
                {QStringLiteral("target"), query.value(4).toString()},
                {QStringLiteral("details"), query.value(5).toString()},
                {QStringLiteral("created_at"), query.value(6).toString()}
            });
        }
    }
    return result;
}

