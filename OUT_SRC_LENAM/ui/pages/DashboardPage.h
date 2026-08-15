#pragma once

#include "models/SensorReading.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QWidget>

namespace Ui { class DashboardPage; }
class QLineSeries;
class QLabel;
class QTableWidget;

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

private:
    void addHistory(const SensorReading &reading);
    void addTelemetryRow(const QJsonObject &device, const QJsonObject &metrics);
    void appendSeriesPoint(QLineSeries *series, double value, double fallbackMin, double fallbackMax);

    Ui::DashboardPage *ui;
    QLabel *m_titleLabel;
    QLabel *m_clockValue;
    QLabel *m_dateValue;
    QLabel *m_pressureChip;
    QLabel *m_distanceChip;
    QLabel *m_temperatureChip;
    QLabel *m_pressureValue;
    QLabel *m_distanceValue;
    QLabel *m_alertValue;
    QLabel *m_updatedAt;
    QLineSeries *m_pressureSeries;
    QLineSeries *m_distanceSeries;
    QTableWidget *m_history;
    int m_sampleIndex = 0;
};
