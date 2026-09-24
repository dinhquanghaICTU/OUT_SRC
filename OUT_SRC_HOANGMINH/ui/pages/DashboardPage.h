#pragma once

#include "models/SensorReading.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QVector>
#include <QWidget>

namespace Ui { class DashboardPage; }
class CoolingSystemWidget;
class QChartView;
class QLineSeries;
class QProgressBar;
class QValueAxis;
class QLabel;
class QPushButton;

struct HoangMinhDataPoint {
    QDateTime timestamp;
    double value;
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
    void setDevices(const QJsonArray &devices);
    void openCoolingConfig();

signals:
    void relayControlRequested(const QString &deviceId, bool state);
    void deviceConfigRequested(const QString &deviceId, const QJsonObject &config);
    void navigateToPageRequested(int pageIndex);

private:
    void setupUi();
    void updateDisplays();
    void checkAutoCoolingLogic();
    QChartView *buildChartView(QLineSeries *series, QValueAxis **axisX, QValueAxis **axisY,
                               double minY, double maxY, const QString &unit, const QColor &color);
    void appendPoint(QLineSeries *series, QVector<HoangMinhDataPoint> &history, double val, QValueAxis *axisX, QValueAxis *axisY);

    Ui::DashboardPage *ui;
    QString m_username;
    QString m_deviceId = QStringLiteral("190782");
    QString m_deviceName = QStringLiteral("Trạm Làm Mát Tự Động");

    // Metrics state
    double m_currentTemp = 28.5;
    double m_currentSound = 0.12;
    bool m_fanOn = false;
    bool m_isOnline = true;
    bool m_autoCoolingMode = true;

    // Thresholds
    double m_fanStartTemp = 34.0;
    double m_fanStopTemp = 28.0;
    double m_maxSoundVpp = 1.5;
    int m_sampleIntervalSec = 2;
    double m_peakTemp = 28.5;

    // History for charts
    QVector<HoangMinhDataPoint> m_tempHistory;
    QVector<HoangMinhDataPoint> m_soundHistory;

    // Column 1 Widgets (Cooling Turbine Tower)
    CoolingSystemWidget *m_coolingWidget = nullptr;
    QLabel *m_digitalTempLabel = nullptr;
    QLabel *m_tempSubLabel = nullptr;
    QLabel *m_fanStateBadge = nullptr;
    QPushButton *m_fanActionBtn = nullptr;

    // Column 2 Widgets (Climate Console & Automation Hub)
    QLabel *m_stationTitle = nullptr;
    QLabel *m_devIdPill = nullptr;
    QLabel *m_onlinePill = nullptr;
    QPushButton *m_modeAutoBtn = nullptr;
    QPushButton *m_modeManualBtn = nullptr;
    QLabel *m_threshStartText = nullptr;
    QLabel *m_threshStopText = nullptr;
    QPushButton *m_configBtn = nullptr;
    QLabel *m_soundValText = nullptr;
    QProgressBar *m_soundBar = nullptr;
    QLabel *m_peakValText = nullptr;
    QLabel *m_relayStateText = nullptr;

    // Column 3 Widgets (Stacked Telemetry Monitors)
    QLineSeries *m_tempSeries = nullptr;
    QValueAxis *m_tempAxisX = nullptr;
    QValueAxis *m_tempAxisY = nullptr;

    QLineSeries *m_soundSeries = nullptr;
    QValueAxis *m_soundAxisX = nullptr;
    QValueAxis *m_soundAxisY = nullptr;
};
