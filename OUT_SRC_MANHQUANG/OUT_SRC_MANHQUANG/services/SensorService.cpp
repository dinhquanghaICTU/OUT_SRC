#include "SensorService.h"

#include "api/ApiClient.h"
#include "config/AppConfig.h"

#include <QDateTime>
#include <QtMath>

SensorService::SensorService(ApiClient *apiClient, QObject *parent)
    : QObject(parent), m_apiClient(apiClient)
{
    m_refreshTimer.setInterval(AppConfig::RefreshIntervalMs);
    m_animTimer.setInterval(40); // 25 FPS smooth interpolation

    connect(&m_refreshTimer, &QTimer::timeout, this, [this] {
        if (!AppConfig::DemoMode) {
            m_apiClient->requestLatestReading();
        }
    });

    connect(&m_animTimer, &QTimer::timeout, this, &SensorService::onAnimationTick);

    connect(m_apiClient, &ApiClient::latestReadingReceived, this, [this](const SensorReading &r) {
        m_currentReading.motionDetected = r.motionDetected;
        m_currentReading.irBlocked = r.irBlocked;
        m_currentReading.antiPinchActive = r.irBlocked;
        m_currentReading.temperatureC = r.temperatureC;
        m_currentReading.passageCount = r.passageCount;
        m_currentReading.measuredAt = r.measuredAt;

        if (r.motionDetected) {
            m_currentReading.pirTriggerCount++;
        }

        // Set target from firmware telemetry
        m_simTargetPos = r.doorPositionPct;

        if (r.irBlocked) {
            m_currentReading.doorState = QStringLiteral("OBSTACLE_STOP");
            m_currentReading.motorDirection = QStringLiteral("STOP");
            m_currentReading.motorSpeedRpm = 0.0;
            emit safetyAlert(QStringLiteral("IR_SENSOR"), QStringLiteral("Phát hiện vật cản tại mép cửa - Đang dừng an toàn"));
        }
    });
}

void SensorService::start()
{
    m_currentReading.doorPositionPct = 0.0;
    m_currentReading.doorState = QStringLiteral("CLOSED");
    m_currentReading.motorDirection = QStringLiteral("STOP");
    m_currentReading.motorSpeedRpm = 0.0;
    m_currentReading.passageCount = 0;
    m_currentReading.temperatureC = 28.5;
    m_simTargetPos = 0.0;

    m_refreshTimer.start();
    m_animTimer.start();

    if (!AppConfig::DemoMode) {
        m_apiClient->requestLatestReading();
    }
}

void SensorService::stop()
{
    m_refreshTimer.stop();
    m_animTimer.stop();
}

void SensorService::onAnimationTick()
{
    const double diff = m_simTargetPos - m_currentReading.doorPositionPct;
    if (std::abs(diff) > 0.5) {
        m_isMoving = true;
        const double step = (diff > 0) ? qMin(diff, 4.0) : qMax(diff, -4.0);
        m_currentReading.doorPositionPct += step;
        m_currentReading.doorPositionPct = qBound(0.0, m_currentReading.doorPositionPct, 100.0);

        if (step > 0) {
            m_currentReading.doorState = QStringLiteral("OPENING");
            m_currentReading.motorDirection = QStringLiteral("CW");
            m_currentReading.motorSpeedRpm = 12.0;
            m_currentReading.motorCurrentMa = 420.0;
        } else {
            if (m_currentReading.irBlocked) {
                m_currentReading.doorState = QStringLiteral("OBSTACLE_STOP");
                m_currentReading.motorDirection = QStringLiteral("STOP");
                m_currentReading.motorSpeedRpm = 0.0;
                m_simTargetPos = 100.0; // Auto safety reverse
            } else {
                m_currentReading.doorState = QStringLiteral("CLOSING");
                m_currentReading.motorDirection = QStringLiteral("CCW");
                m_currentReading.motorSpeedRpm = 12.0;
                m_currentReading.motorCurrentMa = 390.0;
            }
        }
    } else {
        if (m_isMoving) {
            m_isMoving = false;
            if (m_simTargetPos <= 1.0 && m_currentReading.doorPositionPct <= 1.0) {
                m_currentReading.passageCount++;
            }
        }
        m_currentReading.doorPositionPct = m_simTargetPos;
        m_currentReading.motorSpeedRpm = 0.0;
        m_currentReading.motorDirection = QStringLiteral("STOP");
        m_currentReading.motorCurrentMa = 50.0;
        if (m_currentReading.irBlocked) {
            m_currentReading.doorState = QStringLiteral("OBSTACLE_STOP");
        } else {
            m_currentReading.doorState = (m_currentReading.doorPositionPct >= 95.0)
                ? QStringLiteral("OPEN") : QStringLiteral("CLOSED");
        }
    }

    m_currentReading.currentStep = static_cast<int>((m_currentReading.doorPositionPct / 100.0) * 1024);
    m_currentReading.targetStep = static_cast<int>((m_simTargetPos / 100.0) * 1024);
    m_currentReading.measuredAt = QDateTime::currentDateTime();

    emit readingUpdated(m_currentReading);
}

void SensorService::triggerManualOpen()
{
    m_simTargetPos = 100.0;
    m_currentReading.motionDetected = true;
    m_currentReading.pirTriggerCount++;
}

void SensorService::triggerManualClose()
{
    if (!m_currentReading.irBlocked) {
        m_simTargetPos = 0.0;
        m_currentReading.motionDetected = false;
    }
}

void SensorService::triggerManualHoldOpen()
{
    m_simTargetPos = 100.0;
}

void SensorService::triggerManualStop()
{
    m_simTargetPos = m_currentReading.doorPositionPct;
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
        emit safetyAlert(QStringLiteral("IR_SENSOR"), QStringLiteral("Vật cản phát hiện - Kích hoạt mở ngược bảo vệ chống kẹt"));
        m_simTargetPos = 100.0;
    }
}
