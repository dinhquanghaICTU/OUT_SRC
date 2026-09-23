#include "DashboardPage.h"
#include "VirtualKeyboard.h"
#include "ui_DashboardPage.h"

#include <QCheckBox>
#include <QDateTime>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QtMath>

// ============================================================================
// 1. LuxGaugeWidget (Semi-circular Arc Lux Gauge)
// ============================================================================
LuxGaugeWidget::LuxGaugeWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(140, 110);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void LuxGaugeWidget::setValue(double val, double maxVal)
{
    m_value = qMax(0.0, val);
    m_max = (maxVal > 0.0) ? maxVal : 1000.0;
    update();
}

void LuxGaugeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();
    const int cx = w / 2;
    const int cy = h - 18;
    const int radius = 50;
    const int thickness = 9;

    QRectF arcRect(cx - radius, cy - radius, radius * 2, radius * 2);

    // Track arc (start 210 deg, span 240 deg clockwise in standard polar)
    // In Qt drawArc: angles in 1/16th of a degree, 0 is at 3 o'clock, counter-clockwise
    // Span from 210 deg (bottom-left) to -30 deg (bottom-right) = -240 deg
    const int startAngle = 210 * 16;
    const int spanAngle = -240 * 16;

    QPen trackPen(QColor(QStringLiteral("#1a2744")), thickness, Qt::SolidLine, Qt::RoundCap);
    p.setPen(trackPen);
    p.setBrush(Qt::NoBrush);
    p.drawArc(arcRect, startAngle, spanAngle);

    // Active arc
    const double ratio = qBound(0.0, m_value / m_max, 1.0);
    const int activeSpan = static_cast<int>(spanAngle * ratio);

    if (qAbs(activeSpan) > 0) {
        QPen activePen(QColor(QStringLiteral("#f59e0b")), thickness, Qt::SolidLine, Qt::RoundCap);
        if (m_value < 50.0) {
            activePen.setColor(QColor(QStringLiteral("#fbbf24"))); // warm amber
        } else if (m_value <= 500.0) {
            activePen.setColor(QColor(QStringLiteral("#10b981"))); // emerald
        } else {
            activePen.setColor(QColor(QStringLiteral("#38bdf8"))); // bright cyan
        }
        p.setPen(activePen);
        p.drawArc(arcRect, startAngle, activeSpan);
    }

    // Numerical readout inside arc
    p.setPen(QColor(QStringLiteral("#ffffff")));
    QFont valFont(QStringLiteral("sans-serif"), 14, QFont::Bold);
    p.setFont(valFont);
    const QString valStr = QString::number(m_value, 'f', 1);
    QRect textRect(cx - 55, cy - 36, 110, 24);
    p.drawText(textRect, Qt::AlignCenter, valStr);

    p.setPen(QColor(QStringLiteral("#94a3b8")));
    QFont unitFont(QStringLiteral("sans-serif"), 8, QFont::Bold);
    p.setFont(unitFont);
    QRect unitRect(cx - 40, cy - 14, 80, 16);
    p.drawText(unitRect, Qt::AlignCenter, QStringLiteral("LUX"));
}

// ============================================================================
// 2. LuxWaveformWidget (Realtime Rolling Lux Line Chart)
// ============================================================================
LuxWaveformWidget::LuxWaveformWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(52);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    for (int i = 0; i < 24; ++i)
        m_samples.append(100.0 + 20.0 * qSin(i * 0.4));
}

void LuxWaveformWidget::addSample(double val)
{
    m_samples.append(qMax(0.0, val));
    if (m_samples.size() > 24)
        m_samples.removeFirst();
    update();
}

void LuxWaveformWidget::clear()
{
    m_samples.clear();
    update();
}

void LuxWaveformWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();

    // Background container
    p.setPen(QPen(QColor(QStringLiteral("#16233f")), 1));
    p.setBrush(QColor(QStringLiteral("#091124")));
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 6, 6);

    if (m_samples.size() < 2)
        return;

    double maxVal = 100.0;
    double minVal = 0.0;
    for (double s : m_samples) {
        if (s > maxVal) maxVal = s;
    }
    maxVal = maxVal * 1.15;

    const int padX = 6;
    const int padY = 6;
    const double plotW = w - padX * 2;
    const double plotH = h - padY * 2;
    const double stepX = plotW / (m_samples.size() - 1);

    QPainterPath path;
    QPainterPath fillPath;

    for (int i = 0; i < m_samples.size(); ++i) {
        const double x = padX + i * stepX;
        const double ratio = (maxVal > minVal) ? (m_samples[i] - minVal) / (maxVal - minVal) : 0.5;
        const double y = (h - padY) - ratio * plotH;

        if (i == 0) {
            path.moveTo(x, y);
            fillPath.moveTo(x, h - padY);
            fillPath.lineTo(x, y);
        } else {
            path.lineTo(x, y);
            fillPath.lineTo(x, y);
        }
    }

    fillPath.lineTo(padX + (m_samples.size() - 1) * stepX, h - padY);
    fillPath.closeSubpath();

    // Fill area gradient
    QLinearGradient grad(0, 0, 0, h);
    grad.setColorAt(0.0, QColor(245, 158, 11, 80));
    grad.setColorAt(1.0, QColor(245, 158, 11, 0));
    p.fillPath(fillPath, grad);

    // Draw line
    p.setPen(QPen(QColor(QStringLiteral("#fbbf24")), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPath(path);

    // Latest point indicator
    const double lastX = padX + (m_samples.size() - 1) * stepX;
    const double lastRatio = (maxVal > minVal) ? (m_samples.last() - minVal) / (maxVal - minVal) : 0.5;
    const double lastY = (h - padY) - lastRatio * plotH;
    p.setBrush(QColor(QStringLiteral("#ffffff")));
    p.setPen(QPen(QColor(QStringLiteral("#f59e0b")), 2));
    p.drawEllipse(QPointF(lastX, lastY), 3.5, 3.5);
}

// ============================================================================
// 3. LightBulbWidget (Vector Antialiased Lamp Widget)
// ============================================================================
LightBulbWidget::LightBulbWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(88);
    setMinimumWidth(120);
    setCursor(Qt::PointingHandCursor);
}

void LightBulbWidget::setState(bool isOn)
{
    if (m_isOn != isOn) {
        m_isOn = isOn;
        update();
    }
}

void LightBulbWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QWidget::mousePressEvent(event);
}

void LightBulbWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const double cx = width() / 2.0;
    const double cy = 34.0;
    const double r = 22.0;

    // Classic Pear-Shaped Light Bulb Path
    QPainterPath bulbPath;
    bulbPath.moveTo(cx - 8.5, 57.0);
    bulbPath.cubicTo(cx - 11.5, 48.0, cx - r, 42.0, cx - r, cy);
    bulbPath.cubicTo(cx - r, cy - 12.5, cx - 12.5, cy - r, cx, cy - r);
    bulbPath.cubicTo(cx + 12.5, cy - r, cx + r, cy - 12.5, cx + r, cy);
    bulbPath.cubicTo(cx + r, 42.0, cx + 11.5, 48.0, cx + 8.5, 57.0);
    bulbPath.closeSubpath();

    if (m_isOn) {
        // ====================================================================
        // STATE: BẬT (ON) - RADIANT GLOWING GOLDEN BULB
        // ====================================================================

        // 1. Outermost soft ambient aura
        QRadialGradient ambientAura(cx, cy, 54);
        ambientAura.setColorAt(0.0, QColor(254, 240, 138, 170));
        ambientAura.setColorAt(0.35, QColor(245, 158, 11, 90));
        ambientAura.setColorAt(0.70, QColor(217, 119, 6, 25));
        ambientAura.setColorAt(1.0, QColor(217, 119, 6, 0));
        p.setPen(Qt::NoPen);
        p.setBrush(ambientAura);
        p.drawEllipse(QPointF(cx, cy), 54, 54);

        // 2. Radiant light rays beaming outward (10 rays)
        QPen rayPen(QColor(253, 224, 71, 230), 2.2, Qt::SolidLine, Qt::RoundCap);
        p.setPen(rayPen);
        const double rayAngles[] = { -150, -125, -100, -80, -55, -30, 0, 180, 25, 155 };
        for (double deg : rayAngles) {
            const double rad = deg * M_PI / 180.0;
            const double x1 = cx + (r + 4.0) * cos(rad);
            const double y1 = cy + (r + 4.0) * sin(rad);
            const double x2 = cx + (r + 14.0) * cos(rad);
            const double y2 = cy + (r + 14.0) * sin(rad);
            p.drawLine(QPointF(x1, y1), QPointF(x2, y2));
        }

        // 3. Glowing Pear-Shaped Glass Body
        QRadialGradient bulbGrad(cx - 3, cy - 4, r + 6);
        bulbGrad.setColorAt(0.0, QColor(255, 255, 255));
        bulbGrad.setColorAt(0.28, QColor(254, 249, 195));
        bulbGrad.setColorAt(0.65, QColor(250, 204, 21));
        bulbGrad.setColorAt(0.92, QColor(245, 158, 11));
        bulbGrad.setColorAt(1.0, QColor(217, 119, 6));
        p.setPen(QPen(QColor(254, 240, 138), 2.0));
        p.setBrush(bulbGrad);
        p.drawPath(bulbPath);

        // 4. White-Hot Incandescent Filament
        // Filament Support Wires
        p.setPen(QPen(QColor(254, 240, 138), 1.5, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(cx - 3.5, 55.0), QPointF(cx - 5.0, 36.0));
        p.drawLine(QPointF(cx + 3.5, 55.0), QPointF(cx + 5.0, 36.0));

        // Center Looped Glowing Filament Coil
        QPainterPath filPath;
        filPath.moveTo(cx - 5.0, 36.0);
        filPath.cubicTo(cx - 5.0, 25.0, cx - 2.0, 24.0, cx, 28.0);
        filPath.cubicTo(cx + 2.0, 24.0, cx + 5.0, 25.0, cx + 5.0, 36.0);

        // Filament outer halo
        p.setPen(QPen(QColor(254, 240, 138, 200), 3.8, Qt::SolidLine, Qt::RoundCap));
        p.drawPath(filPath);
        // Filament white core
        p.setPen(QPen(QColor(255, 255, 255), 2.0, Qt::SolidLine, Qt::RoundCap));
        p.drawPath(filPath);

        // 5. Specular Gloss Reflection Arc (Top-Left)
        QPainterPath glossPath;
        glossPath.arcMoveTo(QRectF(cx - 17, cy - 17, 34, 34), 125);
        glossPath.arcTo(QRectF(cx - 17, cy - 17, 34, 34), 125, 42);
        p.setPen(QPen(QColor(255, 255, 255, 190), 2.4, Qt::SolidLine, Qt::RoundCap));
        p.drawPath(glossPath);

        // 6. Brass Threaded Screw Base (E27)
        QLinearGradient baseGrad(cx - 8.5, 0, cx + 8.5, 0);
        baseGrad.setColorAt(0.0, QColor(146, 64, 14));
        baseGrad.setColorAt(0.3, QColor(245, 158, 11));
        baseGrad.setColorAt(0.6, QColor(254, 240, 138));
        baseGrad.setColorAt(1.0, QColor(180, 83, 9));

        p.setPen(QPen(QColor(180, 83, 9), 1.0));
        p.setBrush(baseGrad);
        p.drawRoundedRect(QRectF(cx - 8.5, 57.0, 17.0, 4.0), 1.5, 1.5);
        p.drawRoundedRect(QRectF(cx - 8.5, 61.0, 17.0, 4.0), 1.5, 1.5);
        p.drawRoundedRect(QRectF(cx - 7.5, 65.0, 15.0, 4.0), 1.5, 1.5);

        // Bottom Contact Point
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(120, 53, 15));
        p.drawRoundedRect(QRectF(cx - 4.5, 69.0, 9.0, 2.5), 1.0, 1.0);

    } else {
        // ====================================================================
        // STATE: TẮT (OFF) - CRISP HIGH-CONTRAST CLEAR GLASS BULB
        // ====================================================================

        // 1. Smoky Translucent Crystal Glass Body (High contrast against dark card)
        QLinearGradient glassGrad(cx - r, cy - r, cx + r, 57.0);
        glassGrad.setColorAt(0.0, QColor(51, 65, 85, 220));   // Translucent slate
        glassGrad.setColorAt(0.5, QColor(30, 41, 59, 235));
        glassGrad.setColorAt(1.0, QColor(15, 23, 42, 245));

        // Distinct crisp light-slate glass border (stands out clearly)
        p.setPen(QPen(QColor(148, 163, 184, 230), 2.0));
        p.setBrush(glassGrad);
        p.drawPath(bulbPath);

        // Inner cyan refraction ring for realistic glass look
        p.setPen(QPen(QColor(56, 189, 248, 40), 1.0));
        p.setBrush(Qt::NoBrush);
        p.drawPath(bulbPath);

        // 2. Glass Internal Stem Mount
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(71, 85, 105, 190));
        p.drawRoundedRect(QRectF(cx - 2.5, 47.0, 5.0, 10.0), 1.0, 1.0);

        // 3. Support Posts (Hai thanh đỡ dây tóc vonfram)
        p.setPen(QPen(QColor(148, 163, 184, 220), 1.6, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(cx - 3.5, 48.0), QPointF(cx - 5.0, 36.0));
        p.drawLine(QPointF(cx + 3.5, 48.0), QPointF(cx + 5.0, 36.0));

        // 4. Tungsten Filament Loop (Dây tóc vonfram nguội)
        QPainterPath filPath;
        filPath.moveTo(cx - 5.0, 36.0);
        filPath.cubicTo(cx - 5.0, 25.0, cx - 2.0, 24.0, cx, 28.0);
        filPath.cubicTo(cx + 2.0, 24.0, cx + 5.0, 25.0, cx + 5.0, 36.0);
        p.setPen(QPen(QColor(203, 213, 225, 240), 2.0, Qt::SolidLine, Qt::RoundCap));
        p.drawPath(filPath);

        // 5. Glossy Glass Specular Highlights (Tạo độ bóng thủy tinh)
        // Primary Top-Left Highlight
        QPainterPath glossPath;
        glossPath.arcMoveTo(QRectF(cx - 17, cy - 17, 34, 34), 125);
        glossPath.arcTo(QRectF(cx - 17, cy - 17, 34, 34), 125, 42);
        p.setPen(QPen(QColor(255, 255, 255, 175), 2.4, Qt::SolidLine, Qt::RoundCap));
        p.drawPath(glossPath);

        // Secondary subtle rim light
        QPainterPath secGloss;
        secGloss.arcMoveTo(QRectF(cx - 17, cy - 17, 34, 34), -45);
        secGloss.arcTo(QRectF(cx - 17, cy - 17, 34, 34), -45, 28);
        p.setPen(QPen(QColor(255, 255, 255, 65), 1.5, Qt::SolidLine, Qt::RoundCap));
        p.drawPath(secGloss);

        // 6. Silver Metallic Screw Base (Đui ren mạ kẽm E27)
        QLinearGradient baseGrad(cx - 8.5, 0, cx + 8.5, 0);
        baseGrad.setColorAt(0.0, QColor(51, 65, 85));
        baseGrad.setColorAt(0.3, QColor(100, 116, 139));
        baseGrad.setColorAt(0.6, QColor(203, 213, 225));
        baseGrad.setColorAt(1.0, QColor(71, 85, 105));

        p.setPen(QPen(QColor(71, 85, 105), 1.0));
        p.setBrush(baseGrad);
        p.drawRoundedRect(QRectF(cx - 8.5, 57.0, 17.0, 4.0), 1.5, 1.5);
        p.drawRoundedRect(QRectF(cx - 8.5, 61.0, 17.0, 4.0), 1.5, 1.5);
        p.drawRoundedRect(QRectF(cx - 7.5, 65.0, 15.0, 4.0), 1.5, 1.5);

        // Bottom Electrical Foot Contact
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(180, 83, 9));
        p.drawRoundedRect(QRectF(cx - 4.5, 69.0, 9.0, 2.5), 1.0, 1.0);
    }
}

// ============================================================================
// 4. PirMotionWidget (Vector Antialiased PIR Infrared Sensor & Presence Radar)
// ============================================================================
PirMotionWidget::PirMotionWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(88);
    setMinimumWidth(120);
}

void PirMotionWidget::setMotion(bool hasMotion)
{
    if (m_hasMotion != hasMotion) {
        m_hasMotion = hasMotion;
        update();
    }
}

void PirMotionWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const double cx = width() / 2.0;
    const double cy = 42.0;
    const double sensorX = cx - 42.0;
    const double manX = cx + 38.0;

    if (m_hasMotion) {
        // ====================================================================
        // STATE: CÓ CHUYỂN ĐỘNG (MOTION DETECTED) - ACTIVE INFRARED THERMAL SCAN
        // ====================================================================

        // 1. Infrared Detection Wave Field / Thermal Cone
        QRadialGradient coneGrad(sensorX + 4.0, cy, 62.0);
        coneGrad.setColorAt(0.0, QColor(16, 185, 129, 70));
        coneGrad.setColorAt(0.5, QColor(16, 185, 129, 30));
        coneGrad.setColorAt(1.0, QColor(16, 185, 129, 0));
        p.setPen(Qt::NoPen);
        p.setBrush(coneGrad);
        p.drawPie(QRectF(sensorX + 4.0 - 62.0, cy - 62.0, 124.0, 124.0), -38 * 16, 76 * 16);

        // 2. Concentric Infrared Wave Arcs radiating toward human
        struct WaveArc { double r; int span; QColor color; double width; };
        const WaveArc waves[] = {
            { 18.0, 76, QColor(16, 185, 129, 240), 2.8 },
            { 32.0, 68, QColor(52, 211, 153, 210), 2.4 },
            { 48.0, 58, QColor(110, 231, 183, 170), 2.0 }
        };
        for (const auto &w : waves) {
            p.setPen(QPen(w.color, w.width, Qt::SolidLine, Qt::RoundCap));
            p.drawArc(QRectF(sensorX + 4.0 - w.r, cy - w.r, w.r * 2.0, w.r * 2.0),
                      -(w.span / 2) * 16, w.span * 16);
        }

        // 3. Human Figure Thermal Aura (Phát quang thân nhiệt hồng ngoại)
        QRadialGradient heatAura(manX, cy, 32.0);
        heatAura.setColorAt(0.0, QColor(52, 211, 153, 120));
        heatAura.setColorAt(0.5, QColor(16, 185, 129, 50));
        heatAura.setColorAt(1.0, QColor(16, 185, 129, 0));
        p.setPen(Qt::NoPen);
        p.setBrush(heatAura);
        p.drawEllipse(QPointF(manX, cy), 32.0, 32.0);

        // 4. Human Body Graphic (Moving Person Silhouette)
        QLinearGradient bodyGrad(manX, cy - 24.0, manX, cy + 24.0);
        bodyGrad.setColorAt(0.0, QColor(255, 255, 255));
        bodyGrad.setColorAt(0.3, QColor(110, 231, 183));
        bodyGrad.setColorAt(1.0, QColor(16, 185, 129));

        // Head
        p.setBrush(bodyGrad);
        p.setPen(QPen(QColor(16, 185, 129), 1.2));
        p.drawEllipse(QPointF(manX, cy - 19.0), 5.5, 5.5);

        // Torso
        QPainterPath torso;
        torso.moveTo(manX - 5.0, cy - 12.0);
        torso.lineTo(manX + 5.0, cy - 12.0);
        torso.lineTo(manX + 3.5, cy + 2.0);
        torso.lineTo(manX - 3.5, cy + 2.0);
        torso.closeSubpath();
        p.fillPath(torso, bodyGrad);

        // Limbs (Active walking posture)
        p.setPen(QPen(QBrush(bodyGrad), 2.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        // Left arm forward
        p.drawLine(QPointF(manX - 5.0, cy - 11.0), QPointF(manX - 11.0, cy - 2.0));
        p.drawLine(QPointF(manX - 11.0, cy - 2.0), QPointF(manX - 7.0, cy + 6.0));
        // Right arm backward
        p.drawLine(QPointF(manX + 5.0, cy - 11.0), QPointF(manX + 11.0, cy - 3.0));
        // Left leg stepping forward
        p.drawLine(QPointF(manX - 2.5, cy + 2.0), QPointF(manX - 8.0, cy + 13.0));
        p.drawLine(QPointF(manX - 8.0, cy + 13.0), QPointF(manX - 11.0, cy + 23.0));
        // Right leg back
        p.drawLine(QPointF(manX + 2.5, cy + 2.0), QPointF(manX + 6.0, cy + 12.0));
        p.drawLine(QPointF(manX + 6.0, cy + 12.0), QPointF(manX + 9.0, cy + 22.0));

        // 5. PIR Sensor Lens Body (Fresnel Lens + PCB)
        // PCB Base
        p.setPen(QPen(QColor(5, 150, 105), 1.0));
        p.setBrush(QColor(6, 78, 59));
        p.drawRoundedRect(QRectF(sensorX - 12.0, cy - 17.0, 11.0, 34.0), 2.5, 2.5);

        // Active Red Sensor Indicator LED
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(239, 68, 68));
        p.drawEllipse(QPointF(sensorX - 6.5, cy), 2.5, 2.5);
        // Red glow aura
        QRadialGradient ledGlow(sensorX - 6.5, cy, 6.0);
        ledGlow.setColorAt(0.0, QColor(239, 68, 68, 160));
        ledGlow.setColorAt(1.0, QColor(239, 68, 68, 0));
        p.setBrush(ledGlow);
        p.drawEllipse(QPointF(sensorX - 6.5, cy), 6.0, 6.0);

        // Fresnel Dome Lens (Hemisphere)
        QPainterPath dome;
        dome.moveTo(sensorX, cy - 14.0);
        dome.cubicTo(sensorX + 15.0, cy - 14.0, sensorX + 15.0, cy + 14.0, sensorX, cy + 14.0);
        dome.closeSubpath();

        QRadialGradient domeGrad(sensorX + 4.0, cy, 14.0);
        domeGrad.setColorAt(0.0, QColor(236, 253, 245));
        domeGrad.setColorAt(0.5, QColor(110, 231, 183));
        domeGrad.setColorAt(1.0, QColor(16, 185, 129));
        p.setPen(QPen(QColor(52, 211, 153), 1.8));
        p.setBrush(domeGrad);
        p.drawPath(dome);

        // Optical facet grid lines
        p.setPen(QPen(QColor(5, 150, 105, 120), 1.0));
        p.drawLine(QPointF(sensorX + 5.0, cy - 10.0), QPointF(sensorX + 5.0, cy + 10.0));
        p.drawLine(QPointF(sensorX + 9.0, cy - 7.0), QPointF(sensorX + 9.0, cy + 7.0));

    } else {
        // ====================================================================
        // STATE: PHÒNG TRỐNG (NO MOTION) - IDLE STANDBY RADAR
        // ====================================================================

        // 1. Standby Radar Scan Cone (Subtle dotted/dashed guides)
        p.setPen(QPen(QColor(51, 65, 85, 130), 1.0, Qt::DashLine));
        p.drawLine(QPointF(sensorX + 4.0, cy), QPointF(sensorX + 56.0, cy - 32.0));
        p.drawLine(QPointF(sensorX + 4.0, cy), QPointF(sensorX + 56.0, cy + 32.0));

        // 2. Standby Radar Wave Arcs (Faint dashed slate)
        const double idleWaves[] = { 18.0, 32.0, 48.0 };
        for (double r : idleWaves) {
            p.setPen(QPen(QColor(71, 85, 105, 110), 1.2, Qt::DashLine, Qt::RoundCap));
            p.drawArc(QRectF(sensorX + 4.0 - r, cy - r, r * 2.0, r * 2.0), -32 * 16, 64 * 16);
        }

        // 3. Ghost / Empty Silhouette (No human present)
        // Dotted head outline
        p.setPen(QPen(QColor(71, 85, 105, 150), 1.5, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(manX, cy - 19.0), 5.5, 5.5);

        // Dotted body
        QPainterPath torso;
        torso.moveTo(manX - 5.0, cy - 12.0);
        torso.lineTo(manX + 5.0, cy - 12.0);
        torso.lineTo(manX + 3.5, cy + 2.0);
        torso.lineTo(manX - 3.5, cy + 2.0);
        torso.closeSubpath();
        p.drawPath(torso);

        // Standing idle legs & arms (calm dashed)
        p.drawLine(QPointF(manX - 5.0, cy - 11.0), QPointF(manX - 8.0, cy + 3.0));
        p.drawLine(QPointF(manX + 5.0, cy - 11.0), QPointF(manX + 8.0, cy + 3.0));
        p.drawLine(QPointF(manX - 2.5, cy + 2.0), QPointF(manX - 3.0, cy + 22.0));
        p.drawLine(QPointF(manX + 2.5, cy + 2.0), QPointF(manX + 3.0, cy + 22.0));

        // 4. Idle PIR Sensor Lens Body
        // PCB Base
        p.setPen(QPen(QColor(51, 65, 85), 1.0));
        p.setBrush(QColor(15, 23, 42));
        p.drawRoundedRect(QRectF(sensorX - 12.0, cy - 17.0, 11.0, 34.0), 2.5, 2.5);

        // Dark Standby LED
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(51, 65, 85));
        p.drawEllipse(QPointF(sensorX - 6.5, cy), 2.0, 2.0);

        // Fresnel Dome Lens (Cool Frosted Smoky Slate)
        QPainterPath dome;
        dome.moveTo(sensorX, cy - 14.0);
        dome.cubicTo(sensorX + 15.0, cy - 14.0, sensorX + 15.0, cy + 14.0, sensorX, cy + 14.0);
        dome.closeSubpath();

        QLinearGradient domeGrad(sensorX, cy - 14.0, sensorX + 15.0, cy + 14.0);
        domeGrad.setColorAt(0.0, QColor(71, 85, 105));
        domeGrad.setColorAt(1.0, QColor(30, 41, 59));
        p.setPen(QPen(QColor(148, 163, 184, 180), 1.5));
        p.setBrush(domeGrad);
        p.drawPath(dome);

        // Subtle specular highlight on lens
        p.setPen(QPen(QColor(255, 255, 255, 70), 1.2, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(sensorX + 3.0, cy - 10.0), QPointF(sensorX + 9.0, cy - 4.0));
    }
}

// ============================================================================
// 5. DashboardPage Implementation
// ============================================================================
DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::DashboardPage),
      m_relayPendingTimer(new QTimer(this))
{
    ui->setupUi(this);
    setObjectName(QStringLiteral("DashboardPage"));
    setAttribute(Qt::WA_StyledBackground, true);

    m_relayPendingTimer->setSingleShot(true);
    connect(m_relayPendingTimer, &QTimer::timeout, this, [this] {
        m_isRelayPending = false;
        updateUiState();
    });

    QSettings settings(QStringLiteral("ICTU"), QStringLiteral("TuanAnhSmartLight"));
    m_autoModeActive = settings.value(QStringLiteral("auto_mode"), true).toBool();

    setupUi();
    updateUiState();
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

void DashboardPage::setUsername(const QString &username)
{
    m_username = username;
}

void DashboardPage::setupUi()
{
    while (QLayoutItem *item = ui->verticalLayout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    setStyleSheet(
        "QWidget#DashboardPage { background-color: #070d1e; color: #ffffff; font-family: sans-serif; } "
        "QFrame.metricCard { background-color: #0c152d; border: 1.5px solid #1a2744; border-radius: 10px; } "
        "QFrame.metricCard:hover { border-color: #2b3d6a; }"
    );

    ui->verticalLayout->setContentsMargins(10, 8, 10, 8);
    ui->verticalLayout->setSpacing(8);

    // --- 1. Top Header Sub-Bar ---
    auto *topBar = new QHBoxLayout;
    topBar->setContentsMargins(2, 0, 2, 0);

    auto *headerTitleCol = new QVBoxLayout;
    headerTitleCol->setSpacing(1);
    auto *sysTitle = new QLabel(tr("HỆ THỐNG CHIẾU SÁNG & HIỆN DIỆN PHÒNG THÔNG MINH"), this);
    sysTitle->setStyleSheet("color: #38bdf8; font-size: 13px; font-weight: 900; letter-spacing: 0.5px;");
    auto *sysSub = new QLabel(tr("Cảm biến quang học BH1750 • Hiện diện thân nhiệt PIR • Điều khiển đèn tự động"), this);
    sysSub->setStyleSheet("color: #64748b; font-size: 10px; font-weight: 600;");
    headerTitleCol->addWidget(sysTitle);
    headerTitleCol->addWidget(sysSub);
    topBar->addLayout(headerTitleCol);

    topBar->addStretch();

    m_nodeStatusLabel = new QLabel(tr("● Trạm ESP32: 150808 (Online)"), this);
    m_nodeStatusLabel->setStyleSheet(
        "background-color: #0d281e; color: #34d399; border: 1px solid #065f46; border-radius: 12px; padding: 4px 10px; font-size: 10px; font-weight: 800;");
    topBar->addWidget(m_nodeStatusLabel);

    ui->verticalLayout->addLayout(topBar);

    // --- 2. Main 3-Column Grid Layout (800x480 Optimized) ---
    auto *gridRow = new QHBoxLayout;
    gridRow->setSpacing(10);
    gridRow->setContentsMargins(0, 0, 0, 0);

    // ========================================================================
    // COLUMN 1 (33%): CẢM BIẾN ÁNH SÁNG BH1750
    // ========================================================================
    auto *col1Frame = new QFrame(this);
    col1Frame->setProperty("class", QStringLiteral("metricCard"));
    col1Frame->setCursor(Qt::PointingHandCursor);
    auto *col1Layout = new QVBoxLayout(col1Frame);
    col1Layout->setContentsMargins(12, 10, 12, 10);
    col1Layout->setSpacing(8);

    auto *col1Title = new QLabel(tr("ĐỘ RỌI ÁNH SÁNG (BH1750)"), col1Frame);
    col1Title->setStyleSheet("color: #fbbf24; font-size: 11px; font-weight: 900; letter-spacing: 0.3px;");
    col1Layout->addWidget(col1Title, 0, Qt::AlignCenter);

    m_gaugeWidget = new LuxGaugeWidget(col1Frame);
    col1Layout->addWidget(m_gaugeWidget, 0, Qt::AlignCenter);

    m_luxBadgeLabel = new QLabel(tr("ĐỦ SÁNG (TIÊU CHUẨN)"), col1Frame);
    m_luxBadgeLabel->setAlignment(Qt::AlignCenter);
    m_luxBadgeLabel->setStyleSheet(
        "background-color: #064e3b; color: #34d399; border: 1px solid #059669; border-radius: 5px; padding: 4px 8px; font-size: 10px; font-weight: 800;");
    col1Layout->addWidget(m_luxBadgeLabel);

    m_thresholdLabel = new QLabel(tr("Ngưỡng bật: 50 Lux • Tắt: 500 Lux"), col1Frame);
    m_thresholdLabel->setAlignment(Qt::AlignCenter);
    m_thresholdLabel->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: 600;");
    col1Layout->addWidget(m_thresholdLabel);

    col1Layout->addStretch();
    gridRow->addWidget(col1Frame, 33);

    // Click on col1 card opens config dialog
    struct CardClickFilter : public QObject {
        std::function<void()> onClick;
        CardClickFilter(QObject *parent, std::function<void()> cb) : QObject(parent), onClick(cb) {}
        bool eventFilter(QObject *w, QEvent *e) override {
            if (e->type() == QEvent::MouseButtonRelease) {
                if (onClick) onClick();
                return true;
            }
            return QObject::eventFilter(w, e);
        }
    };
    col1Frame->installEventFilter(new CardClickFilter(col1Frame, [this] { openConfigDialog(); }));

    // ========================================================================
    // COLUMN 2 (34%): ĐIỀU KHIỂN CHIẾU SÁNG & CHẾ ĐỘ THÔNG MINH
    // ========================================================================
    auto *col2Frame = new QFrame(this);
    col2Frame->setProperty("class", QStringLiteral("metricCard"));
    auto *col2Layout = new QVBoxLayout(col2Frame);
    col2Layout->setContentsMargins(12, 10, 12, 10);
    col2Layout->setSpacing(8);

    auto *col2Title = new QLabel(tr("ĐIỀU KHIỂN ĐÈN PHÒNG"), col2Frame);
    col2Title->setStyleSheet("color: #38bdf8; font-size: 11px; font-weight: 900; letter-spacing: 0.3px;");
    col2Layout->addWidget(col2Title, 0, Qt::AlignCenter);

    // Bulb state visual box with vector LightBulbWidget
    auto *bulbBox = new QFrame(col2Frame);
    bulbBox->setStyleSheet("background-color: #081022; border: 1px solid #162438; border-radius: 8px;");
    bulbBox->setToolTip(tr("Bấm vào bóng đèn để Bật / Tắt đèn phòng"));
    auto *bulbBoxLayout = new QVBoxLayout(bulbBox);
    bulbBoxLayout->setContentsMargins(8, 6, 8, 8);
    bulbBoxLayout->setSpacing(4);

    m_bulbWidget = new LightBulbWidget(bulbBox);
    m_bulbWidget->setToolTip(tr("Bấm vào bóng đèn để Bật / Tắt đèn phòng"));
    bulbBoxLayout->addWidget(m_bulbWidget, 0, Qt::AlignCenter);
    connect(m_bulbWidget, &LightBulbWidget::clicked, this, [this] {
        if (m_relayToggleBtn) m_relayToggleBtn->click();
    });

    m_lampStateLabel = new QLabel(tr("ĐÈN CHIẾU SÁNG: ĐANG TẮT"), bulbBox);
    m_lampStateLabel->setAlignment(Qt::AlignCenter);
    m_lampStateLabel->setStyleSheet(
        "background-color: rgba(51, 65, 85, 0.35); color: #94a3b8; border: 1px solid #475569; border-radius: 5px; padding: 3px 8px; font-size: 10px; font-weight: 800;");
    bulbBoxLayout->addWidget(m_lampStateLabel);

    col2Layout->addWidget(bulbBox);

    // Relay Toggle Button
    m_relayToggleBtn = new QPushButton(tr("BẬT ĐÈN PHÒNG"), col2Frame);
    m_relayToggleBtn->setCursor(Qt::PointingHandCursor);
    m_relayToggleBtn->setFixedHeight(36);
    m_relayToggleBtn->setStyleSheet(
        "QPushButton { background: #10b981; color: #ffffff; border: none; border-radius: 6px; font-size: 12px; font-weight: 900; } "
        "QPushButton:hover { background: #059669; }");
    connect(m_relayToggleBtn, &QPushButton::clicked, this, [this] {
        if (!m_deviceId.isEmpty()) {
            m_isRelayPending = true;
            m_pendingRelayState = !m_relayState;
            m_relayPendingTimer->start(3500);
            updateUiState();
            emit relayControlRequested(m_deviceId, m_pendingRelayState);
        }
    });
    col2Layout->addWidget(m_relayToggleBtn);

    // Mode Selector: Auto vs Manual
    auto *modeRow = new QHBoxLayout;
    modeRow->setSpacing(6);

    m_autoModeBtn = new QPushButton(tr("Tự Động"), col2Frame);
    m_autoModeBtn->setCursor(Qt::PointingHandCursor);
    m_autoModeBtn->setFixedHeight(28);

    m_manualModeBtn = new QPushButton(tr("Thủ Công"), col2Frame);
    m_manualModeBtn->setCursor(Qt::PointingHandCursor);
    m_manualModeBtn->setFixedHeight(28);

    connect(m_autoModeBtn, &QPushButton::clicked, this, [this] {
        m_autoModeActive = true;
        QSettings s(QStringLiteral("ICTU"), QStringLiteral("TuanAnhSmartLight"));
        s.setValue(QStringLiteral("auto_mode"), true);
        if (!m_deviceId.isEmpty()) {
            QJsonObject thresholds;
            QJsonObject luxObj{
                {QStringLiteral("min"), m_minLuxThreshold},
                {QStringLiteral("max"), m_maxLuxThreshold},
                {QStringLiteral("warning_below"), m_minLuxThreshold},
                {QStringLiteral("warning_above"), m_maxLuxThreshold}
            };
            thresholds.insert(QStringLiteral("light_lux"), luxObj);
            thresholds.insert(QStringLiteral("lux"), luxObj);
            const QJsonObject config{
                {QStringLiteral("sampling_interval_ms"), m_samplingIntervalSec * 1000},
                {QStringLiteral("auto_mode"), true},
                {QStringLiteral("thresholds"), thresholds}
            };
            emit deviceConfigRequested(m_deviceId, config);
        }
        updateUiState();
        checkAutoLightingLogic();
    });

    connect(m_manualModeBtn, &QPushButton::clicked, this, [this] {
        m_autoModeActive = false;
        QSettings s(QStringLiteral("ICTU"), QStringLiteral("TuanAnhSmartLight"));
        s.setValue(QStringLiteral("auto_mode"), false);
        if (!m_deviceId.isEmpty()) {
            QJsonObject thresholds;
            QJsonObject luxObj{
                {QStringLiteral("min"), m_minLuxThreshold},
                {QStringLiteral("max"), m_maxLuxThreshold},
                {QStringLiteral("warning_below"), m_minLuxThreshold},
                {QStringLiteral("warning_above"), m_maxLuxThreshold}
            };
            thresholds.insert(QStringLiteral("light_lux"), luxObj);
            thresholds.insert(QStringLiteral("lux"), luxObj);
            const QJsonObject config{
                {QStringLiteral("sampling_interval_ms"), m_samplingIntervalSec * 1000},
                {QStringLiteral("auto_mode"), false},
                {QStringLiteral("thresholds"), thresholds}
            };
            emit deviceConfigRequested(m_deviceId, config);
        }
        updateUiState();
    });

    modeRow->addWidget(m_autoModeBtn);
    modeRow->addWidget(m_manualModeBtn);
    col2Layout->addLayout(modeRow);

    // Button: Open Threshold Dialog
    m_configBtn = new QPushButton(tr("Cài Đặt Ngưỡng Tự Động"), col2Frame);
    m_configBtn->setCursor(Qt::PointingHandCursor);
    m_configBtn->setFixedHeight(28);
    m_configBtn->setStyleSheet(
        "QPushButton { background: #1e293b; color: #cbd5e1; border: 1px solid #334155; border-radius: 5px; font-size: 10px; font-weight: 800; } "
        "QPushButton:hover { background: #334155; color: #ffffff; }");
    connect(m_configBtn, &QPushButton::clicked, this, &DashboardPage::openConfigDialog);
    col2Layout->addWidget(m_configBtn);

    col2Layout->addStretch();
    gridRow->addWidget(col2Frame, 34);

    // ========================================================================
    // COLUMN 3 (33%): CẢM BIẾN HIỆN DIỆN PIR & XU HƯỚNG
    // ========================================================================
    auto *col3Frame = new QFrame(this);
    col3Frame->setProperty("class", QStringLiteral("metricCard"));
    auto *col3Layout = new QVBoxLayout(col3Frame);
    col3Layout->setContentsMargins(12, 10, 12, 10);
    col3Layout->setSpacing(8);

    auto *col3Title = new QLabel(tr("HIỆN DIỆN (PIR) & ĐỒ THỊ"), col3Frame);
    col3Title->setStyleSheet("color: #38bdf8; font-size: 11px; font-weight: 900; letter-spacing: 0.3px;");
    col3Layout->addWidget(col3Title, 0, Qt::AlignCenter);

    // Motion status container
    auto *motionBox = new QFrame(col3Frame);
    motionBox->setStyleSheet("background-color: #081022; border: 1px solid #162438; border-radius: 8px;");
    auto *motionBoxLayout = new QVBoxLayout(motionBox);
    motionBoxLayout->setContentsMargins(8, 6, 8, 8);
    motionBoxLayout->setSpacing(4);

    m_pirWidget = new PirMotionWidget(motionBox);
    motionBoxLayout->addWidget(m_pirWidget, 0, Qt::AlignCenter);

    m_motionBadgeLabel = new QLabel(tr("PHÒNG TRỐNG (YÊN TĨNH)"), motionBox);
    m_motionBadgeLabel->setAlignment(Qt::AlignCenter);
    m_motionBadgeLabel->setStyleSheet(
        "background-color: rgba(51, 65, 85, 0.35); color: #94a3b8; border: 1px solid #475569; border-radius: 5px; padding: 3px 8px; font-size: 10px; font-weight: 800;");
    motionBoxLayout->addWidget(m_motionBadgeLabel);

    m_motionDetailLabel = new QLabel(tr("Cảm biến PIR: Không có chuyển động"), motionBox);
    m_motionDetailLabel->setAlignment(Qt::AlignCenter);
    m_motionDetailLabel->setStyleSheet("color: #64748b; font-size: 9px; font-weight: 600;");
    motionBoxLayout->addWidget(m_motionDetailLabel);

    col3Layout->addWidget(motionBox);

    auto *waveLabel = new QLabel(tr("XU HƯỚNG ÁNH SÁNG REALTIME:"), col3Frame);
    waveLabel->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: 800;");
    col3Layout->addWidget(waveLabel);

    m_waveformWidget = new LuxWaveformWidget(col3Frame);
    col3Layout->addWidget(m_waveformWidget);

    col3Layout->addStretch();
    gridRow->addWidget(col3Frame, 33);

    ui->verticalLayout->addLayout(gridRow, 1);
}

void DashboardPage::updateUiState()
{
    // 1. Column 1: Lux & Gauge
    if (m_gaugeWidget) {
        m_gaugeWidget->setValue(m_curLux, 1000.0);
    }

    if (m_luxBadgeLabel) {
        if (!m_hasDevice || !m_isOnline) {
            m_luxBadgeLabel->setText(tr("MẤT KẾT NỐI"));
            m_luxBadgeLabel->setStyleSheet(
                "background-color: #3f151e; color: #f87171; border: 1px solid #7f1d1d; border-radius: 5px; padding: 4px 8px; font-size: 10px; font-weight: 800;");
        } else if (m_curLux <= m_minLuxThreshold) {
            m_luxBadgeLabel->setText(tr("PHÒNG TỐI (DƯỚI %1 LUX)").arg(static_cast<int>(m_minLuxThreshold)));
            m_luxBadgeLabel->setStyleSheet(
                "background-color: #3c2a10; color: #fbbf24; border: 1px solid #92400e; border-radius: 5px; padding: 4px 8px; font-size: 10px; font-weight: 800;");
        } else if (m_curLux >= m_maxLuxThreshold) {
            m_luxBadgeLabel->setText(tr("SÁNG TỰ NHIÊN (≥ %1 LUX)").arg(static_cast<int>(m_maxLuxThreshold)));
            m_luxBadgeLabel->setStyleSheet(
                "background-color: #0c2d48; color: #38bdf8; border: 1px solid #0284c7; border-radius: 5px; padding: 4px 8px; font-size: 10px; font-weight: 800;");
        } else {
            m_luxBadgeLabel->setText(tr("ĐỦ SÁNG (TIÊU CHUẨN)"));
            m_luxBadgeLabel->setStyleSheet(
                "background-color: #064e3b; color: #34d399; border: 1px solid #059669; border-radius: 5px; padding: 4px 8px; font-size: 10px; font-weight: 800;");
        }
    }

    if (m_thresholdLabel) {
        m_thresholdLabel->setText(tr("Ngưỡng bật: %1 Lux  •  Tắt: %2 Lux")
                                      .arg(m_minLuxThreshold, 0, 'f', 0)
                                      .arg(m_maxLuxThreshold, 0, 'f', 0));
    }

    // 2. Column 2: Lamp & Relay
    if (m_bulbWidget && m_lampStateLabel) {
        m_bulbWidget->setState(m_relayState);
        if (m_relayState) {
            m_lampStateLabel->setText(tr("● ĐÈN CHIẾU SÁNG: ĐANG BẬT"));
            m_lampStateLabel->setStyleSheet(
                "background-color: rgba(245, 158, 11, 0.18); color: #fbbf24; border: 1.5px solid #f59e0b; border-radius: 5px; padding: 3px 8px; font-size: 10px; font-weight: 900;");
        } else {
            m_lampStateLabel->setText(tr("ĐÈN CHIẾU SÁNG: ĐANG TẮT"));
            m_lampStateLabel->setStyleSheet(
                "background-color: rgba(51, 65, 85, 0.35); color: #94a3b8; border: 1px solid #475569; border-radius: 5px; padding: 3px 8px; font-size: 10px; font-weight: 800;");
        }
    }

    if (m_relayToggleBtn) {
        if (m_isRelayPending) {
            m_relayToggleBtn->setText(m_pendingRelayState ? tr("ĐANG BẬT ĐÈN...") : tr("ĐANG TẮT ĐÈN..."));
            m_relayToggleBtn->setStyleSheet(
                "QPushButton { background: #b45309; color: #ffffff; border: none; border-radius: 6px; font-size: 12px; font-weight: 900; }");
        } else if (m_relayState) {
            m_relayToggleBtn->setText(tr("TẮT ĐÈN PHÒNG"));
            m_relayToggleBtn->setStyleSheet(
                "QPushButton { background: #e11d48; color: #ffffff; border: none; border-radius: 6px; font-size: 12px; font-weight: 900; } "
                "QPushButton:hover { background: #be123c; }");
        } else {
            m_relayToggleBtn->setText(tr("BẬT ĐÈN PHÒNG"));
            m_relayToggleBtn->setStyleSheet(
                "QPushButton { background: #10b981; color: #ffffff; border: none; border-radius: 6px; font-size: 12px; font-weight: 900; } "
                "QPushButton:hover { background: #059669; }");
        }
    }

    // Mode buttons styling
    if (m_autoModeBtn && m_manualModeBtn) {
        if (m_autoModeActive) {
            m_autoModeBtn->setStyleSheet(
                "QPushButton { background: #0284c7; color: #ffffff; border: 1px solid #38bdf8; border-radius: 5px; font-size: 11px; font-weight: 900; }");
            m_manualModeBtn->setStyleSheet(
                "QPushButton { background: #1e293b; color: #64748b; border: 1px solid #334155; border-radius: 5px; font-size: 11px; font-weight: 700; } "
                "QPushButton:hover { background: #334155; color: #cbd5e1; }");
        } else {
            m_autoModeBtn->setStyleSheet(
                "QPushButton { background: #1e293b; color: #64748b; border: 1px solid #334155; border-radius: 5px; font-size: 11px; font-weight: 700; } "
                "QPushButton:hover { background: #334155; color: #cbd5e1; }");
            m_manualModeBtn->setStyleSheet(
                "QPushButton { background: #0284c7; color: #ffffff; border: 1px solid #38bdf8; border-radius: 5px; font-size: 11px; font-weight: 900; }");
        }
    }

    // 3. Column 3: PIR Presence & Waveform
    if (m_pirWidget) {
        m_pirWidget->setMotion(m_curMotion);
    }

    if (m_motionBadgeLabel && m_motionDetailLabel) {
        if (m_curMotion) {
            m_motionBadgeLabel->setText(tr("● PHÁT HIỆN CÓ NGƯỜI"));
            m_motionBadgeLabel->setStyleSheet(
                "background-color: rgba(16, 185, 129, 0.18); color: #34d399; border: 1.5px solid #10b981; border-radius: 5px; padding: 3px 8px; font-size: 10px; font-weight: 900;");
            m_motionDetailLabel->setText(tr("Phát hiện thân nhiệt hồng ngoại chuyển động"));
            m_motionDetailLabel->setStyleSheet("color: #34d399; font-size: 9px; font-weight: 700;");
        } else {
            m_motionBadgeLabel->setText(tr("PHÒNG TRỐNG (YÊN TĨNH)"));
            m_motionBadgeLabel->setStyleSheet(
                "background-color: rgba(51, 65, 85, 0.35); color: #94a3b8; border: 1px solid #475569; border-radius: 5px; padding: 3px 8px; font-size: 10px; font-weight: 800;");
            m_motionDetailLabel->setText(tr("Khu vực không có chuyển động"));
            m_motionDetailLabel->setStyleSheet("color: #64748b; font-size: 9px; font-weight: 600;");
        }
    }

    if (m_nodeStatusLabel) {
        if (!m_hasDevice || !m_isOnline) {
            m_nodeStatusLabel->setText(tr("● Trạm: %1 (Offline)").arg(m_deviceId.isEmpty() ? QStringLiteral("Chưa gán") : m_deviceId));
            m_nodeStatusLabel->setStyleSheet(
                "background-color: #3f151e; color: #f87171; border: 1px solid #7f1d1d; border-radius: 12px; padding: 4px 10px; font-size: 10px; font-weight: 800;");
        } else {
            m_nodeStatusLabel->setText(tr("● Trạm: %1 (Online)").arg(m_deviceId));
            m_nodeStatusLabel->setStyleSheet(
                "background-color: #0d281e; color: #34d399; border: 1px solid #065f46; border-radius: 12px; padding: 4px 10px; font-size: 10px; font-weight: 800;");
        }
    }
}

void DashboardPage::checkAutoLightingLogic()
{
    if (!m_autoModeActive || m_deviceId.isEmpty() || !m_hasDevice || m_isRelayPending)
        return;

    const bool isTooBright = (m_curLux >= m_maxLuxThreshold && m_maxLuxThreshold > 0.0);
    const bool shouldBeOn = m_curMotion && !isTooBright;
    const bool shouldBeOff = !m_curMotion || isTooBright;

    if (shouldBeOn && !m_relayState) {
        m_isRelayPending = true;
        m_pendingRelayState = true;
        m_relayPendingTimer->start(3500);
        updateUiState();
        emit relayControlRequested(m_deviceId, true);
    } else if (shouldBeOff && m_relayState) {
        m_isRelayPending = true;
        m_pendingRelayState = false;
        m_relayPendingTimer->start(3500);
        updateUiState();
        emit relayControlRequested(m_deviceId, false);
    }
}

void DashboardPage::updateReading(const SensorReading &reading)
{
    if (reading.pressureHpa > 0.0) {
        m_curLux = reading.pressureHpa;
        if (m_waveformWidget)
            m_waveformWidget->addSample(m_curLux);
        updateUiState();
        checkAutoLightingLogic();
    }
}

void DashboardPage::updateDeviceMetrics(const QJsonObject &metrics)
{
    // 1. Lux
    double valLux = 0.0;
    if (metrics.contains(QStringLiteral("light_lux"))) {
        valLux = metrics.value(QStringLiteral("light_lux")).toDouble();
    } else if (metrics.contains(QStringLiteral("lux"))) {
        valLux = metrics.value(QStringLiteral("lux")).toDouble();
    } else if (metrics.contains(QStringLiteral("detech")) && metrics.value(QStringLiteral("detech")).toDouble() > 1.0) {
        valLux = metrics.value(QStringLiteral("detech")).toDouble();
    }
    if (valLux > 0.0) {
        m_curLux = valLux;
        if (m_waveformWidget)
            m_waveformWidget->addSample(m_curLux);
    }

    // 2. Motion (PIR)
    if (metrics.contains(QStringLiteral("motion_detected"))) {
        m_curMotion = metrics.value(QStringLiteral("motion_detected")).toBool();
    } else if (metrics.contains(QStringLiteral("pir"))) {
        m_curMotion = (metrics.value(QStringLiteral("pir")).toDouble() > 0.5 || metrics.value(QStringLiteral("pir")).toBool());
    } else if (metrics.contains(QStringLiteral("detech")) && metrics.value(QStringLiteral("detech")).toDouble() <= 1.0) {
        m_curMotion = (metrics.value(QStringLiteral("detech")).toDouble() > 0.5);
    }

    // 3. Relay
    if (metrics.contains(QStringLiteral("relay_on"))) {
        const bool serverRelay = metrics.value(QStringLiteral("relay_on")).toBool();
        m_relayState = serverRelay;
        if (m_isRelayPending && serverRelay == m_pendingRelayState) {
            m_isRelayPending = false;
            m_relayPendingTimer->stop();
        }
    } else if (metrics.contains(QStringLiteral("relay"))) {
        const bool serverRelay = metrics.value(QStringLiteral("relay")).toBool();
        m_relayState = serverRelay;
        if (m_isRelayPending && serverRelay == m_pendingRelayState) {
            m_isRelayPending = false;
            m_relayPendingTimer->stop();
        }
    }

    updateUiState();
    checkAutoLightingLogic();
}

void DashboardPage::setAvailableDevices(const QJsonArray &devices)
{
    m_availableDevices = devices;
}

void DashboardPage::setOwnedDevices(const QJsonArray &devices)
{
    if (!devices.isEmpty()) {
        const auto first = devices.first().toObject();
        m_deviceId = first.value(QStringLiteral("device_id")).toString(QStringLiteral("150808"));
        m_deviceName = first.value(QStringLiteral("name")).toString(tr("Trạm Chiếu Sáng Thông Minh"));
        m_isOnline = first.value(QStringLiteral("is_online")).toBool(true);
        m_hasDevice = true;

        if (first.contains(QStringLiteral("config"))) {
            const auto cfg = first.value(QStringLiteral("config")).toObject();
            const auto thresh = cfg.value(QStringLiteral("thresholds")).toObject();
            QJsonObject luxObj;
            if (thresh.contains(QStringLiteral("lux"))) {
                luxObj = thresh.value(QStringLiteral("lux")).toObject();
            } else if (thresh.contains(QStringLiteral("light_lux"))) {
                luxObj = thresh.value(QStringLiteral("light_lux")).toObject();
            }
            if (luxObj.contains(QStringLiteral("min"))) {
                m_minLuxThreshold = luxObj.value(QStringLiteral("min")).toDouble(m_minLuxThreshold);
            } else if (luxObj.contains(QStringLiteral("warning_below"))) {
                m_minLuxThreshold = luxObj.value(QStringLiteral("warning_below")).toDouble(m_minLuxThreshold);
            }
            if (luxObj.contains(QStringLiteral("max"))) {
                m_maxLuxThreshold = luxObj.value(QStringLiteral("max")).toDouble(m_maxLuxThreshold);
            } else if (luxObj.contains(QStringLiteral("warning_above"))) {
                m_maxLuxThreshold = luxObj.value(QStringLiteral("warning_above")).toDouble(m_maxLuxThreshold);
            }
        }
        if (first.contains(QStringLiteral("state"))) {
            const auto st = first.value(QStringLiteral("state")).toObject();
            if (st.contains(QStringLiteral("relay")))
                m_relayState = st.value(QStringLiteral("relay")).toBool();
        }
        if (first.contains(QStringLiteral("metrics"))) {
            updateDeviceMetrics(first.value(QStringLiteral("metrics")).toObject());
        }
    } else {
        m_hasDevice = false;
        m_isOnline = false;
    }
    updateUiState();
}

void DashboardPage::setDeviceId(const QString &deviceId)
{
    m_deviceId = deviceId;
    updateUiState();
}

void DashboardPage::openConfigDialog()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Cài đặt ngưỡng tự động chiếu sáng"));
    dlg.setModal(true);
    dlg.setFixedSize(540, 420);
    dlg.setStyleSheet(
        "QDialog { background-color: #0b152d; color: #ffffff; font-family: sans-serif; } "
        "QLabel { color: #f1f5f9; font-size: 12px; font-weight: 700; } "
        "QLabel#dlgTitle { color: #38bdf8; font-size: 15px; font-weight: 900; } "
        "QLabel#dlgSubtitle { color: #94a3b8; font-size: 11px; font-weight: 600; } "
        "QLineEdit.valEdit { background-color: #070e22; color: #fbbf24; border: 1.5px solid #233870; border-radius: 6px; font-size: 14px; font-weight: 900; min-height: 32px; min-width: 105px; padding: 2px 6px; } "
        "QPushButton.stepBtn { background-color: #1a294c; color: #38bdf8; border: 1.5px solid #283e74; border-radius: 6px; font-size: 13px; font-weight: 900; min-width: 38px; min-height: 32px; } "
        "QPushButton.stepBtn:hover { background-color: #0284c7; color: #ffffff; border-color: #38bdf8; } "
        "QPushButton.stepBtn:pressed { background-color: #0369a1; } "
        "QPushButton.presetBtn { background-color: #101c3a; color: #94a3b8; border: 1px solid #1e3568; border-radius: 11px; padding: 3px 9px; font-size: 10px; font-weight: 700; } "
        "QPushButton.presetBtn:hover { background-color: #1e3a72; color: #38bdf8; border-color: #38bdf8; } "
        "QCheckBox { color: #ffffff; font-size: 12px; font-weight: 700; spacing: 8px; } "
        "QPushButton#saveBtn { background-color: #10b981; color: #ffffff; border: none; border-radius: 6px; font-size: 12px; font-weight: 900; padding: 8px 20px; min-height: 34px; } "
        "QPushButton#saveBtn:hover { background-color: #059669; } "
        "QPushButton#cancelBtn { background-color: #1e293b; color: #cbd5e1; border: 1px solid #334155; border-radius: 6px; font-size: 11px; font-weight: 800; padding: 6px 16px; min-height: 34px; } "
        "QPushButton#cancelBtn:hover { background-color: #334155; }"
    );

    auto *root = new QVBoxLayout(&dlg);
    root->setContentsMargins(18, 14, 18, 14);
    root->setSpacing(10);

    auto *title = new QLabel(tr("Cài đặt ngưỡng tự động chiếu sáng"), &dlg);
    title->setObjectName(QStringLiteral("dlgTitle"));
    auto *hint = new QLabel(tr("Bấm nút [+][-] hoặc chọn mốc nhanh để chỉnh ngưỡng chiếu sáng phòng."), &dlg);
    hint->setObjectName(QStringLiteral("dlgSubtitle"));
    hint->setWordWrap(true);
    root->addWidget(title);
    root->addWidget(hint);

    double curMin = m_minLuxThreshold;
    double curMax = m_maxLuxThreshold;
    int curInterval = m_samplingIntervalSec;

    // --- Helper Click Filter for Virtual Numpad ---
    struct NumpadClickFilter : public QObject {
        std::function<void()> onClick;
        NumpadClickFilter(QObject *p, std::function<void()> cb) : QObject(p), onClick(cb) {}
        bool eventFilter(QObject *w, QEvent *e) override {
            if (e->type() == QEvent::MouseButtonRelease) {
                if (onClick) onClick();
                return true;
            }
            return QObject::eventFilter(w, e);
        }
    };

    // ========================================================================
    // ROW 1: MIN LUX (Ngưỡng tối tự bật đèn)
    // ========================================================================
    auto *minCard = new QVBoxLayout;
    minCard->setSpacing(4);
    auto *minTitle = new QLabel(tr("Ngưỡng tối tự bật đèn (Min Lux):"), &dlg);
    minTitle->setStyleSheet("color: #fbbf24; font-size: 11px; font-weight: 800;");
    minCard->addWidget(minTitle);

    auto *minStepRow = new QHBoxLayout;
    minStepRow->setSpacing(5);
    auto *btnMinM10 = new QPushButton(QStringLiteral("-10"), &dlg);
    btnMinM10->setProperty("class", QStringLiteral("stepBtn"));
    btnMinM10->setCursor(Qt::PointingHandCursor);
    auto *btnMinM1 = new QPushButton(QStringLiteral("-"), &dlg);
    btnMinM1->setProperty("class", QStringLiteral("stepBtn"));
    btnMinM1->setCursor(Qt::PointingHandCursor);

    auto *minValEdit = new QLineEdit(&dlg);
    minValEdit->setProperty("class", QStringLiteral("valEdit"));
    minValEdit->setAlignment(Qt::AlignCenter);
    minValEdit->setReadOnly(true);
    minValEdit->setCursor(Qt::PointingHandCursor);
    minValEdit->setToolTip(tr("Bấm vào để mở bàn phím số"));

    auto *btnMinP1 = new QPushButton(QStringLiteral("+"), &dlg);
    btnMinP1->setProperty("class", QStringLiteral("stepBtn"));
    btnMinP1->setCursor(Qt::PointingHandCursor);
    auto *btnMinP10 = new QPushButton(QStringLiteral("+10"), &dlg);
    btnMinP10->setProperty("class", QStringLiteral("stepBtn"));
    btnMinP10->setCursor(Qt::PointingHandCursor);

    minStepRow->addWidget(btnMinM10);
    minStepRow->addWidget(btnMinM1);
    minStepRow->addWidget(minValEdit, 1);
    minStepRow->addWidget(btnMinP1);
    minStepRow->addWidget(btnMinP10);
    minCard->addLayout(minStepRow);

    // Min presets
    auto *minPresetRow = new QHBoxLayout;
    minPresetRow->setSpacing(6);
    auto *minPresetLbl = new QLabel(tr("Mốc nhanh:"), &dlg);
    minPresetLbl->setStyleSheet("color: #64748b; font-size: 9px; font-weight: 700;");
    minPresetRow->addWidget(minPresetLbl);
    const QVector<int> minPresets = { 20, 50, 80, 100, 150 };
    auto updateMinUi = [&] {
        minValEdit->setText(QStringLiteral("%1 Lux").arg(curMin, 0, 'f', 0));
    };
    for (int pVal : minPresets) {
        auto *pBtn = new QPushButton(QStringLiteral("%1 Lux").arg(pVal), &dlg);
        pBtn->setProperty("class", QStringLiteral("presetBtn"));
        pBtn->setCursor(Qt::PointingHandCursor);
        connect(pBtn, &QPushButton::clicked, &dlg, [&, pVal] {
            curMin = pVal;
            updateMinUi();
        });
        minPresetRow->addWidget(pBtn);
    }
    minPresetRow->addStretch();
    minCard->addLayout(minPresetRow);
    root->addLayout(minCard);

    connect(btnMinM10, &QPushButton::clicked, &dlg, [&] { curMin = qMax(0.0, curMin - 10.0); updateMinUi(); });
    connect(btnMinM1,  &QPushButton::clicked, &dlg, [&] { curMin = qMax(0.0, curMin - 5.0);  updateMinUi(); });
    connect(btnMinP1,  &QPushButton::clicked, &dlg, [&] { curMin = qMin(2000.0, curMin + 5.0); updateMinUi(); });
    connect(btnMinP10, &QPushButton::clicked, &dlg, [&] { curMin = qMin(2000.0, curMin + 10.0); updateMinUi(); });
    minValEdit->installEventFilter(new NumpadClickFilter(minValEdit, [&] {
        QLineEdit tempEdit;
        tempEdit.setText(QString::number(curMin, 'f', 0));
        VirtualKeyboardDialog::openFor(&tempEdit, &dlg, tr("Nhập ngưỡng tối bật đèn (Min Lux)"));
        bool ok = false;
        double v = tempEdit.text().toDouble(&ok);
        if (ok && v >= 0.0 && v <= 2000.0) {
            curMin = v;
            updateMinUi();
        }
    }));
    updateMinUi();

    // ========================================================================
    // ROW 2: MAX LUX (Ngưỡng sáng tự tắt đèn)
    // ========================================================================
    auto *maxCard = new QVBoxLayout;
    maxCard->setSpacing(4);
    auto *maxTitle = new QLabel(tr("Ngưỡng sáng tự tắt đèn (Max Lux):"), &dlg);
    maxTitle->setStyleSheet("color: #38bdf8; font-size: 11px; font-weight: 800;");
    maxCard->addWidget(maxTitle);

    auto *maxStepRow = new QHBoxLayout;
    maxStepRow->setSpacing(5);
    auto *btnMaxM50 = new QPushButton(QStringLiteral("-50"), &dlg);
    btnMaxM50->setProperty("class", QStringLiteral("stepBtn"));
    btnMaxM50->setCursor(Qt::PointingHandCursor);
    auto *btnMaxM10 = new QPushButton(QStringLiteral("-10"), &dlg);
    btnMaxM10->setProperty("class", QStringLiteral("stepBtn"));
    btnMaxM10->setCursor(Qt::PointingHandCursor);

    auto *maxValEdit = new QLineEdit(&dlg);
    maxValEdit->setProperty("class", QStringLiteral("valEdit"));
    maxValEdit->setAlignment(Qt::AlignCenter);
    maxValEdit->setReadOnly(true);
    maxValEdit->setCursor(Qt::PointingHandCursor);
    maxValEdit->setToolTip(tr("Bấm vào để mở bàn phím số"));

    auto *btnMaxP10 = new QPushButton(QStringLiteral("+10"), &dlg);
    btnMaxP10->setProperty("class", QStringLiteral("stepBtn"));
    btnMaxP10->setCursor(Qt::PointingHandCursor);
    auto *btnMaxP50 = new QPushButton(QStringLiteral("+50"), &dlg);
    btnMaxP50->setProperty("class", QStringLiteral("stepBtn"));
    btnMaxP50->setCursor(Qt::PointingHandCursor);

    maxStepRow->addWidget(btnMaxM50);
    maxStepRow->addWidget(btnMaxM10);
    maxStepRow->addWidget(maxValEdit, 1);
    maxStepRow->addWidget(btnMaxP10);
    maxStepRow->addWidget(btnMaxP50);
    maxCard->addLayout(maxStepRow);

    // Max presets
    auto *maxPresetRow = new QHBoxLayout;
    maxPresetRow->setSpacing(6);
    auto *maxPresetLbl = new QLabel(tr("Mốc nhanh:"), &dlg);
    maxPresetLbl->setStyleSheet("color: #64748b; font-size: 9px; font-weight: 700;");
    maxPresetRow->addWidget(maxPresetLbl);
    const QVector<int> maxPresets = { 200, 350, 500, 700, 1000 };
    auto updateMaxUi = [&] {
        maxValEdit->setText(QStringLiteral("%1 Lux").arg(curMax, 0, 'f', 0));
    };
    for (int pVal : maxPresets) {
        auto *pBtn = new QPushButton(QStringLiteral("%1 Lux").arg(pVal), &dlg);
        pBtn->setProperty("class", QStringLiteral("presetBtn"));
        pBtn->setCursor(Qt::PointingHandCursor);
        connect(pBtn, &QPushButton::clicked, &dlg, [&, pVal] {
            curMax = pVal;
            updateMaxUi();
        });
        maxPresetRow->addWidget(pBtn);
    }
    maxPresetRow->addStretch();
    maxCard->addLayout(maxPresetRow);
    root->addLayout(maxCard);

    connect(btnMaxM50, &QPushButton::clicked, &dlg, [&] { curMax = qMax(curMin + 10.0, curMax - 50.0); updateMaxUi(); });
    connect(btnMaxM10, &QPushButton::clicked, &dlg, [&] { curMax = qMax(curMin + 5.0,  curMax - 10.0); updateMaxUi(); });
    connect(btnMaxP10, &QPushButton::clicked, &dlg, [&] { curMax = qMin(5000.0, curMax + 10.0); updateMaxUi(); });
    connect(btnMaxP50, &QPushButton::clicked, &dlg, [&] { curMax = qMin(5000.0, curMax + 50.0); updateMaxUi(); });
    maxValEdit->installEventFilter(new NumpadClickFilter(maxValEdit, [&] {
        QLineEdit tempEdit;
        tempEdit.setText(QString::number(curMax, 'f', 0));
        VirtualKeyboardDialog::openFor(&tempEdit, &dlg, tr("Nhập ngưỡng sáng tắt đèn (Max Lux)"));
        bool ok = false;
        double v = tempEdit.text().toDouble(&ok);
        if (ok && v >= 0.0 && v <= 5000.0) {
            curMax = v;
            updateMaxUi();
        }
    }));
    updateMaxUi();

    // ========================================================================
    // ROW 3: SAMPLING INTERVAL (Chu kỳ lấy mẫu)
    // ========================================================================
    auto *intCard = new QVBoxLayout;
    intCard->setSpacing(4);
    auto *intTitle = new QLabel(tr("Chu kỳ lấy mẫu cảm biến:"), &dlg);
    intTitle->setStyleSheet("color: #94a3b8; font-size: 11px; font-weight: 800;");
    intCard->addWidget(intTitle);

    auto *intStepRow = new QHBoxLayout;
    intStepRow->setSpacing(5);
    auto *btnIntM1 = new QPushButton(QStringLiteral("-"), &dlg);
    btnIntM1->setProperty("class", QStringLiteral("stepBtn"));
    btnIntM1->setCursor(Qt::PointingHandCursor);

    auto *intValEdit = new QLineEdit(&dlg);
    intValEdit->setProperty("class", QStringLiteral("valEdit"));
    intValEdit->setAlignment(Qt::AlignCenter);
    intValEdit->setReadOnly(true);
    intValEdit->setCursor(Qt::PointingHandCursor);

    auto *btnIntP1 = new QPushButton(QStringLiteral("+"), &dlg);
    btnIntP1->setProperty("class", QStringLiteral("stepBtn"));
    btnIntP1->setCursor(Qt::PointingHandCursor);

    intStepRow->addWidget(btnIntM1);
    intStepRow->addWidget(intValEdit, 1);
    intStepRow->addWidget(btnIntP1);

    // Interval presets
    const QVector<int> intPresets = { 1, 2, 5, 10 };
    auto updateIntUi = [&] {
        intValEdit->setText(QStringLiteral("%1 giây").arg(curInterval));
    };
    for (int pVal : intPresets) {
        auto *pBtn = new QPushButton(QStringLiteral("%1s").arg(pVal), &dlg);
        pBtn->setProperty("class", QStringLiteral("presetBtn"));
        pBtn->setCursor(Qt::PointingHandCursor);
        connect(pBtn, &QPushButton::clicked, &dlg, [&, pVal] {
            curInterval = pVal;
            updateIntUi();
        });
        intStepRow->addWidget(pBtn);
    }
    intCard->addLayout(intStepRow);
    root->addLayout(intCard);

    connect(btnIntM1, &QPushButton::clicked, &dlg, [&] { curInterval = qMax(1, curInterval - 1); updateIntUi(); });
    connect(btnIntP1, &QPushButton::clicked, &dlg, [&] { curInterval = qMin(60, curInterval + 1); updateIntUi(); });
    intValEdit->installEventFilter(new NumpadClickFilter(intValEdit, [&] {
        QLineEdit tempEdit;
        tempEdit.setText(QString::number(curInterval));
        VirtualKeyboardDialog::openFor(&tempEdit, &dlg, tr("Nhập chu kỳ lấy mẫu (giây)"));
        bool ok = false;
        int v = tempEdit.text().toInt(&ok);
        if (ok && v >= 1 && v <= 60) {
            curInterval = v;
            updateIntUi();
        }
    }));
    updateIntUi();

    // ========================================================================
    // ROW 4: AUTO TRIGGER CHECKBOX
    // ========================================================================
    auto *autoCheck = new QCheckBox(tr("Tự động bật đèn khi phòng tối hoặc phát hiện có người"), &dlg);
    autoCheck->setChecked(m_autoModeActive);
    autoCheck->setCursor(Qt::PointingHandCursor);
    root->addWidget(autoCheck);

    // ========================================================================
    // ROW 5: ACTIONS
    // ========================================================================
    auto *actions = new QHBoxLayout;
    actions->setSpacing(10);
    auto *cancel = new QPushButton(tr("Đóng"), &dlg);
    cancel->setObjectName(QStringLiteral("cancelBtn"));
    cancel->setCursor(Qt::PointingHandCursor);

    auto *save = new QPushButton(tr("Lưu && Gửi Thiết Bị"), &dlg);
    save->setObjectName(QStringLiteral("saveBtn"));
    save->setCursor(Qt::PointingHandCursor);

    actions->addStretch();
    actions->addWidget(cancel);
    actions->addWidget(save);
    root->addLayout(actions);

    connect(cancel, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(save, &QPushButton::clicked, &dlg, [&, autoCheck] {
        m_minLuxThreshold = curMin;
        m_maxLuxThreshold = curMax;
        m_samplingIntervalSec = curInterval;
        m_autoModeActive = autoCheck->isChecked();

        QSettings s(QStringLiteral("ICTU"), QStringLiteral("TuanAnhSmartLight"));
        s.setValue(QStringLiteral("auto_mode"), m_autoModeActive);

        if (!m_deviceId.isEmpty()) {
            QJsonObject thresholds;
            QJsonObject luxObj{
                {QStringLiteral("min"), m_minLuxThreshold},
                {QStringLiteral("max"), m_maxLuxThreshold},
                {QStringLiteral("warning_below"), m_minLuxThreshold},
                {QStringLiteral("warning_above"), m_maxLuxThreshold}
            };
            thresholds.insert(QStringLiteral("light_lux"), luxObj);
            thresholds.insert(QStringLiteral("lux"), luxObj);
            const QJsonObject config{
                {QStringLiteral("sampling_interval_ms"), m_samplingIntervalSec * 1000},
                {QStringLiteral("auto_mode"), m_autoModeActive},
                {QStringLiteral("thresholds"), thresholds}
            };
            emit deviceConfigRequested(m_deviceId, config);
        }

        updateUiState();
        checkAutoLightingLogic();
        dlg.accept();
    });

    dlg.exec();
}
