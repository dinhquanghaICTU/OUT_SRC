#include "WaterTankWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QFont>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

WaterTankWidget::WaterTankWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(85, 105);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    connect(&m_animTimer, &QTimer::timeout, this, [this] {
        if (!isVisible()) return;
        m_wavePhase += 0.15;
        if (m_wavePhase > 2 * M_PI) {
            m_wavePhase -= 2 * M_PI;
        }
        update();
    });
    m_animTimer.start(60);
}

void WaterTankWidget::setDistance(double distanceCm, double minDistanceCm, double maxDistanceCm)
{
    m_distanceCm = distanceCm;
    m_minDistanceCm = (minDistanceCm > 0.0) ? minDistanceCm : 10.0;
    m_maxDistanceCm = (maxDistanceCm > m_minDistanceCm) ? maxDistanceCm : 60.0;

    // Khoảng cách cảm biến HC-SR04 đo từ đỉnh bồn xuống mặt nước:
    // Càng gần (<= minDistanceCm) -> Bể càng đầy (100%)
    // Càng xa (>= maxDistanceCm) -> Bể càng cạn (0%)
    double pct = 0.0;
    if (m_distanceCm <= m_minDistanceCm) {
        pct = 100.0;
    } else if (m_distanceCm >= m_maxDistanceCm) {
        pct = 0.0;
    } else {
        pct = 100.0 * (m_maxDistanceCm - m_distanceCm) / (m_maxDistanceCm - m_minDistanceCm);
    }
    setWaterPercent(pct);
}

void WaterTankWidget::setWaterPercent(double percent)
{
    m_percent = qBound(0.0, percent, 100.0);
    update();
}

QString WaterTankWidget::statusText() const
{
    if (m_percent >= 80.0) return tr("BỂ ĐẦY");
    if (m_percent >= 25.0) return tr("BÌNH THƯỜNG");
    return tr("CẠN NƯỚC");
}

QColor WaterTankWidget::statusColor() const
{
    if (m_percent >= 80.0) return QColor("#38bdf8"); // Cyan
    if (m_percent >= 25.0) return QColor("#10b981"); // Emerald
    return QColor("#ef4444"); // Red
}

void WaterTankWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const qreal w = width();
    const qreal h = height();

    // Dành 24px bên phải cho thang đo 100%, 75%, 50%, 25%, 0%
    const qreal scaleWidth = 22.0;
    const QRectF tankRect(4.0, 4.0, w - scaleWidth - 6.0, h - 8.0);
    const qreal cornerRadius = 8.0;

    // --- 1. Vẽ thân bể (Glass Cylinder/Tank) ---
    QPainterPath tankPath;
    tankPath.addRoundedRect(tankRect, cornerRadius, cornerRadius);

    // Nền bể thủy tinh trong suốt
    p.fillPath(tankPath, QColor(11, 22, 50, 200));

    // Nắp và đáy bể (metallic caps)
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(30, 58, 102, 180));
    p.drawRoundedRect(QRectF(tankRect.left() + 2, tankRect.top(), tankRect.width() - 4, 5.0), 2, 2);
    p.drawRoundedRect(QRectF(tankRect.left() + 2, tankRect.bottom() - 5, tankRect.width() - 4, 5.0), 2, 2);

    // --- 2. Vẽ mực nước dạng sóng lượn (Liquid Fill with Waves) ---
    p.save();
    p.setClipPath(tankPath);

    const qreal innerBottom = tankRect.bottom() - 2.0;
    const qreal innerTop = tankRect.top() + 4.0;
    const qreal innerH = innerBottom - innerTop;
    const qreal fillH = (m_percent / 100.0) * innerH;
    const qreal surfaceY = innerBottom - fillH;

    if (fillH > 1.0) {
        QPainterPath waterPath;
        waterPath.moveTo(tankRect.left() - 2.0, innerBottom + 5.0);

        const int steps = 24;
        const qreal stepW = (tankRect.width() + 4.0) / steps;
        for (int i = 0; i <= steps; ++i) {
            qreal curX = tankRect.left() - 2.0 + i * stepW;
            qreal wave = std::sin((curX / tankRect.width()) * 2.0 * M_PI + m_wavePhase) * 2.2
                       + std::cos((curX / tankRect.width()) * 4.0 * M_PI - m_wavePhase * 0.8) * 1.0;
            qreal curY = surfaceY + wave;
            if (curY > innerBottom) curY = innerBottom;
            if (i == 0) {
                waterPath.lineTo(curX, curY);
            } else {
                waterPath.lineTo(curX, curY);
            }
        }
        waterPath.lineTo(tankRect.right() + 2.0, innerBottom + 5.0);
        waterPath.closeSubpath();

        // Gradient nước
        QLinearGradient waterGrad(0, surfaceY, 0, innerBottom);
        if (m_percent >= 25.0) {
            waterGrad.setColorAt(0.0, QColor(56, 189, 248, 230));  // Cyan sáng
            waterGrad.setColorAt(0.35, QColor(14, 165, 233, 240)); // Sky blue
            waterGrad.setColorAt(1.0, QColor(2, 132, 199, 255));   // Deep marine blue
        } else {
            waterGrad.setColorAt(0.0, QColor(251, 191, 36, 230));  // Cam cảnh báo
            waterGrad.setColorAt(1.0, QColor(239, 68, 68, 255));   // Đỏ cạn nước
        }

        p.setPen(Qt::NoPen);
        p.setBrush(waterGrad);
        p.drawPath(waterPath);

        // Viền bọt sóng phát sáng ở bề mặt nước
        p.setPen(QPen(QColor(224, 242, 254, 210), 1.5));
        p.setBrush(Qt::NoBrush);
        QPainterPath crestPath;
        for (int i = 0; i <= steps; ++i) {
            qreal curX = tankRect.left() - 2.0 + i * stepW;
            qreal wave = std::sin((curX / tankRect.width()) * 2.0 * M_PI + m_wavePhase) * 2.2
                       + std::cos((curX / tankRect.width()) * 4.0 * M_PI - m_wavePhase * 0.8) * 1.0;
            qreal curY = surfaceY + wave;
            if (i == 0) crestPath.moveTo(curX, curY);
            else crestPath.lineTo(curX, curY);
        }
        p.drawPath(crestPath);
    }

    // Vệt sáng phản chiếu mặt kính (Glass Sheen Highlight)
    QLinearGradient sheenGrad(tankRect.topLeft(), tankRect.center());
    sheenGrad.setColorAt(0.0, QColor(255, 255, 255, 45));
    sheenGrad.setColorAt(0.5, QColor(255, 255, 255, 10));
    sheenGrad.setColorAt(1.0, QColor(255, 255, 255, 0));
    p.setPen(Qt::NoPen);
    p.setBrush(sheenGrad);
    p.drawRect(QRectF(tankRect.left() + 2, tankRect.top() + 2, tankRect.width() * 0.35, tankRect.height() - 4));

    p.restore();

    // Viền ngoài thành bể
    p.setPen(QPen(QColor(56, 189, 248, 140), 1.5));
    p.setBrush(Qt::NoBrush);
    p.drawPath(tankPath);

    // --- 3. Thang đo mức nước (Graduation Scale Ticks) ---
    QFont scaleFont;
    scaleFont.setPixelSize(7);
    scaleFont.setWeight(QFont::Bold);
    p.setFont(scaleFont);
    p.setPen(QColor("#64748b"));

    const qreal tickX = tankRect.right() + 3.0;
    const int tickMarks[5] = {100, 75, 50, 25, 0};
    for (int mark : tickMarks) {
        qreal tickY = innerBottom - (mark / 100.0) * innerH;
        p.drawLine(QPointF(tickX, tickY), QPointF(tickX + 4.0, tickY));
        p.drawText(QRectF(tickX + 6.0, tickY - 5.0, 16.0, 10.0), Qt::AlignLeft | Qt::AlignVCenter, QString::number(mark));
    }

    // --- 4. Hiển thị phần trăm nổi bật ở giữa bể ---
    QFont pctFont;
    pctFont.setPixelSize(13);
    pctFont.setWeight(QFont::Black);
    p.setFont(pctFont);

    const QString pctStr = QStringLiteral("%1%").arg(qRound(m_percent));
    const QRectF textRect(tankRect.left(), tankRect.top() + (tankRect.height() - 20) / 2.0, tankRect.width(), 20.0);

    // Text Shadow
    p.setPen(QColor(0, 0, 0, 180));
    p.drawText(textRect.translated(1, 1), Qt::AlignCenter, pctStr);

    // Text Foreground
    p.setPen(QColor("#ffffff"));
    p.drawText(textRect, Qt::AlignCenter, pctStr);
}
