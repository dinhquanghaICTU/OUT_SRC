#pragma once

#include "models/SensorReading.h"
#include "ui/dialogs/SensorDetailDialog.h"
#include "ui/dialogs/SelectOnlineDeviceDialog.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QPointer>
#include <QVector>
#include <QWidget>

namespace Ui { class DashboardPage; }

class QLabel;
class QPushButton;
class QProgressBar;
class QStackedWidget;

// Custom Gauge & Visual Components
class CircularGaugeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CircularGaugeWidget(QWidget *parent = nullptr);
    void setValue(double val, double minVal = 0, double maxVal = 300, const QString &unit = "V");
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    double m_value = 0.0;
    double m_min = 0;
    double m_max = 300;
    QString m_unit = "V";
};

class SemiCircleGaugeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SemiCircleGaugeWidget(QWidget *parent = nullptr);
    void setValue(double val, double maxVal = 2000, const QString &unit = "W");
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    double m_value = 0.0;
    double m_max = 2000;
    QString m_unit = "W";
};

class NeonAreaChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NeonAreaChartWidget(QWidget *parent = nullptr);
    void addPoint(double val1, double val2);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QVector<double> m_data1; // Power % wave
    QVector<double> m_data2; // Current % wave
};

class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage() override;
    void setUsername(const QString &username);

public slots:
    void updateReading(const SensorReading &reading);
    void updateDeviceMetrics(const QJsonObject &metrics);
    void setAvailableDevices(const QJsonArray &devices);
    void setOwnedDevices(const QJsonArray &devices);
    void setDeviceId(const QString &deviceId);

    void openVoltageDetail();
    void openCurrentDetail();
    void openPowerDetail();
    void openAddDeviceDialog();

signals:
    void claimDeviceRequested(const QString &deviceId, const QString &deviceName);
    void releaseDeviceRequested(const QString &deviceId);
    void relayControlRequested(const QString &deviceId, bool state);
    void refreshDevicesRequested();

private:
    void setupCustomDashboard();
    void updateDeviceCardState();
    void updateSensorStatusBadges();

    Ui::DashboardPage *ui;

    QString m_username = "Admin";
    QString m_deviceId = "";
    QString m_deviceName = "";
    bool m_hasDevice = false;
    bool m_isOnline = false;

    QJsonArray m_availableDevices;
    QPointer<SelectOnlineDeviceDialog> m_currentSelectDialog;

    // Visual Widgets
    CircularGaugeWidget *m_circularGauge = nullptr;
    SemiCircleGaugeWidget *m_semiCircleGauge = nullptr;
    NeonAreaChartWidget *m_areaChart = nullptr;

    QLabel *m_voltageValLbl = nullptr;
    QLabel *m_voltageBadge = nullptr;
    QLabel *m_voltageSubLbl = nullptr;

    QLabel *m_currentValLbl = nullptr;
    QLabel *m_powerBigLbl = nullptr;

    // Card 7 Widgets (Add Device or Device Control)
    QStackedWidget *m_deviceCardStack = nullptr;
    QWidget *m_noDeviceWidget = nullptr;
    QWidget *m_hasDeviceWidget = nullptr;

    QLabel *m_devIdLbl = nullptr;
    QLabel *m_devOnlineBadge = nullptr;
    QLabel *m_devVoltageLbl = nullptr;
    QLabel *m_devCurrentLbl = nullptr;
    QPushButton *m_relayBtn = nullptr;

    // Real historical buffers from ESP32
    QVector<SensorDataPoint> m_voltageHistory;
    QVector<SensorDataPoint> m_currentHistory;
    QVector<SensorDataPoint> m_powerHistory;

    double m_curVoltage = 0.0;
    double m_curCurrent = 0.0;
    double m_curPower = 0.0;
    bool m_relayState = false;
};
