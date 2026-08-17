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

// 1. Vertical Cyber Lux Meter Bar
class VerticalLuxBarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VerticalLuxBarWidget(QWidget *parent = nullptr);
    void setValue(double val, double maxVal = 2000.0);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    double m_value = 0.0;
    double m_max = 2000.0;
};

// 2. Futuristic Tactical Motion Radar
class TacticalRadarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TacticalRadarWidget(QWidget *parent = nullptr);
    void setDetected(bool detected);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    bool m_detected = false;
    double m_angle = 0.0;
};

// 3. Mini Cyber Waveform Strip
class CyberWaveformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CyberWaveformWidget(QWidget *parent = nullptr);
    void addSample(double val);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QVector<double> m_samples;
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

    void openLuxDetail();
    void openAddDeviceDialog();

signals:
    void claimDeviceRequested(const QString &deviceId, const QString &deviceName);
    void releaseDeviceRequested(const QString &deviceId);
    void relayControlRequested(const QString &deviceId, bool state);
    void refreshDevicesRequested();

private:
    void setupHudDashboard();
    void updateHudState();

    Ui::DashboardPage *ui;

    QString m_username = "Admin";
    QString m_deviceId = "";
    QString m_deviceName = "";
    bool m_hasDevice = false;
    bool m_isOnline = false;

    QJsonArray m_availableDevices;
    QPointer<SelectOnlineDeviceDialog> m_currentSelectDialog;

    // HUD Custom Widgets
    VerticalLuxBarWidget *m_luxBar = nullptr;
    TacticalRadarWidget *m_radarWidget = nullptr;
    CyberWaveformWidget *m_waveWidget = nullptr;

    // Telemetry labels
    QLabel *m_luxValueLbl = nullptr;
    QLabel *m_luxStatusBadge = nullptr;
    QLabel *m_motionStatusBadge = nullptr;
    QLabel *m_motionDetailLbl = nullptr;

    // Controls
    QPushButton *m_lightSwitchBtn = nullptr;
    QPushButton *m_autoModeBtn = nullptr;
    bool m_autoModeActive = true;

    // Right Pod
    QLabel *m_devPodTitle = nullptr;
    QLabel *m_devStatusBadge = nullptr;
    QPushButton *m_devActionBtn = nullptr;
    QLabel *m_pingLbl = nullptr;
    QLabel *m_sampleRateLbl = nullptr;

    // Data buffer
    QVector<SensorDataPoint> m_luxHistory;
    double m_curLux = 0.0;
    bool m_curMotion = false;
    bool m_relayState = false;
    bool m_isRelayPending = false;
    bool m_pendingRelayState = false;
    QTimer *m_relayPendingTimer = nullptr;
};
