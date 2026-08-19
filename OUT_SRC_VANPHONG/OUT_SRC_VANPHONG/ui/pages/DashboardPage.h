#pragma once

#include "models/SensorReading.h"

#include <QWidget>
#include <QLineSeries>

namespace Ui { class DashboardPage; }
class QChart;
class QChartView;
class PlantSoilVisualizerWidget;

class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage() override;

    void setUsername(const QString &username);
    void updateReading(const SensorReading &reading);

signals:
    void pumpCommandRequested(bool state);
    void simDrySoilRequested();
    void simMoistSoilRequested();

private:
    void setupCharts();

    Ui::DashboardPage *ui;
    PlantSoilVisualizerWidget *m_plantVisualizer = nullptr;
    QLineSeries *m_soilSeries = nullptr;
    QLineSeries *m_humiditySeries = nullptr;
    QChart *m_chart = nullptr;
    QChartView *m_chartView = nullptr;
    int m_sampleIndex = 0;
};
