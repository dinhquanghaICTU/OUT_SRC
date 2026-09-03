#pragma once

#include "models/SensorReading.h"

#include <QObject>
#include <QTimer>

class ApiClient;

class SensorService : public QObject
{
    Q_OBJECT

public:
    explicit SensorService(ApiClient *apiClient, QObject *parent = nullptr);

    void start();
    void stop();
    void setPumpManual(bool active);
    void setAutoIrrigationMode(bool enabled);
    void triggerSimDrySoil();
    void triggerSimMoistSoil();

signals:
    void readingUpdated(const SensorReading &reading);
    void thresholdExceeded(const QString &message, double value);
    void safetyAlert(const QString &source, const QString &message);

private:
    ApiClient *m_apiClient;
    QTimer m_refreshTimer;
    SensorReading m_currentReading;
    int m_demoCycle = 0;
};
