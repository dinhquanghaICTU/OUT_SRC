#pragma once

#include "models/SensorReading.h"

#include <QWidget>

namespace Ui { class DashboardPage; }
class QGridLayout;
class QLineSeries;
class QLabel;
class QResizeEvent;
class QTableWidget;
class QWidget;

class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage() override;
    void setUsername(const QString &username);

public slots:
    void updateReading(const SensorReading &reading);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void applyResponsiveLayout();
    void addHistory(const SensorReading &reading);
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
    QGridLayout *m_contentGrid = nullptr;
    QWidget *m_weatherCard = nullptr;
    QWidget *m_scenesCard = nullptr;
    QWidget *m_mediaCard = nullptr;
    QWidget *m_cameraCard = nullptr;
    QWidget *m_historyCard = nullptr;
    int m_sampleIndex = 0;
};
