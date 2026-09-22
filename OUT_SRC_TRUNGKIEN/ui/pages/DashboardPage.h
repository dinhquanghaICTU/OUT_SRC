#pragma once

#include "models/SensorReading.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QWidget>

namespace Ui { class DashboardPage; }
class QChart;
class QChartView;
class QGridLayout;
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
    void setDevices(const QJsonArray &devices);

signals:
    void historyPageRequested();
    void relayToggleRequested(const QString &deviceId, bool state);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUiCustom();
    void updateUvCard(double uvIndex, double uvVoltage);
    void updatePressureCard(double pressureHpa, double tempC);
    void updateRealtimeChart(double uvIndex, double pressureHpa);
    void applyResponsiveLayout();

    Ui::DashboardPage *ui;
    QString m_username;
    QString m_activeDeviceId = QStringLiteral("Trungkien-150304");
    bool m_relayActive = false;

    // Header widgets
    QLabel *m_stationNameLabel = nullptr;
    QLabel *m_statusBadge = nullptr;
    QLabel *m_clockLabel = nullptr;
    QLabel *m_dateLabel = nullptr;
    QLabel *m_lastUpdatedLabel = nullptr;

    // UV Card widgets
    QLabel *m_uvValueLabel = nullptr;
    QLabel *m_uvRiskBadge = nullptr;
    QLabel *m_uvVoltageLabel = nullptr;
    QLabel *m_uvAdviceLabel = nullptr;
    QProgressBar *m_uvRiskBar = nullptr;

    // Pressure Card widgets
    QLabel *m_pressureValueLabel = nullptr;
    QLabel *m_pressureMmHgLabel = nullptr;
    QLabel *m_altitudeLabel = nullptr;
    QLabel *m_weatherForecastBadge = nullptr;
    QLabel *m_weatherAdviceLabel = nullptr;

    // Realtime chart widgets
    QChartView *m_chartView = nullptr;
    QChart *m_chart = nullptr;
    QLineSeries *m_uvSeries = nullptr;
    QLineSeries *m_pressureSeries = nullptr;
    QValueAxis *m_axisX = nullptr;
    QValueAxis *m_axisY_Uv = nullptr;
    QValueAxis *m_axisY_Pres = nullptr;
    QPushButton *m_chartFilterAll = nullptr;
    QPushButton *m_chartFilterUv = nullptr;
    QPushButton *m_chartFilterPres = nullptr;
    int m_chartMode = 0; // 0: All, 1: UV only, 2: Pres only
    int m_sampleCount = 0;

    // Station & Control widgets
    QLabel *m_stationIdLabel = nullptr;
    QLabel *m_sensorTypeLabel = nullptr;
    QLabel *m_samplingRateLabel = nullptr;
    QPushButton *m_relayButton = nullptr;
    QPushButton *m_viewHistoryButton = nullptr;

    QGridLayout *m_mainGrid = nullptr;
    QWidget *m_cardUv = nullptr;
    QWidget *m_cardPressure = nullptr;
    QWidget *m_cardChart = nullptr;
    QWidget *m_cardStation = nullptr;
};
