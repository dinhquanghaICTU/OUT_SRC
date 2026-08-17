#include "SensorService.h"

#include "api/ApiClient.h"
#include "config/AppConfig.h"

#include <QDateTime>
#include <QtMath>

SensorService::SensorService(ApiClient *apiClient, QObject *parent)
    : QObject(parent), m_apiClient(apiClient)
{
    m_refreshTimer.setInterval(AppConfig::RefreshIntervalMs);
    if (AppConfig::DemoMode) {
        connect(&m_refreshTimer, &QTimer::timeout, this, [this] {
            SensorReading reading;
            reading.pressureHpa = 1008.0 + 7.5 * qSin(m_demoSample / 5.0);
            reading.distanceCm = 42.0 + 26.0 * qCos(m_demoSample / 4.0);
            reading.temperatureC = 29.0 + 2.5 * qSin(m_demoSample / 7.0);
            reading.measuredAt = QDateTime::currentDateTime();
            ++m_demoSample;
            emit readingUpdated(reading);
            if (reading.distanceCm < 20.0)
                emit thresholdExceeded(tr("Khoảng cách dưới ngưỡng"), reading.distanceCm);
        });
    } else {
        connect(&m_refreshTimer, &QTimer::timeout,
                m_apiClient, &ApiClient::requestLatestReading);
        connect(m_apiClient, &ApiClient::latestReadingReceived,
                this, &SensorService::readingUpdated);
    }
}

void SensorService::start()
{
    if (!AppConfig::DemoMode)
        m_apiClient->requestLatestReading();
    else
        QMetaObject::invokeMethod(&m_refreshTimer, "timeout", Qt::DirectConnection);
    m_refreshTimer.start();
}

void SensorService::stop()
{
    m_refreshTimer.stop();
}
