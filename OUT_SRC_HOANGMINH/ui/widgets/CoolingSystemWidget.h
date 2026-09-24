#pragma once

#include <QWidget>
#include <QColor>

class CoolingSystemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CoolingSystemWidget(QWidget *parent = nullptr);
    ~CoolingSystemWidget() override = default;

    void setTemperature(double tempC);
    void setFanRunning(bool running);
    void setSoundVpp(double soundVpp);
    void setThreshold(double triggerTempC);

    double temperature() const { return m_temperatureC; }
    bool isFanRunning() const { return m_fanRunning; }
    double soundVpp() const { return m_soundVpp; }
    double threshold() const { return m_thresholdTempC; }

    QString statusText() const;
    QColor statusColor() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override { return QSize(140, 150); }
    QSize minimumSizeHint() const override { return QSize(110, 120); }

private:
    double m_temperatureC = 28.5;
    bool m_fanRunning = false;
    double m_soundVpp = 0.12;
    double m_thresholdTempC = 35.0;
};
