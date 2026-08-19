#pragma once

#include "models/SensorReading.h"

#include <QWidget>
#include <QLineSeries>

namespace Ui { class DashboardPage; }
class QChart;
class QChartView;
class DoorVisualizerWidget;

class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage() override;

    void setUsername(const QString &username);
    void updateReading(const SensorReading &reading);

signals:
    void doorCommandRequested(const QString &action);
    void simMotionRequested();
    void simObstacleRequested(bool blocked);

private:
    void setupCharts();

    Ui::DashboardPage *ui;
    DoorVisualizerWidget *m_doorVisualizer = nullptr;
    QLineSeries *m_positionSeries = nullptr;
    QChart *m_chart = nullptr;
    QChartView *m_chartView = nullptr;
    int m_sampleIndex = 0;
    bool m_obstacleActive = false;
};
