#pragma once

#include <QWidget>
#include <QTimer>

class DoorVisualizerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DoorVisualizerWidget(QWidget *parent = nullptr);

    void setDoorPosition(double pct);
    void setMotionDetected(bool detected);
    void setIrBlocked(bool blocked);
    void setDoorState(const QString &state);
    void setMotorSpeed(double rpm);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_doorPositionPct = 0.0; // 0.0 (Closed) -> 100.0 (Open)
    bool m_motionDetected = false;
    bool m_irBlocked = false;
    QString m_doorState = QStringLiteral("CLOSED");
    double m_motorRpm = 0.0;
    double m_radarPulse = 0.0;
    double m_motorAngle = 0.0;
    QTimer m_animTimer;
};
