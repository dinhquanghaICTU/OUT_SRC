#include "EmbeddedServer.h"

#include "server/api/ApiServer.h"
#include "server/database/Database.h"
#include "server/mqtt/MqttDiscoveryService.h"
#include "server/system/HotspotManager.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>

EmbeddedServer::EmbeddedServer(QObject *parent) : QObject(parent) {}

EmbeddedServer::~EmbeddedServer() = default;

bool EmbeddedServer::start(QString *error)
{
    return start(Config{}, error);
}

bool EmbeddedServer::start(const Config &config, QString *error)
{
    if (m_started) return true;
    m_config = config;

    m_databasePath = m_config.databasePath;
    if (m_databasePath.isEmpty()) {
        const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        if (!QDir().mkpath(dataDir)) {
            if (error) *error = QStringLiteral("Không tạo được thư mục dữ liệu: %1").arg(dataDir);
            return false;
        }
        m_databasePath = dataDir + QStringLiteral("/environment.db");
    }

    m_database = std::make_unique<Database>(m_databasePath);
    QString dbError;
    if (!m_database->open(&dbError)) {
        if (error) *error = QStringLiteral("Database startup failed: %1").arg(dbError);
        return false;
    }

    if (m_config.enableHotspot) {
        HotspotManager::Config hotspot;
        hotspot.interfaceName = m_config.hotspotInterface;
        hotspot.connectionName = m_config.hotspotName;
        hotspot.ssid = m_config.hotspotSsid;
        hotspot.password = m_config.hotspotPassword;
        hotspot.addressCidr = m_config.hotspotAddressCidr;

        QString hotspotError;
        if (!HotspotManager::ensureStarted(hotspot, &hotspotError)) {
            qWarning().noquote() << QStringLiteral("Hotspot startup warning: %1").arg(hotspotError);
            qWarning().noquote() << QStringLiteral("App vẫn chạy, nhưng ESP cần WiFi AP khác hoặc chạy bằng quyền sudo trên Pi.");
        }
    }

    m_mqtt = std::make_unique<MqttDiscoveryService>(m_database.get());
    m_api = std::make_unique<ApiServer>(m_database.get(), m_mqtt.get());

    QString listenError;
    if (!m_api->listen(m_config.httpPort, &listenError)) {
        if (error) *error = listenError;
        return false;
    }

    m_mqtt->start(m_config.mqttHost, m_config.mqttPort);
    m_started = true;

    qInfo().noquote() << QStringLiteral("HoangAnh app embedded API listening on %1").arg(baseUrl());
    qInfo().noquote() << QStringLiteral("SQLite: %1").arg(m_databasePath);
    qInfo().noquote() << QStringLiteral("MQTT broker: %1:%2").arg(m_config.mqttHost).arg(m_config.mqttPort);
    return true;
}

QString EmbeddedServer::baseUrl() const
{
    return QStringLiteral("http://127.0.0.1:%1").arg(m_config.httpPort);
}

QString EmbeddedServer::databasePath() const
{
    return m_databasePath;
}
