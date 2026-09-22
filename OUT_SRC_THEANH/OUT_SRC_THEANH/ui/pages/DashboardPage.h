#pragma once

#include "models/SensorReading.h"
#include "ui/dialogs/SensorDetailDialog.h"
#include "ui/dialogs/SelectOnlineDeviceDialog.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QPointer>
#include <QVector>
#include <QWidget>

namespace Ui { class DashboardPage; }
class QChart;
class QChartView;
class QHBoxLayout;
class QLabel;
class QLineSeries;
class QProgressBar;
class QPushButton;
class QResizeEvent;
class QValueAxis;

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
    void historyPageRequested();
    void devicesPageRequested();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUiCustom();
    void applyResponsiveLayout();
    void updateVoltageDisplay(double voltageV);
    void updateCurrentDisplay(double currentA);
    void updatePowerDisplay(double powerW);
    void updateRealtimeChart(double voltageV, double currentA, double powerW);

    Ui::DashboardPage *ui;

    QString m_username = QStringLiteral("Admin");
    QString m_deviceId = QStringLiteral("Theanh-190782");
    QString m_deviceName = QStringLiteral("Trạm đo điện năng ACS712 & ZMPT101B");
    bool m_hasDevice = false;
    bool m_isOnline = false;
    bool m_relayActive = true;

    QJsonArray m_availableDevices;
    QPointer<SelectOnlineDeviceDialog> m_currentSelectDialog;

    // --- LEFT PANEL: Sơ đồ nguyên lý biểu trưng & Cảm biến ---
    QWidget *m_leftPanel = nullptr;

    // ZMPT101B Block
    QWidget *m_blockZmpt = nullptr;
    QLabel *m_voltageValLabel = nullptr;
    QLabel *m_voltageStatusBadge = nullptr;
    QLabel *m_voltageFreqLabel = nullptr;
    QLabel *m_voltagePeakLabel = nullptr;
    QLabel *m_voltageImageLabel = nullptr;

    // ACS712 Block
    QWidget *m_blockAcs = nullptr;
    QLabel *m_currentValLabel = nullptr;
    QLabel *m_currentStatusBadge = nullptr;
    QLabel *m_currentMaxLabel = nullptr;
    QLabel *m_currentSensLabel = nullptr;
    QLabel *m_currentImageLabel = nullptr;

    // Load & Relay Block
    QWidget *m_blockLoadRelay = nullptr;
    QLabel *m_powerValLabel = nullptr;
    QLabel *m_powerFactorLabel = nullptr;
    QLabel *m_energyKwhLabel = nullptr;
    QPushButton *m_relayButton = nullptr;
    QPushButton *m_viewHistoryButton = nullptr;
    QPushButton *m_devicesButton = nullptr;

    // --- RIGHT PANEL: Biểu đồ thời gian thực (giống Trung Kiên - không co rít) ---
    QWidget *m_rightPanel = nullptr;
    QChartView *m_chartView = nullptr;
    QChart *m_chart = nullptr;
    QLineSeries *m_voltageSeries = nullptr;
    QLineSeries *m_currentSeries = nullptr;
    QLineSeries *m_powerSeries = nullptr;
    QValueAxis *m_axisX = nullptr;
    QValueAxis *m_axisY_Voltage = nullptr;
    QValueAxis *m_axisY_Current = nullptr;
    QPushButton *m_chartFilterAll = nullptr;
    QPushButton *m_chartFilterVoltage = nullptr;
    QPushButton *m_chartFilterCurrent = nullptr;
    QPushButton *m_chartFilterPower = nullptr;
    int m_chartMode = 0; // 0: All, 1: Voltage, 2: Current, 3: Power
    int m_sampleCount = 0;

    // Safety & Range Bars
    QProgressBar *m_voltageBar = nullptr;
    QProgressBar *m_currentBar = nullptr;
    QLabel *m_statusAdviceLabel = nullptr;
    QLabel *m_lastUpdatedLabel = nullptr;

    QHBoxLayout *m_masterHLayout = nullptr;

    // Sensor histories
    QVector<SensorDataPoint> m_voltageHistory;
    QVector<SensorDataPoint> m_currentHistory;
    QVector<SensorDataPoint> m_powerHistory;

    double m_curVoltage = 220.0;
    double m_curCurrent = 2.35;
    double m_curPower = 517.0;
};
