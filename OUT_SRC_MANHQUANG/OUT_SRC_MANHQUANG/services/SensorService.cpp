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
            // Simulated realistic smart door cycle
            ++m_demoCycle;

            // PIR motion triggers every 15-20 cycles
            if (m_demoCycle % 20 == 2) {
                m_currentReading.motionDetected = true;
                m_currentReading.pirTriggerCount++;
                m_simTargetPos = 100.0; // Open door
            } else if (m_demoCycle % 20 == 8) {
                m_currentReading.motionDetected = false;
            }

            // Simulate IR obstacle occasionally
            if (m_demoCycle % 35 == 12) {
                m_currentReading.irBlocked = true;
                m_currentReading.antiPinchActive = true;
                emit safetyAlert(QStringLiteral("IR_SENSOR"), QStringLiteral("Phát hiện vật cản tại mép cửa - Kích hoạt chống kẹt"));
                // Safety reverse if door was closing
                if (m_currentReading.doorPositionPct < 100.0 && m_currentReading.doorPositionPct > 0.0) {
                    m_simTargetPos = 100.0;
                }
            } else if (m_demoCycle % 35 == 17) {
                m_currentReading.irBlocked = false;
                m_currentReading.antiPinchActive = false;
            }

            // Auto close after motion ceases
            if (m_demoCycle % 20 == 12 && !m_currentReading.irBlocked && !m_currentReading.motionDetected) {
                m_simTargetPos = 0.0; // Close door
            }

            // Move door toward target position smoothly
            if (std::abs(m_currentReading.doorPositionPct - m_simTargetPos) > 1.0) {
                if (m_currentReading.doorPositionPct < m_simTargetPos) {
                    m_currentReading.doorPositionPct = qMin(100.0, m_currentReading.doorPositionPct + 15.0);
                    m_currentReading.doorState = QStringLiteral("OPENING");
                    m_currentReading.motorDirection = QStringLiteral("CW");
                    m_currentReading.motorSpeedRpm = 120.0;
                    m_currentReading.motorCurrentMa = 450.0;
                } else {
                    if (!m_currentReading.irBlocked) {
                        m_currentReading.doorPositionPct = qMax(0.0, m_currentReading.doorPositionPct - 15.0);
                        m_currentReading.doorState = QStringLiteral("CLOSING");
                        m_currentReading.motorDirection = QStringLiteral("CCW");
                        m_currentReading.motorSpeedRpm = 100.0;
                        m_currentReading.motorCurrentMa = 420.0;
                        if (m_currentReading.doorPositionPct <= 0.0) {
                            m_currentReading.passageCount++;
                        }
                    } else {
                        m_currentReading.doorState = QStringLiteral("OBSTACLE_STOP");
                        m_currentReading.motorSpeedRpm = 0.0;
                        m_currentReading.motorCurrentMa = 50.0;
                    }
                }
            } else {
                m_currentReading.doorPositionPct = m_simTargetPos;
                m_currentReading.motorSpeedRpm = 0.0;
                m_currentReading.motorDirection = QStringLiteral("STOP");
                m_currentReading.motorCurrentMa = 80.0;
                m_currentReading.doorState = (m_simTargetPos >= 99.0) ? QStringLiteral("OPEN") : QStringLiteral("CLOSED");
            }

            m_currentReading.currentStep = static_cast<int>((m_currentReading.doorPositionPct / 100.0) * 3200);
            m_currentReading.targetStep = static_cast<int>((m_simTargetPos / 100.0) * 3200);
            m_currentReading.temperatureC = 28.5 + 2.0 * qSin(m_demoCycle / 15.0);
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
        m_currentReading.doorPositionPct = 0.0;
        m_currentReading.doorState = QStringLiteral("CLOSED");
        m_currentReading.motorSpeedRpm = 0.0;
        m_currentReading.passageCount = 24;
    } else {
        m_apiClient->requestLatestReading();
    }
    m_refreshTimer.start();
}

void SensorService::stop()
{
    m_refreshTimer.stop();
}

void SensorService::triggerManualMotion()
{
    m_currentReading.motionDetected = true;
    m_currentReading.pirTriggerCount++;
    m_simTargetPos = 100.0;
}

void SensorService::triggerObstacle(bool blocked)
{
    m_currentReading.irBlocked = blocked;
    m_currentReading.antiPinchActive = blocked;
    if (blocked) {
        emit safetyAlert(QStringLiteral("IR_SENSOR"), QStringLiteral("Vật cản phát hiện - Khóa chiều đóng"));
        m_simTargetPos = 100.0;
    }
}
