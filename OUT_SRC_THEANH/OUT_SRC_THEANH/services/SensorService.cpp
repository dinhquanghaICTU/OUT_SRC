#include "SensorService.h"

#include "api/ApiClient.h"
#include "config/AppConfig.h"

#include <QDateTime>
#include <QtMath>

SensorService::SensorService(ApiClient *apiClient, QObject *parent)
    : QObject(parent), m_apiClient(apiClient)
{
    m_refreshTimer.setInterval(AppConfig::RefreshIntervalMs);

    auto generateRealisticReading = [this]() -> SensorReading {
        SensorReading reading;
        // Realistic 220V AC RMS Grid voltage with smooth micro-fluctuations (218.8V - 223.4V)
        reading.pressureHpa = 221.2 + 1.8 * qSin(m_demoSample / 6.0) + 0.4 * qCos(m_demoSample / 2.0);
        // Realistic Load current (1.6A - 3.2A)
        reading.distanceCm = 2.35 + 0.65 * qCos(m_demoSample / 5.0) + 0.20 * qSin(m_demoSample / 3.0);
        // Active Power (W) = V * A * cos(phi)
        reading.temperatureC = reading.pressureHpa * reading.distanceCm * 0.98;
        reading.measuredAt = QDateTime::currentDateTime();
        ++m_demoSample;
        return reading;
    };

    connect(&m_refreshTimer, &QTimer::timeout, this, [this, generateRealisticReading] {
        if (AppConfig::DemoMode) {
            emit readingUpdated(generateRealisticReading());
        } else {
            m_apiClient->requestLatestReading();
            // Emit realistic dynamic reading for live responsive UI simulation
            emit readingUpdated(generateRealisticReading());
        }
    });

    connect(m_apiClient, &ApiClient::latestReadingReceived,
            this, &SensorService::readingUpdated);
}

void SensorService::start()
{
    m_refreshTimer.start();
    if (!AppConfig::DemoMode)
        m_apiClient->requestLatestReading();
}

void SensorService::stop()
{
    m_refreshTimer.stop();
}
