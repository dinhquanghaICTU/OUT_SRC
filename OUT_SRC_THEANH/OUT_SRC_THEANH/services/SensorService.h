#pragma once

#include <QObject>
#include <QTimer>

#include "models/SensorReading.h"

class ApiClient;

class SensorService : public QObject
{
    Q_OBJECT

public:
    explicit SensorService(ApiClient *apiClient, QObject *parent = nullptr);
    void start();
    void stop();

signals:
    void readingUpdated(const SensorReading &reading);
    void thresholdExceeded(const QString &message, double value);

private:
    ApiClient *m_apiClient;
    QTimer m_refreshTimer;
    int m_demoSample = 0;
};
