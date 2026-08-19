#pragma once

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QPointF>

class PlantSoilVisualizerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlantSoilVisualizerWidget(QWidget *parent = nullptr);

    void setSoilMoisture(double pct);
    void setTemperature(double tempC);
    void setHumidity(double humPct);
    void setPumpActive(bool active);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct WaterParticle {
        QPointF pos;
        double speedY;
        double size;
        double alpha;
    };

    double m_soilMoisturePct = 55.0;
    double m_temperatureC = 27.5;
    double m_humidityPct = 65.0;
    bool m_pumpActive = false;

    double m_leafFlutter = 0.0;
    QVector<WaterParticle> m_particles;
    QTimer m_animTimer;
};
