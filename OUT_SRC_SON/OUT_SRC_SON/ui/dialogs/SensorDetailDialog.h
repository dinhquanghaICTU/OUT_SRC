#pragma once

#include <QDialog>
#include <QDateTime>
#include <QVector>
#include <QPointF>

class QChartView;
class QLineSeries;
class QTableWidget;
class QStackedWidget;
class QPushButton;
class QLabel;
class QDoubleSpinBox;
class QSpinBox;
class QCheckBox;

struct SensorDataPoint {
    QDateTime timestamp;
    double value;
};

class SensorDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorDetailDialog(const QString &sensorName,
                                const QString &unit,
                                const QString &accentColor,
                                const QVector<SensorDataPoint> &history,
                                QWidget *parent = nullptr,
                                double minThreshold = 0.0,
                                double maxThreshold = 100.0);

signals:
    void thresholdChanged(double minVal, double maxVal);

private:
    void setupUI(const QString &sensorName, const QString &unit, const QString &accentColor, double minThreshold, double maxThreshold);
    void populateData(const QVector<SensorDataPoint> &history, const QString &unit, const QString &accentColor);

    QStackedWidget *m_viewStack = nullptr;
    QPushButton *m_chartModeBtn = nullptr;
    QPushButton *m_tableModeBtn = nullptr;
    QPushButton *m_thresholdModeBtn = nullptr;
    QTableWidget *m_tableWidget = nullptr;
    QChartView *m_chartView = nullptr;

    // Threshold widgets
    QDoubleSpinBox *m_minThresholdSpin = nullptr;
    QDoubleSpinBox *m_maxThresholdSpin = nullptr;
    QCheckBox *m_autoPumpCheck = nullptr;
    QSpinBox *m_intervalSpin = nullptr;
    QLabel *m_saveStatusLbl = nullptr;
};
