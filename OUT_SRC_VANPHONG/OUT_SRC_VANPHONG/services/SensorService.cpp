#include "SensorService.h"

#include "api/ApiClient.h"
#include "config/AppConfig.h"

#include <QDateTime>
#include <QtMath>

SensorService::SensorService(ApiClient *apiClient, QObject *parent)
    : QObject(parent), m_apiClient(apiClient)
{
    m_refreshTimer.setInterval(AppConfig::RefreshIntervalMs);

    connect(&m_refreshTimer, &QTimer::timeout, this, [this] {
        if (AppConfig::DemoMode) {
            ++m_demoCycle;

            // Soil moisture logic
            if (m_currentReading.pumpActive) {
                // Pump is running -> increase soil moisture
                m_currentReading.soilMoisturePct = qMin(92.0, m_currentReading.soilMoisturePct + 4.5);
                m_currentReading.pumpRuntimeSeconds += 1;
                m_currentReading.waterTankLevelPct = qMax(5.0, m_currentReading.waterTankLevelPct - 0.2);
                m_currentReading.totalWaterUsedLiters += 0.05;

                // Auto stop pump when soil reaches 75%
                if (m_currentReading.autoIrrigationMode && m_currentReading.soilMoisturePct >= 75.0) {
                    m_currentReading.pumpActive = false;
                    m_currentReading.pumpRuntimeSeconds = 0;
                }
            } else {
                // Pump is OFF -> slow gradual drying
                m_currentReading.soilMoisturePct = qMax(20.0, m_currentReading.soilMoisturePct - 0.3);

                // Auto trigger pump when soil is dry (< 38%)
                if (m_currentReading.autoIrrigationMode && m_currentReading.soilMoisturePct <= 38.0) {
                    m_currentReading.pumpActive = true;
                    m_currentReading.totalWateringCountToday++;
                    emit safetyAlert(QStringLiteral("SOIL_SENSOR"), QStringLiteral("Độ ẩm đất thấp (38%) - Đã tự động kích hoạt máy bơm"));
                }
            }

            // Determine soil status
            if (m_currentReading.soilMoisturePct < 40.0) {
                m_currentReading.soilStatus = QStringLiteral("DRY");
            } else if (m_currentReading.soilMoisturePct > 80.0) {
                m_currentReading.soilStatus = QStringLiteral("WET");
            } else {
                m_currentReading.soilStatus = QStringLiteral("OPTIMAL");
            }

            // DHT11 sensor readings
            m_currentReading.temperatureC = 27.0 + 2.5 * qSin(m_demoCycle / 20.0);
            m_currentReading.humidityPct = 65.0 + 8.0 * qCos(m_demoCycle / 15.0);
            m_currentReading.measuredAt = QDateTime::currentDateTime();

            emit readingUpdated(m_currentReading);
        } else {
            m_apiClient->requestLatestReading();
        }
    });

    connect(m_apiClient, &ApiClient::latestReadingReceived, this, [this](const SensorReading &r) {
        m_currentReading = r;
        emit readingUpdated(r);
    });
}

void SensorService::start()
{
    if (AppConfig::DemoMode) {
        m_currentReading.soilMoisturePct = 52.0;
        m_currentReading.temperatureC = 27.5;
        m_currentReading.humidityPct = 65.0;
        m_currentReading.pumpActive = false;
        m_currentReading.waterTankLevelPct = 88.0;
        m_currentReading.totalWateringCountToday = 4;
    } else {
        m_apiClient->requestLatestReading();
    }
    m_refreshTimer.start();
}

void SensorService::stop()
{
    m_refreshTimer.stop();
}

void SensorService::setPumpManual(bool active)
{
    m_currentReading.pumpActive = active;
    if (active) {
        m_currentReading.pumpRuntimeSeconds = 0;
        m_currentReading.totalWateringCountToday++;
    }
}

void SensorService::setAutoIrrigationMode(bool enabled)
{
    m_currentReading.autoIrrigationMode = enabled;
    emit readingUpdated(m_currentReading);
}

void SensorService::triggerSimDrySoil()
{
    m_currentReading.soilMoisturePct = 32.0;
    m_currentReading.soilStatus = QStringLiteral("DRY");
    emit safetyAlert(QStringLiteral("SOIL_SENSOR"), QStringLiteral("Cảnh báo: Đất khô hạn nghiêm trọng (32%)"));
}

void SensorService::triggerSimMoistSoil()
{
    m_currentReading.soilMoisturePct = 78.0;
    m_currentReading.soilStatus = QStringLiteral("OPTIMAL");
}
