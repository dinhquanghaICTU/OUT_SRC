#pragma once

#include "models/SensorReading.h"
#include "ui/dialogs/SensorDetailDialog.h"
#include "ui/dialogs/PumpAutoConfigDialog.h"

#include <QJsonObject>
#include <QWidget>
#include <QVector>

namespace Ui { class DashboardPage; }
class QLabel;
class QPushButton;
class QLineSeries;
class QValueAxis;
class QStackedWidget;

class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage() override;
    void setUsername(const QString &username);

signals:
    void relayControlRequested(const QString &deviceId, bool state);
    void deviceConfigRequested(const QString &deviceId, const QJsonObject &config);
    void addDeviceRequested();
    void releaseDeviceRequested(const QString &deviceId);

public slots:
    void updateReading(const SensorReading &reading);
    void updateDeviceMetrics(const QJsonObject &metrics);
    void setDeviceId(const QString &deviceId);
    void setHasDevice(bool hasDevice, const QString &deviceId = QString(), const QString &deviceName = QString());
    void openPumpAutoConfig();

private:
    void setupDashboardLayout();
    void updateDisplays();
    void appendPoint(QLineSeries *series, QValueAxis *axisX, QValueAxis *axisY,
                     double value, double minVal, double maxVal);
    void openSensorDetail(const QString &sensorName, const QString &unit,
                          const QString &accentColor, const QVector<SensorDataPoint> &history);

    Ui::DashboardPage *ui;

    QString m_deviceId;
    QString m_deviceName;
    bool m_hasDevice = false;

    // Auto Pump Config
    bool m_autoPumpMode = false;
    double m_distanceStartCm = 35.0;
    double m_distanceStopCm = 10.0;

    // Real telemetry state from ESP32
    double m_currentTemp = 24.0;
    double m_currentHumidity = 60.0;
    double m_currentPressure = 1002.0;
    double m_currentDistance = 0.0;
    double m_currentFlow = 0.0;
    double m_totalLiters = 0.0;
    bool m_pumpOn = false;

    // Live UI Widgets
    QLabel *m_heroTempValue = nullptr;
    QLabel *m_heroHumidityValue = nullptr;
    QLabel *m_heroPressureValue = nullptr;
    QLabel *m_heroAcOpValue = nullptr;
    QLabel *m_heroIonValue = nullptr;
    QLabel *m_heroFanValue = nullptr;

    // Pump Control UI Widgets (Stack: 0 = Add Device +, 1 = Active Pump Control)
    QStackedWidget *m_pumpCardStack = nullptr;
    QLabel *m_pumpDeviceNameLbl = nullptr;
    QLabel *m_pumpStatusBadge = nullptr;
    QLabel *m_pumpFlowValue = nullptr;
    QLabel *m_pumpTotalValue = nullptr;
    QPushButton *m_pumpToggleButton = nullptr;

    // Realtime Charts & Axes (Khoảng cách & Lưu lượng)
    QLineSeries *m_distanceSeries = nullptr;
    QValueAxis *m_distanceAxisX = nullptr;
    QValueAxis *m_distanceAxisY = nullptr;

    QLineSeries *m_flowSeries = nullptr;
    QValueAxis *m_flowAxisX = nullptr;
    QValueAxis *m_flowAxisY = nullptr;

    // Historical buffers for Table & Chart Modal (Real data only)
    QVector<SensorDataPoint> m_distanceHistory;
    QVector<SensorDataPoint> m_flowHistory;

    int m_sampleIndex = 0;
};
