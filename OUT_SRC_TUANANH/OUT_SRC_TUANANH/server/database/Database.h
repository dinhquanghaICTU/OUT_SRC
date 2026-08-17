#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QString>

class Database final
{
public:
    explicit Database(QString path);
    ~Database();

    bool open(QString *error);
    bool verifyUser(const QString &username, const QString &password, QString *role);
    bool createUser(const QString &username, const QString &password, const QString &role,
                    QString *errorCode, QString *error);
    bool updateUser(const QString &oldUsername, const QString &newUsername,
                    const QString &password, const QString &role, bool enabled,
                    QString *errorCode, QString *error);
    bool deleteUser(const QString &username, QString *errorCode, QString *error);
    QJsonArray users(QString *error) const;

    bool claimDevice(const QString &username, const QString &deviceId, const QString &name,
                     QString *errorCode, QString *error);
    bool releaseDevice(const QString &username, const QString &deviceId,
                       QString *errorCode, QString *error);
    QJsonArray devicesForUser(const QString &username, int onlineWindowSeconds,
                              QString *error) const;
    bool recordDevicePresence(const QString &deviceId, bool online,
                              const QJsonObject &metrics, QString *error);
    bool recordTelemetry(const QString &deviceId, const QJsonObject &metrics,
                         const QString &recordedAt, QString *error);
    QJsonObject deviceTelemetryHistory(const QString &username, const QString &deviceId,
                                       const QString &period, const QString &selectedDate,
                                       int limit, QString *error) const;
    bool recordDeviceState(const QString &deviceId, const QJsonObject &state, QString *error);
    bool userOwnsDevice(const QString &username, const QString &deviceId, QString *error) const;
    bool updateDeviceConfig(const QString &username, const QString &deviceId,
                            const QJsonObject &config, QString *errorCode, QString *error);
    QJsonArray availableDevices(int onlineWindowSeconds, QString *error) const;

    bool insertReading(double pressureHpa, double distanceCm, double temperatureC,
                       const QString &measuredAt, QString *error);
    QJsonObject latestReading(QString *error) const;
    QJsonArray pressureHistory(int limit, QString *error) const;
    QJsonArray distanceHistory(int limit, QString *error) const;
    QJsonArray alerts(int limit, QString *error) const;

    QJsonObject config(QString *error) const;
    bool updateConfig(const QJsonObject &config, QString *error);
    bool isOpen() const;

private:
    bool migrate(QString *error);
    bool seedDefaults(QString *error);
    static QByteArray makeSalt();
    static QByteArray hashPassword(const QString &password, const QByteArray &salt);

    QString m_path;
    QString m_connectionName;
    QSqlDatabase m_db;
};
