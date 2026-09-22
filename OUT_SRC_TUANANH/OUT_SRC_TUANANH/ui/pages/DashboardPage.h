#pragma once

#include "models/SensorReading.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QPointer>
#include <QVector>
#include <QWidget>

namespace Ui { class DashboardPage; }

class QLabel;
class QPushButton;
class QTimer;

// 1. Semi-circular Arc Lux Gauge Widget
class LuxGaugeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LuxGaugeWidget(QWidget *parent = nullptr);
    void setValue(double val, double maxVal = 1000.0);
    double value() const { return m_value; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_value = 0.0;
    double m_max = 1000.0;
};

// 2. Rolling Realtime Lux Waveform Widget
class LuxWaveformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LuxWaveformWidget(QWidget *parent = nullptr);
    void addSample(double val);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> m_samples;
};

// 3. Custom Vector Light Bulb Widget
class LightBulbWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LightBulbWidget(QWidget *parent = nullptr);
    void setState(bool isOn);
    bool isOn() const { return m_isOn; }

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    bool m_isOn = false;
};

// 4. Custom Vector PIR Motion & Presence Radar Widget
class PirMotionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PirMotionWidget(QWidget *parent = nullptr);
    void setMotion(bool hasMotion);
    bool hasMotion() const { return m_hasMotion; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_hasMotion = false;
};

// 5. Main Dashboard Page
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

    void openConfigDialog();

signals:
    void claimDeviceRequested(const QString &deviceId, const QString &deviceName);
    void releaseDeviceRequested(const QString &deviceId);
    void relayControlRequested(const QString &deviceId, bool state);
    void deviceConfigRequested(const QString &deviceId, const QJsonObject &config);
    void refreshDevicesRequested();
    void navigateToPageRequested(int pageIndex);

private:
    void setupUi();
    void updateUiState();
    void checkAutoLightingLogic();

    Ui::DashboardPage *ui;

    QString m_username = QStringLiteral("Admin");
    QString m_deviceId = QStringLiteral("150808");
    QString m_deviceName = QStringLiteral("Trạm Chiếu Sáng Thông Minh");
    bool m_hasDevice = true;
    bool m_isOnline = true;

    // Telemetry state
    double m_curLux = 125.0;
    bool m_curMotion = false;
    bool m_relayState = false;

    // Auto-lighting mode & thresholds
    bool m_autoModeActive = true;
    double m_minLuxThreshold = 50.0;   // Bật đèn khi Lux <= min hoặc có người
    double m_maxLuxThreshold = 500.0;  // Tắt đèn khi Lux >= max
    int m_samplingIntervalSec = 2;

    // Relay command pending
    bool m_isRelayPending = false;
    bool m_pendingRelayState = false;
    QTimer *m_relayPendingTimer = nullptr;

    QJsonArray m_availableDevices;

    // UI elements
    LuxGaugeWidget *m_gaugeWidget = nullptr;
    QLabel *m_luxNumberLabel = nullptr;
    QLabel *m_luxBadgeLabel = nullptr;
    QLabel *m_thresholdLabel = nullptr;

    LightBulbWidget *m_bulbWidget = nullptr;
    QLabel *m_lampStateLabel = nullptr;
    QPushButton *m_relayToggleBtn = nullptr;
    QPushButton *m_autoModeBtn = nullptr;
    QPushButton *m_manualModeBtn = nullptr;
    QPushButton *m_configBtn = nullptr;

    PirMotionWidget *m_pirWidget = nullptr;
    QLabel *m_motionBadgeLabel = nullptr;
    QLabel *m_motionDetailLabel = nullptr;
    LuxWaveformWidget *m_waveformWidget = nullptr;
    QLabel *m_nodeStatusLabel = nullptr;
};
