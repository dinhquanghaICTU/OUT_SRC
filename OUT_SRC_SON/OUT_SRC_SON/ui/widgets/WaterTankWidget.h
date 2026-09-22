#pragma once

#include <QWidget>
#include <QTimer>

class WaterTankWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WaterTankWidget(QWidget *parent = nullptr);
    ~WaterTankWidget() override = default;

    void setDistance(double distanceCm, double minDistanceCm = 10.0, double maxDistanceCm = 60.0);
    void setWaterPercent(double percent);

    double waterPercent() const { return m_percent; }
    double distanceCm() const { return m_distanceCm; }
    QString statusText() const;
    QColor statusColor() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_distanceCm = 18.5;
    double m_minDistanceCm = 10.0; // Bể đầy khi khoảng cách <= 10cm
    double m_maxDistanceCm = 60.0; // Bể cạn khi khoảng cách >= 60cm
    double m_percent = 83.0;
    double m_wavePhase = 0.0;
    QTimer m_animTimer;
};
