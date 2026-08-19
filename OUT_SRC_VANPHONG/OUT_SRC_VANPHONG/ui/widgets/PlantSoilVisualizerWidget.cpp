#include "PlantSoilVisualizerWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QRandomGenerator>
#include <QtMath>

PlantSoilVisualizerWidget::PlantSoilVisualizerWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(220, 240);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(&m_animTimer, &QTimer::timeout, this, [this] {
        m_leafFlutter += 0.05;

        if (m_pumpActive) {
            if (m_particles.size() < 35 && QRandomGenerator::global()->bounded(10) > 2) {
                WaterParticle p;
                p.pos = QPointF(width() * 0.2 + QRandomGenerator::global()->bounded(static_cast<int>(width() * 0.6)), 20.0);
                p.speedY = 3.0 + QRandomGenerator::global()->bounded(40) / 10.0;
                p.size = 2.5 + QRandomGenerator::global()->bounded(20) / 10.0;
                p.alpha = 200 + QRandomGenerator::global()->bounded(55);
                m_particles.append(p);
            }
        }

        for (int i = m_particles.size() - 1; i >= 0; --i) {
            m_particles[i].pos.ry() += m_particles[i].speedY;
            if (m_particles[i].pos.y() > height() - 40) {
                m_particles.removeAt(i);
            }
        }

        update();
    });
    m_animTimer.start(40);
}

void PlantSoilVisualizerWidget::setSoilMoisture(double pct)
{
    m_soilMoisturePct = qBound(0.0, pct, 100.0);
    update();
}

void PlantSoilVisualizerWidget::setTemperature(double tempC)
{
    m_temperatureC = tempC;
    update();
}

void PlantSoilVisualizerWidget::setHumidity(double humPct)
{
    m_humidityPct = humPct;
    update();
}

void PlantSoilVisualizerWidget::setPumpActive(bool active)
{
    m_pumpActive = active;
    if (!active) m_particles.clear();
    update();
}

void PlantSoilVisualizerWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();

    // Dark bio pod background
    QRectF bgRect(0, 0, w, h);
    p.fillRect(bgRect, QColor(QStringLiteral("#05130b")));

    // Hydroponic/Soil Pot Curve at the bottom
    const double potTop = h - 85.0;
    QRectF potRect(14, potTop, w - 28, 75);

    // Soil moisture gradient
    double moistFactor = m_soilMoisturePct / 100.0;
    int r = static_cast<int>(110 * (1.0 - moistFactor) + 20 * moistFactor);
    int g = static_cast<int>(55 * (1.0 - moistFactor) + 35 * moistFactor);
    int b = static_cast<int>(15 * (1.0 - moistFactor) + 25 * moistFactor);

    QLinearGradient potGrad(potRect.topLeft(), potRect.bottomLeft());
    potGrad.setColorAt(0.0, QColor(r + 20, g + 15, b + 10));
    potGrad.setColorAt(1.0, QColor(r, g, b));

    p.setBrush(potGrad);
    p.setPen(QPen(QColor(QStringLiteral("#10b981")), 1.8));
    p.drawRoundedRect(potRect, 10, 10);

    // Soil texture surface
    p.setPen(QPen(QColor(52, 211, 153, 180), 2.0));
    p.drawLine(QPointF(16, potTop + 3), QPointF(w - 16, potTop + 3));

    // Plant Stem & Leaves
    const double plantCenterX = w / 2.0;
    const double plantBaseY = potTop + 2.0;
    const double plantHeight = 65.0;

    QPainterPath stemPath;
    stemPath.moveTo(plantCenterX, plantBaseY);
    double sway = qSin(m_leafFlutter) * 3.5;
    stemPath.quadTo(plantCenterX + sway, plantBaseY - plantHeight / 2.0, plantCenterX + sway * 1.5, plantBaseY - plantHeight);
    p.setPen(QPen(QColor(QStringLiteral("#16a34a")), 4.0, Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawPath(stemPath);

    // Plant roots inside soil
    p.setPen(QPen(QColor(QStringLiteral("#ca8a04")), 1.5, Qt::DashLine));
    p.drawLine(QPointF(plantCenterX, plantBaseY), QPointF(plantCenterX - 25, potTop + 45));
    p.drawLine(QPointF(plantCenterX, plantBaseY), QPointF(plantCenterX + 25, potTop + 45));
    p.drawLine(QPointF(plantCenterX, plantBaseY), QPointF(plantCenterX, potTop + 55));

    // Left Leaf
    p.save();
    p.translate(plantCenterX + sway * 0.8, plantBaseY - plantHeight * 0.55);
    p.rotate(-35.0 + qSin(m_leafFlutter) * 8.0);
    p.setBrush(QColor(QStringLiteral("#22c55e")));
    p.setPen(QPen(QColor(QStringLiteral("#15803d")), 1.2));
    p.drawEllipse(QRectF(0, -10, 26, 14));
    p.restore();

    // Right Leaf
    p.save();
    p.translate(plantCenterX + sway * 1.1, plantBaseY - plantHeight * 0.75);
    p.rotate(35.0 - qSin(m_leafFlutter + 1.0) * 8.0);
    p.setBrush(QColor(QStringLiteral("#4ade80")));
    p.setPen(QPen(QColor(QStringLiteral("#15803d")), 1.2));
    p.drawEllipse(QRectF(-26, -10, 26, 14));
    p.restore();

    // Top Leaf Sprout
    p.save();
    p.translate(plantCenterX + sway * 1.5, plantBaseY - plantHeight);
    p.rotate(qSin(m_leafFlutter * 1.2) * 6.0);
    p.setBrush(QColor(QStringLiteral("#86efac")));
    p.setPen(QPen(QColor(QStringLiteral("#16a34a")), 1.2));
    p.drawEllipse(QRectF(-9, -20, 18, 22));
    p.restore();

    // Water particles when pump is ON
    if (m_pumpActive) {
        p.setBrush(QColor(QStringLiteral("#0284c7")));
        p.setPen(QPen(QColor(QStringLiteral("#38bdf8")), 1.5));
        p.drawRect(QRectF(plantCenterX - 10, 16, 20, 8));

        p.setPen(Qt::NoPen);
        for (const auto &part : m_particles) {
            p.setBrush(QColor(56, 189, 248, static_cast<int>(part.alpha)));
            p.drawEllipse(part.pos, part.size, part.size * 1.5);
        }
    }

    // Status Footer inside canvas
    p.setPen(QColor(QStringLiteral("#86efac")));
    QFont f = p.font();
    f.setPointSize(8);
    f.setBold(true);
    p.setFont(f);
    p.drawText(QRectF(20, potTop + 55, w - 40, 16), Qt::AlignCenter,
               tr("Mực ẩm rễ: %1%").arg(QString::number(m_soilMoisturePct, 'f', 1)));
}
