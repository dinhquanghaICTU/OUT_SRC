#include "DoorVisualizerWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QtMath>

DoorVisualizerWidget::DoorVisualizerWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(320, 180);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(&m_animTimer, &QTimer::timeout, this, [this] {
        m_radarPulse += 0.08;
        if (m_radarPulse > 1.0) m_radarPulse = 0.0;

        if (std::abs(m_motorRpm) > 0.1) {
            m_motorAngle += (m_motorRpm > 0 ? 8.0 : -8.0);
            if (m_motorAngle >= 360.0) m_motorAngle -= 360.0;
            if (m_motorAngle < 0.0) m_motorAngle += 360.0;
        }
        update();
    });
    m_animTimer.start(40);
}

void DoorVisualizerWidget::setDoorPosition(double pct)
{
    m_doorPositionPct = qBound(0.0, pct, 100.0);
    update();
}

void DoorVisualizerWidget::setMotionDetected(bool detected)
{
    m_motionDetected = detected;
    update();
}

void DoorVisualizerWidget::setIrBlocked(bool blocked)
{
    m_irBlocked = blocked;
    update();
}

void DoorVisualizerWidget::setDoorState(const QString &state)
{
    m_doorState = state;
    update();
}

void DoorVisualizerWidget::setMotorSpeed(double rpm)
{
    m_motorRpm = rpm;
    update();
}

void DoorVisualizerWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const int w = width();
    const int h = height();

    // Dark slate container card background
    QRectF bgRect(0, 0, w, h);
    p.fillRect(bgRect, QColor(QStringLiteral("#0d1322")));

    // Cyber border
    QPen borderPen(QColor(QStringLiteral("#1e293b")), 1.5);
    p.setPen(borderPen);
    p.drawRoundedRect(bgRect.adjusted(1, 1, -1, -1), 10, 10);

    // Header label on canvas
    p.setPen(QColor(QStringLiteral("#94a3b8")));
    QFont headerFont = p.font();
    headerFont.setPointSize(9);
    headerFont.setBold(true);
    p.setFont(headerFont);
    p.drawText(QRectF(14, 10, 200, 20), Qt::AlignLeft | Qt::AlignVCenter, tr("MÔ PHỎNG CỬA TRƯỢT 2 CÁNH"));

    // Top Right Door Status Badge
    QString stateText;
    QColor stateColor;
    if (m_irBlocked) {
        stateText = tr("⚠️ KẸT VẬT CẢN (IR)");
        stateColor = QColor(QStringLiteral("#ef4444"));
    } else if (m_doorState == QStringLiteral("OPEN") || m_doorPositionPct >= 98.0) {
        stateText = tr("🟢 MỞ HOÀN TOÀN");
        stateColor = QColor(QStringLiteral("#10b981"));
    } else if (m_doorState == QStringLiteral("OPENING")) {
        stateText = tr("↗ ĐANG MỞ...");
        stateColor = QColor(QStringLiteral("#06b6d4"));
    } else if (m_doorState == QStringLiteral("CLOSING")) {
        stateText = tr("↙ ĐANG ĐÓNG...");
        stateColor = QColor(QStringLiteral("#f59e0b"));
    } else {
        stateText = tr("⚪ CỬA ĐÃ ĐÓNG");
        stateColor = QColor(QStringLiteral("#64748b"));
    }

    QRectF badgeRect(w - 170, 8, 156, 24);
    p.setBrush(QColor(stateColor.red(), stateColor.green(), stateColor.blue(), 35));
    p.setPen(QPen(stateColor, 1.2));
    p.drawRoundedRect(badgeRect, 12, 12);
    p.setPen(stateColor);
    QFont badgeFont = p.font();
    badgeFont.setPointSize(8);
    badgeFont.setBold(true);
    p.setFont(badgeFont);
    p.drawText(badgeRect, Qt::AlignCenter, stateText);

    // Door Frame Dimensions
    const double frameMarginX = 36.0;
    const double frameTop = 40.0;
    const double frameHeight = h - 60.0;
    const double frameWidth = w - (frameMarginX * 2.0);
    const double doorFullWidth = frameWidth / 2.0;

    // Outer door frame background (room opening)
    QRectF doorwayRect(frameMarginX, frameTop, frameWidth, frameHeight);
    QLinearGradient doorwayGrad(doorwayRect.topLeft(), doorwayRect.bottomLeft());
    doorwayGrad.setColorAt(0.0, QColor(QStringLiteral("#070b14")));
    doorwayGrad.setColorAt(1.0, QColor(QStringLiteral("#0f172a")));
    p.setBrush(doorwayGrad);
    p.setPen(QPen(QColor(QStringLiteral("#334155")), 2.0));
    p.drawRect(doorwayRect);

    // Floor line & Track
    p.setPen(QPen(QColor(QStringLiteral("#475569")), 3.0));
    p.drawLine(QPointF(frameMarginX - 10, frameTop + frameHeight),
               QPointF(frameMarginX + frameWidth + 10, frameTop + frameHeight));

    // Top Guide Rail
    p.setPen(QPen(QColor(QStringLiteral("#3b82f6")), 2.5));
    p.drawLine(QPointF(frameMarginX, frameTop + 2),
               QPointF(frameMarginX + frameWidth, frameTop + 2));

    // 1. SR602 PIR Motion Radar Zone (Top center sensor)
    const QPointF pirSensorPos(w / 2.0, frameTop + 2);
    p.setBrush(m_motionDetected ? QColor(QStringLiteral("#10b981")) : QColor(QStringLiteral("#64748b")));
    p.setPen(Qt::NoPen);
    p.drawEllipse(pirSensorPos, 5, 5);

    // Animated motion wave
    if (m_motionDetected) {
        p.setBrush(Qt::NoBrush);
        for (int i = 1; i <= 3; ++i) {
            double radius = 18.0 * i + (m_radarPulse * 20.0);
            int alpha = qMax(0, static_cast<int>(180 - (radius * 1.6)));
            p.setPen(QPen(QColor(16, 185, 129, alpha), 1.5, Qt::DashLine));
            QRectF arcRect(pirSensorPos.x() - radius, pirSensorPos.y() - radius / 2.0, radius * 2.0, radius);
            p.drawArc(arcRect, 200 * 16, 140 * 16);
        }

        // Motion text
        p.setPen(QColor(QStringLiteral("#10b981")));
        QFont pirFont = p.font();
        pirFont.setPointSize(7);
        pirFont.setBold(true);
        p.setFont(pirFont);
        p.drawText(QRectF(w / 2.0 - 60, frameTop + 8, 120, 16), Qt::AlignCenter, tr("SR602: PHÁT HIỆN NGƯỜI"));
    }

    // 2. Door Sliding Leaves (Left and Right doors)
    // When doorPositionPct = 0% -> doors meet in middle.
    // When doorPositionPct = 100% -> left door slides to x=frameMarginX - doorWidth + 8, right door slides to right.
    const double slideDist = (doorFullWidth - 10.0) * (m_doorPositionPct / 100.0);

    // Left Door Leaf
    const double leftDoorX = frameMarginX + (doorFullWidth - slideDist - doorFullWidth);
    QRectF leftDoor(leftDoorX, frameTop + 4, doorFullWidth, frameHeight - 6);

    // Right Door Leaf
    const double rightDoorX = frameMarginX + doorFullWidth + slideDist;
    QRectF rightDoor(rightDoorX, frameTop + 4, doorFullWidth, frameHeight - 6);

    // Draw Left Glass Door
    QLinearGradient glassGrad1(leftDoor.topLeft(), leftDoor.bottomRight());
    glassGrad1.setColorAt(0.0, QColor(30, 58, 138, 160));
    glassGrad1.setColorAt(0.5, QColor(6, 182, 212, 90));
    glassGrad1.setColorAt(1.0, QColor(15, 23, 42, 220));
    p.setBrush(glassGrad1);
    p.setPen(QPen(QColor(QStringLiteral("#38bdf8")), 1.8));
    p.drawRect(leftDoor);

    // Left Door Handle
    p.setPen(QPen(QColor(QStringLiteral("#e2e8f0")), 3.0));
    p.drawLine(QPointF(leftDoor.right() - 8, leftDoor.top() + frameHeight * 0.35),
               QPointF(leftDoor.right() - 8, leftDoor.top() + frameHeight * 0.65));

    // Draw Right Glass Door
    QLinearGradient glassGrad2(rightDoor.topLeft(), rightDoor.bottomRight());
    glassGrad2.setColorAt(0.0, QColor(30, 58, 138, 160));
    glassGrad2.setColorAt(0.5, QColor(6, 182, 212, 90));
    glassGrad2.setColorAt(1.0, QColor(15, 23, 42, 220));
    p.setBrush(glassGrad2);
    p.setPen(QPen(QColor(QStringLiteral("#38bdf8")), 1.8));
    p.drawRect(rightDoor);

    // Right Door Handle
    p.setPen(QPen(QColor(QStringLiteral("#e2e8f0")), 3.0));
    p.drawLine(QPointF(rightDoor.left() + 8, rightDoor.top() + frameHeight * 0.35),
               QPointF(rightDoor.left() + 8, rightDoor.top() + frameHeight * 0.65));

    // 3. IR Sensor Safety Laser Beam (horizontal across threshold)
    const double irBeamY = frameTop + (frameHeight * 0.60);
    QPointF irEmitter(frameMarginX, irBeamY);
    QPointF irReceiver(frameMarginX + frameWidth, irBeamY);

    if (m_irBlocked) {
        // Red laser with obstacle pulse
        p.setPen(QPen(QColor(QStringLiteral("#ef4444")), 2.5, Qt::SolidLine));
        p.drawLine(irEmitter, irReceiver);

        // Pulsing obstacle icon in center
        p.setBrush(QColor(239, 68, 68, 200));
        p.drawEllipse(QPointF(w / 2.0, irBeamY), 12, 12);
        p.setPen(QColor(QStringLiteral("#ffffff")));
        QFont warnFont = p.font();
        warnFont.setPointSize(8);
        warnFont.setBold(true);
        p.setFont(warnFont);
        p.drawText(QRectF(w / 2.0 - 10, irBeamY - 10, 20, 20), Qt::AlignCenter, QStringLiteral("!"));

        p.setPen(QColor(QStringLiteral("#ef4444")));
        p.drawText(QRectF(w / 2.0 - 100, irBeamY + 14, 200, 16), Qt::AlignCenter, tr("CHÙM TIA IR BỊ CHẮN"));
    } else {
        // Green subtle laser beam
        p.setPen(QPen(QColor(16, 185, 129, 90), 1.2, Qt::DashLine));
        p.drawLine(irEmitter, irReceiver);
    }

    // 4. Stepper Motor Pulley Indicator (Top Right of frame)
    const QPointF motorCenter(frameMarginX + frameWidth - 16, frameTop + 14);
    p.setBrush(QColor(QStringLiteral("#1e293b")));
    p.setPen(QPen(QColor(QStringLiteral("#94a3b8")), 1.5));
    p.drawEllipse(motorCenter, 9, 9);

    // Rotating spokes
    p.save();
    p.translate(motorCenter);
    p.rotate(m_motorAngle);
    p.setPen(QPen(QColor(QStringLiteral("#38bdf8")), 1.5));
    p.drawLine(QPointF(-7, 0), QPointF(7, 0));
    p.drawLine(QPointF(0, -7), QPointF(0, 7));
    p.restore();

    // Bottom info footer
    p.setPen(QColor(QStringLiteral("#64748b")));
    QFont footFont = p.font();
    footFont.setPointSize(8);
    p.setFont(footFont);
    p.drawText(QRectF(frameMarginX, h - 18, 150, 16), Qt::AlignLeft,
               tr("Vị trí: %1%").arg(QString::number(m_doorPositionPct, 'f', 0)));
    p.drawText(QRectF(w - frameMarginX - 160, h - 18, 160, 16), Qt::AlignRight,
               tr("Động cơ bước: %1 RPM").arg(QString::number(m_motorRpm, 'f', 0)));
}
