#include "CoolingSystemWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QtMath>

CoolingSystemWidget::CoolingSystemWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void CoolingSystemWidget::setTemperature(double tempC)
{
    if (qFuzzyCompare(m_temperatureC, tempC))
        return;
    m_temperatureC = tempC;
    update();
}

void CoolingSystemWidget::setFanRunning(bool running)
{
    if (m_fanRunning == running)
        return;
    m_fanRunning = running;
    update();
}

void CoolingSystemWidget::setSoundVpp(double soundVpp)
{
    if (qFuzzyCompare(m_soundVpp, soundVpp))
        return;
    m_soundVpp = soundVpp;
    update();
}

void CoolingSystemWidget::setThreshold(double triggerTempC)
{
    if (qFuzzyCompare(m_thresholdTempC, triggerTempC))
        return;
    m_thresholdTempC = triggerTempC;
    update();
}

QString CoolingSystemWidget::statusText() const
{
    if (m_temperatureC >= 42.0)
        return m_fanRunning ? QStringLiteral("QUÁ NHIỆT - QUẠT BẬT") : QStringLiteral("QUÁ NHIỆT - QUẠT TẮT");
    if (m_temperatureC >= m_thresholdTempC)
        return m_fanRunning ? QStringLiteral("CẢNH BÁO - QUẠT BẬT") : QStringLiteral("CẢNH BÁO NHIỆT ĐỘ");
    if (m_fanRunning)
        return QStringLiteral("QUẠT: ĐANG BẬT");
    return QStringLiteral("QUẠT: ĐANG TẮT");
}

QColor CoolingSystemWidget::statusColor() const
{
    if (m_temperatureC >= 42.0)
        return QColor(239, 68, 68); // Red
    if (m_temperatureC >= m_thresholdTempC)
        return QColor(245, 158, 11); // Amber
    if (m_fanRunning)
        return QColor(6, 182, 212); // Cyan
    return QColor(100, 116, 139); // Slate Gray
}

void CoolingSystemWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const int w = width();
    const int h = height();

    // 1. Tech Container Background
    QRectF outerRect(2, 2, w - 4, h - 4);
    QLinearGradient bgGrad(0, 0, w, h);
    bgGrad.setColorAt(0.0, QColor(15, 23, 42, 240)); // Slate 900
    bgGrad.setColorAt(1.0, QColor(9, 14, 26, 250));
    p.setBrush(bgGrad);
    p.setPen(QPen(QColor(30, 41, 59, 200), 1.5));
    p.drawRoundedRect(outerRect, 14, 14);

    // Corner decorative industrial cyber brackets
    p.setPen(QPen(m_fanRunning ? QColor(56, 189, 248, 180) : QColor(71, 85, 105, 140), 1.5));
    p.drawLine(8, 6, 18, 6);
    p.drawLine(6, 8, 6, 18);
    p.drawLine(w - 18, 6, w - 8, 6);
    p.drawLine(w - 6, 8, w - 6, 18);
    p.drawLine(8, h - 6, 18, h - 6);
    p.drawLine(6, h - 18, 6, h - 8);
    p.drawLine(w - 18, h - 6, w - 8, h - 6);
    p.drawLine(w - 6, h - 18, w - 6, h - 8);

    // Center coordinates
    const double centerX = w / 2.0;
    const double centerY = h * 0.44;
    const double radius = qMin(w * 0.40, h * 0.38);

    // 2. Thermal Progress Gauge Arc (0°C to 60°C mapped to 240 degrees)
    const double startAngleDeg = 150.0;
    const double totalSpanDeg = 240.0;
    const double clampedTemp = qBound(0.0, m_temperatureC, 60.0);
    const double tempProgress = clampedTemp / 60.0;
    const double activeSpanDeg = tempProgress * totalSpanDeg;

    QRectF arcRect(centerX - radius, centerY - radius, radius * 2.0, radius * 2.0);

    // Track arc
    p.setPen(QPen(QColor(30, 41, 59, 180), 5.0, Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(arcRect, int(-startAngleDeg * 16), int(-totalSpanDeg * 16));

    // Active gradient arc
    QConicalGradient arcGrad(centerX, centerY, -startAngleDeg);
    arcGrad.setColorAt(0.0, QColor(56, 189, 248));   // < 25°C Cool blue
    arcGrad.setColorAt(0.4, QColor(16, 185, 129));   // 25-32°C Normal green
    arcGrad.setColorAt(0.65, QColor(245, 158, 11));  // 32-38°C Warning amber
    arcGrad.setColorAt(0.85, QColor(239, 68, 68));   // > 38°C Hot red
    arcGrad.setColorAt(1.0, QColor(220, 38, 38));

    p.setPen(QPen(QBrush(arcGrad), 5.0, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(arcRect, int(-startAngleDeg * 16), int(-activeSpanDeg * 16));

    // 3. Circular Turbine Housing & Rim
    const double fanRadius = radius * 0.78;
    QRectF fanHousingRect(centerX - fanRadius, centerY - fanRadius, fanRadius * 2.0, fanRadius * 2.0);
    QRadialGradient housingGrad(centerX, centerY, fanRadius);
    housingGrad.setColorAt(0.0, QColor(11, 19, 36, 230));
    housingGrad.setColorAt(0.82, QColor(22, 33, 56, 240));
    housingGrad.setColorAt(1.0, QColor(51, 65, 85, 255));
    p.setBrush(housingGrad);
    p.setPen(QPen(m_fanRunning ? QColor(6, 182, 212, 180) : QColor(71, 85, 105, 120), 1.5));
    p.drawEllipse(fanHousingRect);

    // Outer turbine vent tick marks
    for (int deg = 0; deg < 360; deg += 30) {
        const double r1 = fanRadius * 0.88;
        const double r2 = fanRadius * 0.96;
        const double rad = qDegreesToRadians(double(deg));
        p.setPen(QPen(m_fanRunning ? QColor(6, 182, 212, 120) : QColor(71, 85, 105, 100), 1.2));
        p.drawLine(QPointF(centerX + r1 * qCos(rad), centerY + r1 * qSin(rad)),
                   QPointF(centerX + r2 * qCos(rad), centerY + r2 * qSin(rad)));
    }

    // 4. Static 5-blade Turbine Impeller (Fixed orientation)
    p.save();
    p.translate(centerX, centerY);
    const double staticAngle = 18.0; // Clean, pleasing static angle
    p.rotate(staticAngle);

    const double bladeLen = fanRadius * 0.76;
    const double bladeW = fanRadius * 0.34;
    const int numBlades = 5;

    for (int i = 0; i < numBlades; ++i) {
        p.save();
        p.rotate(i * (360.0 / numBlades));

        QPainterPath bladePath;
        bladePath.moveTo(0, -fanRadius * 0.16);
        bladePath.cubicTo(bladeW * 0.9, -bladeLen * 0.35,
                          bladeW * 0.7, -bladeLen * 0.88,
                          0, -bladeLen);
        bladePath.cubicTo(-bladeW * 0.3, -bladeLen * 0.8,
                          -bladeW * 0.2, -bladeLen * 0.3,
                          0, -fanRadius * 0.16);

        QLinearGradient bladeGrad(0, 0, bladeW, -bladeLen);
        if (m_fanRunning) {
            bladeGrad.setColorAt(0.0, QColor(34, 211, 238, 255)); // Active Cyan / Neon
            bladeGrad.setColorAt(0.6, QColor(6, 182, 212, 240));
            bladeGrad.setColorAt(1.0, QColor(2, 132, 199, 220));
        } else {
            bladeGrad.setColorAt(0.0, QColor(71, 85, 105, 180));  // Idle Slate
            bladeGrad.setColorAt(0.7, QColor(51, 65, 85, 160));
            bladeGrad.setColorAt(1.0, QColor(30, 41, 59, 140));
        }

        p.setBrush(bladeGrad);
        p.setPen(QPen(m_fanRunning ? QColor(56, 189, 248, 200) : QColor(100, 116, 139, 100), 1.0));
        p.drawPath(bladePath);
        p.restore();
    }

    // Metallic center hub
    const double hubRadius = fanRadius * 0.26;
    QRadialGradient hubGrad(0, 0, hubRadius);
    hubGrad.setColorAt(0.0, QColor(241, 245, 249));
    hubGrad.setColorAt(0.5, QColor(100, 116, 139));
    hubGrad.setColorAt(1.0, QColor(30, 41, 59));
    p.setBrush(hubGrad);
    p.setPen(QPen(QColor(15, 23, 42), 1.2));
    p.drawEllipse(QRectF(-hubRadius, -hubRadius, hubRadius * 2.0, hubRadius * 2.0));

    // Center indicator core
    p.setBrush(m_fanRunning ? QColor(6, 182, 212) : QColor(100, 116, 139));
    p.setPen(Qt::NoPen);
    p.drawEllipse(QRectF(-hubRadius * 0.45, -hubRadius * 0.45, hubRadius * 0.9, hubRadius * 0.9));

    p.restore();

    // 5. Status Pill Badge at bottom (Illustrating ON/OFF clearly)
    const QColor col = statusColor();
    const double pillW = qMin(w - 20.0, 170.0);
    const double pillH = 20.0;
    const double pillX = (w - pillW) / 2.0;
    const double pillY = h - pillH - 7.0;

    QRectF pillRect(pillX, pillY, pillW, pillH);
    p.setBrush(QColor(col.red(), col.green(), col.blue(), m_fanRunning ? 40 : 25));
    p.setPen(QPen(QColor(col.red(), col.green(), col.blue(), m_fanRunning ? 180 : 100), 1.2));
    p.drawRoundedRect(pillRect, 10, 10);

    QFont pillFont;
    pillFont.setBold(true);
    pillFont.setPixelSize(10);
    p.setFont(pillFont);
    p.setPen(col);
    p.drawText(pillRect, Qt::AlignCenter, statusText());
}

