#include "mainwindow.h"
#include "VirtualKeyboard.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <QApplication>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QCheckBox>
#include <QComboBox>
#include <QConicalGradient>
#include <QDate>
#include <QDateTime>
#include <QDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QLinearGradient>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStyle>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>

/* =========================================================================
   TACTICAL RADAR SCAN WIDGET (Radar Giám Sát Chuyển Động Xoay 360 Độ)
   ========================================================================= */
RadarScanWidget::RadarScanWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(130, 130);
    setMaximumSize(155, 155);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    m_animTimer = new QTimer(this);
    connect(m_animTimer, &QTimer::timeout, this, [this] {
        m_angle += 4.0;
        if (m_angle >= 360.0) m_angle -= 360.0;

        if (m_pulseUp) {
            m_pulse += 0.08;
            if (m_pulse >= 1.0) { m_pulse = 1.0; m_pulseUp = false; }
        } else {
            m_pulse -= 0.08;
            if (m_pulse <= 0.0) { m_pulse = 0.0; m_pulseUp = true; }
        }
        update();
    });
    m_animTimer->start(35);
}

void RadarScanWidget::setMotionDetected(bool detected)
{
    if (m_motionDetected != detected) {
        m_motionDetected = detected;
        update();
    }
}

void RadarScanWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int size = qMin(width(), height()) - 8;
    const int cx = width() / 2;
    const int cy = height() / 2;
    const int r = size / 2;

    // Dark Radar Background
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(m_motionDetected ? "#220510" : "#020d1e"));
    p.drawEllipse(QPoint(cx, cy), r, r);

    // Concentric Range Rings (1.5m, 3.0m, 5.0m)
    QPen gridPen(m_motionDetected ? QColor(255, 23, 68, 80) : QColor(0, 242, 254, 70), 1);
    p.setPen(gridPen);
    p.setBrush(Qt::NoBrush);

    p.drawEllipse(QPoint(cx, cy), int(r * 0.33), int(r * 0.33));
    p.drawEllipse(QPoint(cx, cy), int(r * 0.66), int(r * 0.66));
    p.drawEllipse(QPoint(cx, cy), r - 1, r - 1);

    // Range Distance Labels
    QFont f = p.font();
    f.setPointSize(6);
    f.setBold(true);
    p.setFont(f);
    p.setPen(m_motionDetected ? QColor(255, 100, 120, 180) : QColor(0, 242, 254, 150));
    p.drawText(cx + 4, cy - int(r * 0.33) + 8, "1.5m");
    p.drawText(cx + 4, cy - int(r * 0.66) + 8, "3.0m");
    p.drawText(cx + 4, cy - r + 10, "5.0m");

    // Azimuth Crosshairs
    p.drawLine(cx - r, cy, cx + r, cy);
    p.drawLine(cx, cy - r, cx, cy + r);

    // Diagonal Spokes (45, 135, 225, 315 deg)
    QPen diagPen(m_motionDetected ? QColor(255, 23, 68, 40) : QColor(0, 242, 254, 35), 1, Qt::DotLine);
    p.setPen(diagPen);
    const int dOffset = int(r * 0.707);
    p.drawLine(cx - dOffset, cy - dOffset, cx + dOffset, cy + dOffset);
    p.drawLine(cx - dOffset, cy + dOffset, cx + dOffset, cy - dOffset);

    // Cardinal Labels
    f.setPointSize(7);
    f.setBold(true);
    p.setFont(f);
    p.setPen(m_motionDetected ? QColor(255, 80, 110) : QColor(0, 242, 254));
    p.drawText(cx - 4, cy - r + 12, "N");
    p.drawText(cx - 4, cy + r - 3, "S");
    p.drawText(cx + r - 13, cy + 3, "E");
    p.drawText(cx - r + 4, cy + 3, "W");

    // Rotating Radar Beam
    p.save();
    p.translate(cx, cy);
    p.rotate(m_angle);

    QConicalGradient grad(0, 0, 0);
    if (m_motionDetected) {
        grad.setColorAt(0.0, QColor(255, 23, 68, 230));
        grad.setColorAt(0.15, QColor(255, 23, 68, 60));
        grad.setColorAt(0.30, QColor(255, 23, 68, 0));
        grad.setColorAt(1.0, QColor(255, 23, 68, 0));
    } else {
        grad.setColorAt(0.0, QColor(0, 242, 254, 230));
        grad.setColorAt(0.15, QColor(0, 242, 254, 60));
        grad.setColorAt(0.30, QColor(0, 242, 254, 0));
        grad.setColorAt(1.0, QColor(0, 242, 254, 0));
    }
    p.setBrush(grad);
    p.setPen(Qt::NoPen);
    p.drawPie(-r, -r, r * 2, r * 2, 0, 360 * 16);

    // Leading sweep ray
    QPen leadPen(m_motionDetected ? QColor("#ff1744") : QColor("#00f2fe"), 2);
    p.setPen(leadPen);
    p.drawLine(0, 0, r, 0);
    p.restore();

    // Target Blip when Motion is detected
    if (m_motionDetected) {
        const int blipX = cx + int(r * 0.52 * std::cos(0.785));
        const int blipY = cy - int(r * 0.52 * std::sin(0.785));
        const int pulseR = 4 + int(m_pulse * 10);

        p.setPen(QPen(QColor(255, 23, 68, int(230 * (1.0 - m_pulse))), 2));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPoint(blipX, blipY), pulseR, pulseR);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#ff1744"));
        p.drawEllipse(QPoint(blipX, blipY), 4, 4);

        f.setPointSize(6);
        p.setFont(f);
        p.setPen(QColor("#ff1744"));
        p.drawText(blipX + 6, blipY - 4, "MỤC TIÊU!");
    }

    // Center Station Point
    p.setPen(Qt::NoPen);
    p.setBrush(m_motionDetected ? QColor("#ff1744") : QColor("#00f2fe"));
    p.drawEllipse(QPoint(cx, cy), 3, 3);

    // Outer Glow Border
    QPen borderPen(m_motionDetected ? QColor("#ff1744") : QColor("#00f2fe"), 1.8);
    p.setPen(borderPen);
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QPoint(cx, cy), r, r);
}

/* =========================================================================
   BAROMETER DIAL GAUGE WIDGET (Đồng Hồ Áp Suất Khí Quyển Chuyên Dụng)
   ========================================================================= */
BarometerDialWidget::BarometerDialWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(130, 130);
    setMaximumSize(155, 155);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void BarometerDialWidget::setPressure(double pressureHpa)
{
    m_pressure = pressureHpa;
    m_hasData = true;
    update();
}

void BarometerDialWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int size = qMin(width(), height()) - 8;
    const int cx = width() / 2;
    const int cy = height() / 2;
    const int r = size / 2;

    // Outer Dial Metallic Bezel
    QLinearGradient bezelGrad(cx - r, cy - r, cx + r, cy + r);
    bezelGrad.setColorAt(0.0, QColor("#1e3a8a"));
    bezelGrad.setColorAt(0.5, QColor("#0f172a"));
    bezelGrad.setColorAt(1.0, QColor("#0284c7"));
    p.setPen(QPen(bezelGrad, 2));
    p.setBrush(QColor("#040d1f"));
    p.drawEllipse(QPoint(cx, cy), r, r);

    // Inner Dial Face
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#06132d"));
    p.drawEllipse(QPoint(cx, cy), r - 4, r - 4);

    // Arc Range: 225 deg down to -45 deg (270 degrees total span)
    // Scale: 960 hPa to 1050 hPa (delta = 90 hPa)
    const double pMin = 960.0;
    const double pMax = 1050.0;

    // Background track arc
    p.setPen(QPen(QColor("#132752"), 7, Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(cx - r + 14, cy - r + 14, (r - 14) * 2, (r - 14) * 2, 225 * 16, -270 * 16);

    // Active Gradient Arc
    double ratio = (m_pressure - pMin) / (pMax - pMin);
    ratio = qBound(0.0, ratio, 1.0);
    const int activeSpan = int(-270.0 * ratio * 16);

    QColor activeColor = QColor("#00f2fe");
    if (m_pressure < 995.0) activeColor = QColor("#f43f5e");
    else if (m_pressure > 1025.0) activeColor = QColor("#818cf8");

    QPen activePen(activeColor, 7, Qt::SolidLine, Qt::RoundCap);
    p.setPen(activePen);
    p.drawArc(cx - r + 14, cy - r + 14, (r - 14) * 2, (r - 14) * 2, 225 * 16, activeSpan);

    // Ticks & Labels around scale (every 10 hPa)
    for (int pVal = 960; pVal <= 1050; pVal += 10) {
        double tRatio = (pVal - pMin) / (pMax - pMin);
        double angleDeg = 225.0 - (270.0 * tRatio);
        double rad = angleDeg * M_PI / 180.0;

        int tLen = (pVal % 20 == 0) ? 7 : 4;
        int x1 = cx + int((r - 14) * std::cos(rad));
        int y1 = cy - int((r - 14) * std::sin(rad));
        int x2 = cx + int((r - 14 - tLen) * std::cos(rad));
        int y2 = cy - int((r - 14 - tLen) * std::sin(rad));

        p.setPen(QPen(pVal == 1010 ? QColor("#ffffff") : QColor("#475569"), pVal == 1010 ? 2 : 1));
        p.drawLine(x1, y1, x2, y2);
    }

    // 1 ATM Marker Line (1013.25 hPa)
    double stdRatio = (1013.25 - pMin) / (pMax - pMin);
    double stdAngleDeg = 225.0 - (270.0 * stdRatio);
    double stdRad = stdAngleDeg * M_PI / 180.0;
    int sx1 = cx + int((r - 14) * std::cos(stdRad));
    int sy1 = cy - int((r - 14) * std::sin(stdRad));
    int sx2 = cx + int((r - 24) * std::cos(stdRad));
    int sy2 = cy - int((r - 24) * std::sin(stdRad));
    p.setPen(QPen(QColor("#facc15"), 2.5));
    p.drawLine(sx1, sy1, sx2, sy2);

    // Dial Needle Pointer
    double curAngleDeg = 225.0 - (270.0 * ratio);
    double curRad = curAngleDeg * M_PI / 180.0;
    int nx = cx + int((r - 20) * std::cos(curRad));
    int ny = cy - int((r - 20) * std::sin(curRad));

    QPen needlePen(QColor("#ffffff"), 2.5, Qt::SolidLine, Qt::RoundCap);
    p.setPen(needlePen);
    p.drawLine(cx, cy, nx, ny);

    // Center Metal Cap / Hub
    p.setPen(QPen(QColor("#38bdf8"), 1.5));
    p.setBrush(QColor("#0f172a"));
    p.drawEllipse(QPoint(cx, cy), 8, 8);
    p.setBrush(QColor("#00f2fe"));
    p.drawEllipse(QPoint(cx, cy), 3, 3);

    // Digital readout inside lower part of dial
    QFont font = p.font();
    font.setPointSize(12);
    font.setBold(true);
    p.setFont(font);
    p.setPen(activeColor);
    QString valStr = QString::number(m_pressure, 'f', 1);
    p.drawText(QRect(cx - 40, cy + 16, 80, 18), Qt::AlignCenter, valStr);

    font.setPointSize(7);
    font.setBold(false);
    p.setFont(font);
    p.setPen(QColor("#94a3b8"));
    p.drawText(QRect(cx - 40, cy + 32, 80, 14), Qt::AlignCenter, "hPa · 1.0 ATM");
}

/* =========================================================================
   THERMAL METER WIDGET (Thước Đo Nhiệt Độ Khí Quyển)
   ========================================================================= */
ThermalMeterWidget::ThermalMeterWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(160, 38);
    setMaximumHeight(46);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void ThermalMeterWidget::setTemperature(double tempC)
{
    m_temp = tempC;
    m_hasData = true;
    update();
}

void ThermalMeterWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int pad = 4;
    const int barW = w - pad * 2;
    const int barH = 10;
    const int barY = 20;

    // Range: 0°C to 50°C
    double ratio = (m_temp - 0.0) / 50.0;
    ratio = qBound(0.0, ratio, 1.0);

    // Label Top
    QFont f = p.font();
    f.setPointSize(8);
    f.setBold(true);
    p.setFont(f);
    p.setPen(QColor("#ff9100"));
    p.drawText(pad, 13, QStringLiteral("NHIỆT ĐỘ: %1 °C  (An toàn: < 40°C)").arg(QString::number(m_temp, 'f', 1)));

    // Track Background
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#0b1736"));
    p.drawRoundedRect(pad, barY, barW, barH, 5, 5);

    // Gradient Level Bar
    QLinearGradient grad(pad, 0, pad + barW, 0);
    grad.setColorAt(0.0, QColor("#38bdf8"));
    grad.setColorAt(0.35, QColor("#10b981"));
    grad.setColorAt(0.7, QColor("#f59e0b"));
    grad.setColorAt(1.0, QColor("#ef4444"));
    p.setBrush(grad);
    p.drawRoundedRect(pad, barY, int(barW * ratio), barH, 5, 5);

    // Indicator Marker Needle
    int curX = pad + int(barW * ratio);
    p.setPen(QPen(QColor("#ffffff"), 2));
    p.drawLine(curX, barY - 2, curX, barY + barH + 2);
}

namespace {
QString friendlyMetricTitle(const QString &key)
{
    if (key == "temperature_c") return QStringLiteral("Nhiệt độ khí quyển (°C)");
    if (key == "pressure_hpa") return QStringLiteral("Áp suất khí quyển (hPa)");
    if (key == "ir_detected") return QStringLiteral("Cảm biến chuyển động (IR)");
    if (key == "sound_vpp") return QStringLiteral("Âm thanh môi trường (Vpp)");
    if (key == "uv_index") return QStringLiteral("Chỉ số UV");
    if (key == "lux") return QStringLiteral("Cường độ ánh sáng (Lux)");
    if (key == "voltage_v") return QStringLiteral("Điện áp (V)");
    if (key == "current_a") return QStringLiteral("Dòng điện (A)");
    if (key == "flow_l_min") return QStringLiteral("Lưu lượng (L/m)");
    if (key == "total_liters") return QStringLiteral("Tổng nước (L)");
    return key;
}

QColor metricChartColor(const QString &key)
{
    if (key == "pressure_hpa") return QColor("#00f2fe");
    if (key == "temperature_c") return QColor("#ff7a18");
    if (key == "ir_detected") return QColor("#ff2d55");
    if (key == "sound_vpp") return QColor("#fbbf24");
    if (key == "uv_index") return QColor("#c084fc");
    if (key == "lux") return QColor("#facc15");
    if (key == "voltage_v") return QColor("#ffee58");
    if (key == "current_a") return QColor("#2dd4bf");
    if (key == "flow_l_min") return QColor("#06b6d4");
    if (key == "total_liters") return QColor("#3b82f6");
    return QColor("#818cf8");
}

QFrame *panel(const QString &name)
{
    auto *f = new QFrame;
    f->setObjectName(name);
    return f;
}

QLabel *label(const QString &text, const QString &name = {})
{
    auto *l = new QLabel(text);
    if (!name.isEmpty()) l->setObjectName(name);
    l->setWordWrap(true);
    return l;
}

QPushButton *button(const QString &text, const QString &name = {})
{
    auto *b = new QPushButton(text);
    if (!name.isEmpty()) b->setObjectName(name);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

bool isDeviceOnline(const QJsonObject &device)
{
    if (!device.value("online").toBool(false)) return false;

    const QString raw = device.value("last_seen_at").toString();
    if (raw.isEmpty()) return true;

    QDateTime last = QDateTime::fromString(raw, Qt::ISODateWithMs);
    if (!last.isValid()) last = QDateTime::fromString(raw, Qt::ISODate);
    if (!last.isValid()) return device.value("online").toBool(false);
    if (last.timeSpec() == Qt::LocalTime) last.setTimeSpec(Qt::UTC);

    return last.secsTo(QDateTime::currentDateTimeUtc()) <= 35;
}

void showCustomMessageBox(QWidget *parent, QMessageBox::Icon icon, const QString &title, const QString &text)
{
    QMessageBox msgBox(icon, title, text, QMessageBox::Ok, parent);
    msgBox.setStyleSheet(R"QSS(
        QMessageBox {
            background-color: #0c142b;
            border: 2px solid #00f2fe;
            border-radius: 12px;
        }
        QLabel {
            color: #ffffff;
            font-size: 13px;
            font-weight: 700;
            background: transparent;
            min-height: 40px;
            padding: 6px;
        }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #3b82f6);
            color: #030a1c;
            border: none;
            border-radius: 8px;
            padding: 6px 22px;
            font-weight: 900;
            font-size: 12px;
            min-width: 80px;
            min-height: 30px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #33f5ff, stop:1 #60a5fa);
        }
    )QSS");
    msgBox.exec();
}

bool showCustomQuestionBox(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox msgBox(QMessageBox::Question, title, text, QMessageBox::Yes | QMessageBox::No, parent);
    msgBox.setStyleSheet(R"QSS(
        QMessageBox {
            background-color: #0c142b;
            border: 2px solid #00f2fe;
            border-radius: 12px;
        }
        QLabel {
            color: #ffffff;
            font-size: 13px;
            font-weight: 700;
            background: transparent;
            min-height: 40px;
            padding: 6px;
        }
        QPushButton {
            background: #1e293b;
            color: #ffffff;
            border: 1px solid #334155;
            border-radius: 8px;
            padding: 6px 22px;
            font-weight: 900;
            font-size: 12px;
            min-width: 80px;
            min-height: 30px;
        }
        QPushButton:hover {
            background: #334155;
        }
    )QSS");
    return (msgBox.exec() == QMessageBox::Yes);
}

}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Lê Nam - Hệ Thống Giám Sát Áp Suất Khí Quyển & Chuyển Động (Real-time IoT)"));
    resize(800, 480);
    setMinimumSize(780, 440);

    m_root = new QStackedWidget(this);
    setCentralWidget(m_root);

    const QString appStyle = R"QSS(
        QMainWindow, QWidget#shell { background-color: #060b17; color: #ecf2ff; }
        QWidget { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", sans-serif; }
        QLabel { color: #ecf2ff; background: transparent; }
        
        /* === DIALOGS & POPUPS === */
        QDialog, QMessageBox {
            background-color: #091024;
            color: #ecf2ff;
        }
        QMessageBox {
            background-color: #091024;
            border: 2px solid #00f2fe;
            border-radius: 12px;
        }
        
        /* === LOGIN PAGE HUD === */
        QFrame#loginWrap { background: qradialgradient(cx:0.3, cy:0.25, radius:1.2, stop:0 #112048, stop:0.55 #081024, stop:1 #040814); }
        QFrame#heroCard { background: #0c1735; border: 1.5px solid #203468; border-radius: 12px; }
        QFrame#loginCard { background: #0c1735; border: 1.5px solid #00f2fe; border-radius: 12px; }
        QLabel#stationTag { background: rgba(0, 242, 254, 0.15); color: #00f2fe; border: 1px solid rgba(0, 242, 254, 0.4); border-radius: 6px; padding: 3px 8px; font-size: 10px; font-weight: 800; }
        QLabel#statusReadyTag { background: rgba(16, 185, 129, 0.2); color: #10b981; border: 1px solid rgba(16, 185, 129, 0.4); border-radius: 6px; padding: 3px 8px; font-size: 10px; font-weight: 800; }
        QLabel#loginHeroTitle { color: #ffffff; font-size: 16px; font-weight: 900; line-height: 22px; }
        QLabel#loginHeroSub { color: #94a3b8; font-size: 11px; font-weight: 600; line-height: 15px; }
        QLabel#featureTag { background: #14224c; color: #38bdf8; border: 1px solid #283e7a; border-radius: 6px; padding: 3px 6px; font-size: 9px; font-weight: 800; }
        QLabel#loginCardTitle { color: #ffffff; font-size: 14px; font-weight: 900; }
        QLabel#loginCardSub { color: #64748b; font-size: 10px; font-weight: 600; }
        
        /* === INPUTS & CONTROLS === */
        QLineEdit, QComboBox { background: #080e22; color: #ffffff; border: 1.5px solid #1e3060; border-radius: 8px; padding: 5px 10px; min-height: 26px; font-size: 12px; font-weight: 700; }
        QLineEdit:focus, QComboBox:focus { border: 1.5px solid #00f2fe; background: #0e1a3d; }
        QPushButton { background: #1a2744; border: 1px solid #2a3d68; border-radius: 8px; color: white; padding: 6px 12px; font-weight: 900; font-size: 11px; }
        QPushButton:hover { background: #2a3d68; }
        QPushButton:pressed { background: #0f182d; }
        QPushButton#primaryBtn { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff7a18, stop:1 #7c5cff); border: none; }
        QPushButton#primaryBtn:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff6600, stop:1 #6742f5); }
        QPushButton#cyanBtn { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #3b82f6); color: #030919; border: none; }
        QPushButton#cyanBtn:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #33f5ff, stop:1 #60a5fa); }
        QPushButton#ghost { background: rgba(255,255,255,0.06); border: 1px solid rgba(255,255,255,0.15); color: #d4e0ff; }
        QPushButton#ghost:hover { background: rgba(255,255,255,0.12); }
        QPushButton#danger { background: #e11d48; border: none; }
        QPushButton#danger:hover { background: #be123c; }
        QPushButton#kbToggleBtn { background: rgba(0, 242, 254, 0.15); border: 1px solid rgba(0, 242, 254, 0.35); color: #00f2fe; font-size: 11px; padding: 3px 8px; border-radius: 6px; }

        /* === TOP HUD HEADER === */
        QFrame#topHud {
            background: #091229;
            border: 1.5px solid #1c2e5d;
            border-radius: 8px;
        }
        QLabel#hudBrandTitle { font-size: 11px; font-weight: 900; color: #00f2fe; }
        QLabel#hudAuthorTag { font-size: 8px; font-weight: 700; color: #94a3b8; }
        
        QPushButton#hudNav {
            background: transparent;
            color: #94a3b8;
            border-radius: 6px;
            padding: 4px 8px;
            font-size: 10px;
            font-weight: 800;
            border: 1px solid transparent;
        }
        QPushButton#hudNav:hover {
            background: rgba(0, 242, 254, 0.08);
            color: #ffffff;
            border: 1px solid rgba(0, 242, 254, 0.25);
        }
        QPushButton#hudNav:checked {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #3b82f6);
            color: #03081a;
            font-weight: 900;
            border: none;
        }

        QLabel#hudClock { color: #00f2fe; font-size: 10px; font-weight: 800; background: #070e22; border: 1px solid #16264f; border-radius: 6px; padding: 2px 6px; }
        QLabel#hudProto { color: #38bdf8; font-size: 9px; font-weight: 800; }
        QPushButton#hudLogout { background: rgba(225, 29, 72, 0.18); border: 1px solid rgba(225, 29, 72, 0.45); color: #fb7185; font-size: 9.5px; padding: 3px 8px; border-radius: 6px; font-weight: 800; }
        QPushButton#hudLogout:hover { background: #e11d48; color: white; }

        /* === TELEMETRY DASHBOARD PANELS === */
        QFrame#telemetryCard {
            background-color: #0a142c;
            border: 1.5px solid #1e3060;
            border-radius: 10px;
        }
        QFrame#telemetryCard:hover {
            border: 1.5px solid #00f2fe;
        }
        QFrame#motionSafeCard {
            background-color: rgba(16, 185, 129, 0.09);
            border: 1.5px solid #10b981;
            border-radius: 8px;
        }
        QFrame#motionAlertCard {
            background-color: rgba(255, 23, 68, 0.22);
            border: 2px solid #ff1744;
            border-radius: 8px;
        }
        QFrame#stationPodCard {
            background-color: #0c1836;
            border: 1.5px solid #24396f;
            border-radius: 8px;
        }
        
        /* === LABELS & VALUES === */
        QLabel#panelHeader { font-size: 12px; font-weight: 900; color: #00f2fe; text-transform: uppercase; }
        QLabel#metricName { font-size: 11px; font-weight: 800; color: #cbd5e1; }
        QLabel#metricZone { font-size: 10px; font-weight: 800; color: #38bdf8; background: #132452; border: 1px solid #273f7c; border-radius: 4px; padding: 2px 6px; }
        QLabel#metricValTemp { font-size: 26px; font-weight: 900; color: #ff9100; }
        QLabel#metricValPressure { font-size: 26px; font-weight: 900; color: #00f2fe; }
        QLabel#metricValSub { font-size: 11px; font-weight: 700; color: #93c5fd; }
        QLabel#motionAlertText { font-size: 13px; font-weight: 900; color: #ff1744; }
        QLabel#motionSafeText { font-size: 13px; font-weight: 900; color: #10b981; }
        QLabel#kpiVal { font-size: 15px; font-weight: 900; color: #ffffff; }
        QLabel#kpiTitle { font-size: 10px; font-weight: 800; color: #38bdf8; text-transform: uppercase; }
        QLabel#onlineBadge { background: #10b981; color: #ffffff; border-radius: 5px; padding: 3px 8px; font-size: 10px; font-weight: 900; }
        QLabel#offlineBadge { background: #64748b; color: #ffffff; border-radius: 5px; padding: 3px 8px; font-size: 10px; font-weight: 900; }
        QLabel#pageTitle { font-size: 13px; font-weight: 900; color: #00f2fe; }

        /* === DEVICE CARDS & CHIPS === */
        QFrame#deviceCard {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #0f1d3d, stop:1 #09132b);
            border: 1.5px solid #223c72;
            border-radius: 12px;
        }
        QFrame#deviceCard:hover {
            background-color: #12244c;
            border: 2px solid #00f2fe;
        }
        QFrame#availableCard {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #092138, stop:1 #061727);
            border: 1.5px solid #00f2fe;
            border-radius: 12px;
        }
        QFrame#availableCard:hover {
            border: 2px solid #38bdf8;
            background-color: #0d2c4b;
        }
        QFrame#discoveryRadarCard {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #08162f, stop:1 #061124);
            border: 1.5px dashed #00f2fe;
            border-radius: 12px;
        }
        QLabel#nodeIconBadge {
            background: #102452;
            color: #00f2fe;
            border: 1px solid #2a488e;
            border-radius: 6px;
            padding: 4px 8px;
            font-weight: 900;
            font-size: 11px;
        }
        QLabel#discoveredBadge {
            background: #1e3a1f;
            color: #10b981;
            border: 1px solid #10b981;
            border-radius: 6px;
            padding: 4px 8px;
            font-weight: 900;
            font-size: 11px;
        }
        QLabel#deviceName { font-size: 14px; font-weight: 900; color: #ffffff; }
        
        QFrame#metricChip {
            background: #070e22;
            border: 1px solid #1c3262;
            border-radius: 8px;
        }
        QLabel#chipTitle { font-size: 10px; font-weight: 900; color: #7dd3fc; text-transform: uppercase; }
        QLabel#chipValPressure { font-size: 14px; font-weight: 900; color: #00f2fe; }
        QLabel#chipValTemp { font-size: 14px; font-weight: 900; color: #ff9100; }
        QLabel#chipValMotionAlert { font-size: 12px; font-weight: 900; color: #ff1744; }
        QLabel#chipValMotionSafe { font-size: 12px; font-weight: 900; color: #10b981; }
        QLabel#chipSub { font-size: 9px; font-weight: 700; color: #94a3b8; }

        QPushButton#cardConfigBtn {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #3b82f6);
            color: #030818;
            border: none;
            border-radius: 6px;
            font-weight: 900;
            font-size: 11px;
            padding: 6px 12px;
            min-height: 28px;
        }
        QPushButton#cardConfigBtn:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #33f5ff, stop:1 #60a5fa);
        }
        QPushButton#cardReleaseBtn {
            background: rgba(225, 29, 72, 0.18);
            border: 1px solid rgba(225, 29, 72, 0.5);
            color: #fb7185;
            border-radius: 6px;
            font-weight: 900;
            font-size: 11px;
            padding: 6px 12px;
            min-height: 28px;
        }
        QPushButton#cardReleaseBtn:hover {
            background: #e11d48;
            color: #ffffff;
        }
        QPushButton#addDeviceBtn {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff7a18, stop:1 #ff3e00);
            color: #ffffff;
            border: 1px solid #fed7aa;
            border-radius: 8px;
            font-weight: 900;
            font-size: 11px;
            padding: 6px 14px;
            min-height: 28px;
        }
        QPushButton#addDeviceBtn:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff9100, stop:1 #ea580c);
        }

        /* === TABLES & CHARTS === */
        QTableWidget { background: #091024; color: #ecf2ff; border: 1.5px solid #1c2d59; border-radius: 8px; gridline-color: rgba(255, 255, 255, 0.08); selection-background-color: #3b82f6; font-size: 10px; }
        QHeaderView::section { background: #0e1a3b; color: #93c5fd; border: none; padding: 5px; font-weight: 900; font-size: 10px; }
        QFrame#miniChartCard { background: #0a142c; border: 1.5px solid #1e3060; border-radius: 8px; }
        QLabel#chartTitle { font-size: 11px; font-weight: 900; color: #ffffff; }
        QLabel#chartValueBadge { background: rgba(0, 242, 254, 0.15); color: #00f2fe; border: 1px solid rgba(0, 242, 254, 0.4); border-radius: 4px; padding: 1px 5px; font-size: 9px; font-weight: 800; }
        QPushButton#toggleTab { background: rgba(255, 255, 255, 0.06); border: 1px solid #24396f; border-radius: 6px; padding: 4px 8px; font-size: 10px; font-weight: 800; color: #94a3b8; }
        QPushButton#toggleTab:hover { background: rgba(255, 255, 255, 0.12); color: #ffffff; }
        QPushButton#toggleTab:checked { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #3b82f6); color: #03081a; font-weight: 900; border: none; }
        QChartView { background: transparent; border: none; }
        QScrollArea { border: none; background: transparent; }
    )QSS";

    setStyleSheet(appStyle);
    if (qApp) qApp->setStyleSheet(appStyle);

    buildLogin();
    buildShell();
    m_root->addWidget(m_loginPage);
    m_root->addWidget(m_shellPage);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::refreshAll);
}

QNetworkRequest MainWindow::request(const QString &path) const
{
    QNetworkRequest req(QUrl(m_baseUrl + path));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setTransferTimeout(3500);
    if (!m_token.isEmpty()) req.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());
    return req;
}

void MainWindow::get(const QString &path, std::function<void(QJsonObject)> ok)
{
    auto *reply = m_net.get(request(path));
    connect(reply, &QNetworkReply::finished, this, [this, reply, ok] {
        const auto body = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) ok(QJsonDocument::fromJson(body).object());
        else if (m_status) m_status->setText(reply->errorString());
        reply->deleteLater();
    });
}

void MainWindow::post(const QString &path, const QJsonObject &body, std::function<void(QJsonObject)> ok)
{
    auto *reply = m_net.post(request(path), QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, ok] {
        const auto body = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) { if (ok) ok(QJsonDocument::fromJson(body).object()); }
        else showCustomMessageBox(this, QMessageBox::Warning, tr("Lỗi"), reply->errorString());
        reply->deleteLater();
    });
}

void MainWindow::del(const QString &path, std::function<void(QJsonObject)> ok)
{
    auto *reply = m_net.deleteResource(request(path));
    connect(reply, &QNetworkReply::finished, this, [this, reply, ok] {
        const auto body = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) { if (ok) ok(QJsonDocument::fromJson(body).object()); }
        else showCustomMessageBox(this, QMessageBox::Warning, tr("Lỗi"), reply->errorString());
        reply->deleteLater();
    });
}

void MainWindow::logout()
{
    m_timer->stop();
    m_token.clear();
    m_role.clear();
    m_username.clear();
    m_root->setCurrentWidget(m_loginPage);
    if (m_loginKeyboard) m_loginKeyboard->show();
}

void MainWindow::buildLogin()
{
    m_loginPage = panel("loginWrap");
    auto *mainVBox = new QVBoxLayout(m_loginPage);
    mainVBox->setContentsMargins(12, 10, 12, 8);
    mainVBox->setSpacing(6);

    // ---- TOP SECTION: Brand Hero Banner (Left) & Compact Cyber Login Pod (Right) ----
    auto *topRow = new QHBoxLayout;
    topRow->setSpacing(10);

    // Left Banner: Station Identity
    auto *heroCard = panel("heroCard");
    auto *heroL = new QVBoxLayout(heroCard);
    heroL->setContentsMargins(16, 12, 16, 12);
    heroL->setSpacing(6);
    
    auto *tagRow = new QHBoxLayout;
    tagRow->addWidget(label("[RADAR] LÊ NAM · TRẠM ĐIỀU HÀNH", "stationTag"));
    tagRow->addWidget(label("● SẴN SÀNG KẾT NỐI", "statusReadyTag"));
    tagRow->addStretch();
    heroL->addLayout(tagRow);

    heroL->addWidget(label("HỆ THỐNG GIÁM SÁT\nÁP SUẤT KHÍ QUYỂN & CHUYỂN ĐỘNG", "loginHeroTitle"));
    heroL->addWidget(label("Đề tài: Xây dựng hệ thống giám sát áp suất khí quyển và chuyển động theo thời gian thực\nSinh viên thực hiện: Lê Nam (ICTU)", "loginHeroSub"));

    auto *featureRow = new QHBoxLayout;
    featureRow->addWidget(label("[PRES] ÁP SUẤT", "featureTag"));
    featureRow->addWidget(label("[TEMP] NHIỆT ĐỘ", "featureTag"));
    featureRow->addWidget(label("[PIR] CHUYỂN ĐỘNG", "featureTag"));
    featureRow->addWidget(label("[MQTT] PORT 1883", "featureTag"));
    featureRow->addStretch();
    heroL->addLayout(featureRow);
    heroL->addStretch();

    // Right Card: Login Form Pod
    auto *card = panel("loginCard");
    card->setFixedWidth(330);
    auto *cl = new QVBoxLayout(card);
    cl->setContentsMargins(16, 10, 16, 10);
    cl->setSpacing(6);

    auto *cardHead = new QHBoxLayout;
    auto *cardTitleBox = new QVBoxLayout;
    cardTitleBox->setSpacing(0);
    cardTitleBox->addWidget(label("XÁC THỰC TRẠM", "loginCardTitle"));
    cardTitleBox->addWidget(label("Đăng nhập quyền quản trị & giám sát", "loginCardSub"));
    cardHead->addLayout(cardTitleBox, 1);

    auto *kbToggle = button("[PHÍM]", "kbToggleBtn");
    cardHead->addWidget(kbToggle);
    cl->addLayout(cardHead);

    auto *u = new QLineEdit;
    u->setPlaceholderText("Tài khoản (admin/user)");
    u->setText("admin");

    auto *p = new QLineEdit;
    p->setPlaceholderText("Mật khẩu (password)");
    p->setEchoMode(QLineEdit::Password);
    p->setText("admin");

    auto *passRow = new QHBoxLayout;
    passRow->setSpacing(4);
    passRow->addWidget(p, 1);
    auto *eyeBtn = button("[XEM]", "ghost");
    eyeBtn->setFixedSize(50, 28);
    eyeBtn->setToolTip(tr("Xem/Ẩn mật khẩu"));
    connect(eyeBtn, &QPushButton::clicked, this, [p] {
        p->setEchoMode(p->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password);
    });
    passRow->addWidget(eyeBtn);

    auto *loginBtn = button("TRUY CẬP TRẠM GIÁM SÁT >>", "cyanBtn");
    loginBtn->setFixedHeight(32);

    cl->addWidget(u);
    cl->addLayout(passRow);
    cl->addWidget(loginBtn);

    topRow->addWidget(heroCard, 1);
    topRow->addWidget(card);
    mainVBox->addLayout(topRow, 1);

    // ---- BOTTOM SECTION: Built-in Touchscreen Virtual Keyboard ----
    m_loginKeyboard = new VirtualKeyboard(m_loginPage);
    m_loginKeyboard->attachTo(u);
    mainVBox->addWidget(m_loginKeyboard, 0);

    connect(kbToggle, &QPushButton::clicked, this, [this] {
        m_loginKeyboard->setVisible(!m_loginKeyboard->isVisible());
    });

    connect(qApp, &QApplication::focusChanged, this, [this, u, p](QWidget *, QWidget *now) {
        if (now == u) {
            m_loginKeyboard->attachTo(u);
            m_loginKeyboard->show();
        } else if (now == p) {
            m_loginKeyboard->attachTo(p);
            m_loginKeyboard->show();
        }
    });

    auto doLogin = [=] {
        triggerLogin(u->text(), p->text(), 0);
    };

    connect(loginBtn, &QPushButton::clicked, this, doLogin);
    connect(p, &QLineEdit::returnPressed, this, doLogin);
    connect(m_loginKeyboard, &VirtualKeyboard::enterPressed, this, doLogin);
}

void MainWindow::triggerLogin(const QString &username, const QString &password, int targetPage)
{
    post("/api/auth/login", {{"username", username.trimmed()}, {"password", password}}, [=](QJsonObject obj) {
        m_token = obj.value("token").toString();
        const auto user = obj.value("user").toObject();
        m_role = user.value("role").toString();
        m_username = user.value("username").toString(username);
        const bool isAdmin = (m_role == "admin");
        const auto navButtons = m_shellPage->findChildren<QPushButton*>("hudNav");
        for (int i = 0; i < navButtons.size(); ++i) {
            auto *nav = navButtons[i];
            if (nav->text().contains(QStringLiteral("QUẢN TRỊ")) || nav->text().contains(QStringLiteral("TÀI KHOẢN")) || nav->text().contains(QStringLiteral("NHẬT KÝ"))) {
                nav->setVisible(true);
                nav->setEnabled(true);
                if (isAdmin) {
                    nav->setText(QStringLiteral("■ QUẢN TRỊ & NHẬT KÝ"));
                    nav->setToolTip(QStringLiteral("Quản lý tài khoản, lịch sử đăng nhập & điều khiển"));
                } else {
                    nav->setText(QStringLiteral("■ NHẬT KÝ THAO TÁC"));
                    nav->setToolTip(QStringLiteral("Xem lịch sử các thao tác của bạn trong hệ thống"));
                }
            }
            nav->setChecked(i == targetPage);
        }
        setPage(targetPage);
        m_root->setCurrentWidget(m_shellPage);
        m_timer->start(2000);
        refreshAll();
    });
}

void MainWindow::setInitialHistoryDate(const QDate &d)
{
    if (d.isValid()) {
        m_historyDate = d;
        updateHistoryDateDisplay();
    }
}

void MainWindow::setHistorySubTab(int index)
{
    if (index == 0 && m_histBtnPressure) m_histBtnPressure->click();
    else if (index == 1 && m_histBtnMotion) m_histBtnMotion->click();
    else if (index == 2 && m_histBtnTemp) m_histBtnTemp->click();
    else if (index == 3 && m_histBtnMulti) m_histBtnMulti->click();
    else if (index == 4 && m_histBtnTable) m_histBtnTable->click();
    else if (m_historyStack && index >= 0 && index < m_historyStack->count()) {
        m_historyStack->setCurrentIndex(index);
    }
}

void MainWindow::buildShell()
{
    m_shellPage = panel("shell");
    auto *root = new QVBoxLayout(m_shellPage);
    root->setContentsMargins(6, 6, 6, 6);
    root->setSpacing(6);

    // === TOP AERO HUD HEADER BAR (Height: 42px - Perfectly fitted for 7-inch 800x480) ===
    auto *topHud = panel("topHud");
    topHud->setFixedHeight(42);
    auto *hl = new QHBoxLayout(topHud);
    hl->setContentsMargins(8, 2, 8, 2);
    hl->setSpacing(6);

    // Left Brand Badge
    auto *brandBox = new QVBoxLayout;
    brandBox->setSpacing(0);
    auto *bTitle = label("[RADAR] LÊ NAM", "hudBrandTitle");
    bTitle->setWordWrap(false);
    auto *bSub = label("Khí Quyển & C.Động", "hudAuthorTag");
    bSub->setWordWrap(false);
    brandBox->addWidget(bTitle);
    brandBox->addWidget(bSub);
    hl->addLayout(brandBox);
    hl->addSpacing(4);

    // Center Nav Capsule Buttons (Compact for 7 inch screen)
    const QStringList navNames{
        QStringLiteral("● QUAN TRẮC"),
        QStringLiteral("◆ TRẠM ĐO"),
        QStringLiteral("▲ ĐỒ THỊ"),
        QStringLiteral("■ QUẢN TRỊ")
    };
    for (int i = 0; i < navNames.size(); ++i) {
        auto *b = button(navNames[i], "hudNav");
        b->setCheckable(true);
        b->setFixedHeight(30);
        if (i == 0) b->setChecked(true);
        connect(b, &QPushButton::clicked, this, [this, i, b] {
            setPage(i);
            const auto navButtons = m_shellPage->findChildren<QPushButton*>("hudNav");
            for (auto *btn : navButtons) {
                btn->setChecked(btn == b);
            }
        });
        hl->addWidget(b);
    }
    hl->addStretch();

    // Node & Network Status Pills in Top HUD
    m_kpiDevices = label("1 TRẠM", "hudClock");
    m_kpiOnline = label("ONLINE", "onlineBadge");
    m_kpiType = label("REALTIME", "metricZone");
    m_kpiType->hide();
    hl->addWidget(m_kpiDevices);
    hl->addWidget(m_kpiOnline);

    // Right Health Status
    m_status = label("● LIVE", "hudClock");
    hl->addWidget(m_status);

    auto *fsBtn = button("[F11]", "ghost");
    fsBtn->setFixedHeight(28);
    fsBtn->setFixedWidth(44);
    fsBtn->setToolTip(tr("Bật/Tắt toàn màn hình (Khớp màn hình 7 inch)"));
    connect(fsBtn, &QPushButton::clicked, this, [this] {
        if (isFullScreen()) showNormal();
        else showFullScreen();
    });
    hl->addWidget(fsBtn);

    auto *outBtn = button("THOÁT", "hudLogout");
    outBtn->setFixedHeight(28);
    connect(outBtn, &QPushButton::clicked, this, &MainWindow::logout);
    hl->addWidget(outBtn);

    root->addWidget(topHud);

    // === MAIN MULTI-PAGE STACK ===
    m_pages = new QStackedWidget;
    buildHome();
    buildDevices();
    buildHistory();
    buildUsers();
    root->addWidget(m_pages, 1);
}

void MainWindow::setPage(int index)
{
    m_pages->setCurrentIndex(index);
    if (index == 3) {
        const bool isAdmin = (m_role == "admin");
        if (m_tabUsersBtn) m_tabUsersBtn->setVisible(isAdmin);
        if (m_tabLoginBtn) m_tabLoginBtn->setVisible(isAdmin);
        if (m_addUserBtn) m_addUserBtn->setVisible(isAdmin);
        if (!isAdmin && m_usersStack) {
            m_usersStack->setCurrentIndex(2);
            if (m_tabAuditBtn) m_tabAuditBtn->setChecked(true);
        }
    }
    refreshAll();
}

void MainWindow::buildHome()
{
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_homeStack = new QStackedWidget;

    // View 0: Live Telemetry 3-Sector Cockpit (Full Height)
    m_homeLiveView = new QWidget;
    auto *liveLayout = new QHBoxLayout(m_homeLiveView);
    liveLayout->setContentsMargins(0, 0, 0, 0);
    liveLayout->setSpacing(8);

    // =========================================================================
    // SECTOR 1 (TRÁI): ÁP SUẤT KHÍ QUYỂN (BMP180 - BAROMETER)
    // =========================================================================
    auto *baroCard = panel("telemetryCard");
    auto *bcl = new QVBoxLayout(baroCard);
    bcl->setContentsMargins(10, 8, 10, 8);
    bcl->setSpacing(6);

    auto *bHead = new QHBoxLayout;
    bHead->addWidget(label("ÁP SUẤT KHÍ QUYỂN (BMP180)", "panelHeader"));
    bHead->addStretch();
    bHead->addWidget(label("CHUẨN 1.0 ATM", "metricZone"));
    bcl->addLayout(bHead);

    // Precision Dial Widget
    m_baroDial = new BarometerDialWidget(baroCard);
    bcl->addWidget(m_baroDial, 0, Qt::AlignCenter);

    auto *pBox = panel("stationPodCard");
    auto *pbl = new QVBoxLayout(pBox);
    pbl->setContentsMargins(10, 6, 10, 6);
    pbl->setSpacing(3);
    m_livePressure = label("-- hPa", "metricValPressure");
    pbl->addWidget(m_livePressure);
    pbl->addWidget(label("Khí áp tiêu chuẩn: 1013.25 hPa · BMP180", "metricValSub"));
    pbl->addWidget(label("Độ cao ước tính: ~ 82 m · Áp suất ổn định", "metricValSub"));
    bcl->addWidget(pBox);
    bcl->addStretch();

    // =========================================================================
    // SECTOR 2 (GIỮA): RADAR GIÁM SÁT CHUYỂN ĐỘNG (IR HC-SR501)
    // =========================================================================
    auto *motionCard = panel("telemetryCard");
    auto *mcl = new QVBoxLayout(motionCard);
    mcl->setContentsMargins(10, 8, 10, 8);
    mcl->setSpacing(6);

    auto *mHead = new QHBoxLayout;
    mHead->addWidget(label("RADAR QUÉT CHUYỂN ĐỘNG", "panelHeader"));
    mHead->addStretch();
    m_liveStationStatus = label("ONLINE", "onlineBadge");
    mHead->addWidget(m_liveStationStatus);
    mcl->addLayout(mHead);

    // Tactical Radar Scanner
    m_radarWidget = new RadarScanWidget(motionCard);
    mcl->addWidget(m_radarWidget, 0, Qt::AlignCenter);

    // Motion Alert Box
    m_liveMotionCard = panel("motionSafeCard");
    auto *macL = new QVBoxLayout(m_liveMotionCard);
    macL->setContentsMargins(10, 8, 10, 8);
    macL->setSpacing(3);
    m_liveMotion = label("[OK] VÙNG AN TOÀN · KHÔNG CÓ CHUYỂN ĐỘNG", "motionSafeText");
    m_liveMotionSub = label("Cảm biến hồng ngoại IR đang quét liên tục", "metricValSub");
    macL->addWidget(m_liveMotion);
    macL->addWidget(m_liveMotionSub);
    mcl->addWidget(m_liveMotionCard);
    mcl->addStretch();

    // =========================================================================
    // SECTOR 3 (PHẢI): NHIỆT ĐỘ KHÍ QUYỂN & ĐIỀU HÀNH TRẠM
    // =========================================================================
    auto *ctrlCard = panel("telemetryCard");
    auto *ccl = new QVBoxLayout(ctrlCard);
    ccl->setContentsMargins(10, 8, 10, 8);
    ccl->setSpacing(6);

    auto *cHead = new QHBoxLayout;
    cHead->addWidget(label("NHIỆT ĐỘ & ĐIỀU HÀNH TRẠM", "panelHeader"));
    cHead->addStretch();
    ccl->addLayout(cHead);

    // Temperature Pod
    auto *tBox = panel("stationPodCard");
    auto *tbl = new QVBoxLayout(tBox);
    tbl->setContentsMargins(10, 6, 10, 6);
    tbl->setSpacing(3);
    m_liveTemp = label("-- °C", "metricValTemp");
    tbl->addWidget(m_liveTemp);
    tbl->addWidget(label("Cảm biến nhiệt độ khí quyển BMP180", "metricValSub"));
    ccl->addWidget(tBox);

    // Thermal Meter Bar
    m_thermalMeter = new ThermalMeterWidget(ctrlCard);
    ccl->addWidget(m_thermalMeter);

    // Station Info Box & Actions
    auto *infoBox = panel("stationPodCard");
    auto *ibl = new QVBoxLayout(infoBox);
    ibl->setContentsMargins(10, 6, 10, 6);
    ibl->setSpacing(6);

    auto *stnRow = new QHBoxLayout;
    m_liveStationName = label("Trạm Khí Quyển & Chuyển Động", "deviceName");
    m_liveStationId = label("ID: --", "metricValSub");
    stnRow->addWidget(m_liveStationName);
    stnRow->addStretch();
    stnRow->addWidget(m_liveStationId);
    ibl->addLayout(stnRow);

    auto *actRow = new QHBoxLayout;
    m_liveConfigBtn = button("CÀI ĐẶT NGƯỠNG", "cyanBtn");
    m_liveConfigBtn->setFixedHeight(32);
    m_liveReleaseBtn = button("Gỡ trạm", "danger");
    m_liveReleaseBtn->setFixedHeight(32);
    actRow->addWidget(m_liveConfigBtn, 1);
    actRow->addWidget(m_liveReleaseBtn);
    ibl->addLayout(actRow);
    ccl->addWidget(infoBox);
    ccl->addStretch();

    liveLayout->addWidget(baroCard, 1);
    liveLayout->addWidget(motionCard, 1);
    liveLayout->addWidget(ctrlCard, 1);
    m_homeStack->addWidget(m_homeLiveView);

    // View 1: Empty View
    m_homeEmptyView = panel("telemetryCard");
    auto *empL = new QVBoxLayout(m_homeEmptyView);
    empL->setContentsMargins(20, 20, 20, 20);
    empL->setSpacing(8);
    empL->setAlignment(Qt::AlignCenter);

    auto *empTitle = label("Chưa có trạm khí quyển & chuyển động nào được liên kết", "loginHeroTitle");
    empTitle->setAlignment(Qt::AlignCenter);
    auto *empSub = label("Hãy chuyển sang tab 'TRẠM CẢM BIẾN' để ghép nối thiết bị đo vào hệ thống", "loginHeroSub");
    empSub->setAlignment(Qt::AlignCenter);
    auto *gotoDevBtn = button("Chuyển đến Quản Lý Trạm Cảm Biến >>", "primaryBtn");
    gotoDevBtn->setFixedWidth(280);
    gotoDevBtn->setFixedHeight(36);
    connect(gotoDevBtn, &QPushButton::clicked, this, [this] {
        setPage(1);
        const auto navButtons = m_shellPage->findChildren<QPushButton*>("hudNav");
        for (int i = 0; i < navButtons.size(); ++i) {
            navButtons[i]->setChecked(i == 1);
        }
    });

    empL->addWidget(empTitle);
    empL->addWidget(empSub);
    empL->addWidget(gotoDevBtn, 0, Qt::AlignCenter);
    m_homeStack->addWidget(m_homeEmptyView);

    root->addWidget(m_homeStack, 1);
    m_pages->addWidget(page);
}

void MainWindow::buildDevices()
{
    auto *page = new QWidget;
    page->setStyleSheet("background-color: #060b17;");
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(2, 2, 2, 2);
    root->setSpacing(6);

    // 1. Top Action Strip (Height: 34px - Clean, single-line, zero overlap)
    auto *topBanner = new QFrame;
    topBanner->setObjectName("devicesActionBar");
    topBanner->setStyleSheet(
        "QFrame#devicesActionBar { background: #08122a; border: 1px solid #1a3260; border-radius: 8px; }"
    );
    topBanner->setFixedHeight(34);
    auto *tbl = new QHBoxLayout(topBanner);
    tbl->setContentsMargins(10, 2, 8, 2);
    tbl->setSpacing(8);

    auto *bTitle = new QLabel("🛰 MẠNG LƯỚI TRẠM ĐO ÁP SUẤT & CHUYỂN ĐỘNG");
    bTitle->setWordWrap(false);
    bTitle->setStyleSheet("color: #00f2fe; font-size: 11px; font-weight: 900; letter-spacing: 0.5px;");
    tbl->addWidget(bTitle);

    tbl->addStretch();

    auto *scanBtn = new QPushButton("⟳ Quét Mạng");
    scanBtn->setFixedHeight(24);
    scanBtn->setCursor(Qt::PointingHandCursor);
    scanBtn->setStyleSheet("QPushButton { background: #132752; color: #38bdf8; border: 1px solid #234584; border-radius: 4px; padding: 2px 10px; font-size: 9.5px; font-weight: bold; } QPushButton:hover { background: #1a3875; color: #ffffff; }");
    connect(scanBtn, &QPushButton::clicked, this, &MainWindow::refreshDevices);
    tbl->addWidget(scanBtn);

    auto *manualAddBtn = new QPushButton("+ Nhập ID");
    manualAddBtn->setFixedHeight(24);
    manualAddBtn->setCursor(Qt::PointingHandCursor);
    manualAddBtn->setStyleSheet("QPushButton { background: #132752; color: #94a3b8; border: 1px solid #234584; border-radius: 4px; padding: 2px 10px; font-size: 9.5px; font-weight: bold; } QPushButton:hover { background: #1a3875; color: #ffffff; }");
    connect(manualAddBtn, &QPushButton::clicked, this, &MainWindow::openAddDeviceDialog);
    tbl->addWidget(manualAddBtn);

    auto *addBtn = new QPushButton("+ THÊM TRẠM MỚI");
    addBtn->setFixedHeight(24);
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setStyleSheet("QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #2563eb); color: #020917; border: none; border-radius: 4px; padding: 2px 12px; font-size: 9.5px; font-weight: 900; } QPushButton:hover { background: #38bdf8; }");
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::openAddDeviceDialog);
    tbl->addWidget(addBtn);

    root->addWidget(topBanner);

    // 2. Main 2-Column Cockpit Layout (Height ~385px, Zero Scrollbar!)
    auto *body = new QHBoxLayout;
    body->setSpacing(8);

    // =========================================================================
    // LEFT COCKPIT: TRẠM ĐANG HOẠT ĐỘNG (Flex 4)
    // =========================================================================
    auto *leftPanel = new QFrame;
    leftPanel->setObjectName("deviceCockpitLeft");
    leftPanel->setStyleSheet(
        "QFrame#deviceCockpitLeft { background: #08132b; border: 1.5px solid #1e3a6c; border-radius: 10px; }"
    );
    auto *lpLay = new QVBoxLayout(leftPanel);
    lpLay->setContentsMargins(10, 8, 10, 8);
    lpLay->setSpacing(6);

    m_devLeftStack = new QStackedWidget(leftPanel);

    // View 0: Active Station View
    m_devLeftActiveView = new QWidget;
    auto *actLay = new QVBoxLayout(m_devLeftActiveView);
    actLay->setContentsMargins(0, 0, 0, 0);
    actLay->setSpacing(6);

    // Row 1: Header of Active Station (Station Selector + Badges)
    auto *stHead = new QHBoxLayout;
    stHead->setSpacing(6);

    m_devSelectCombo = new QComboBox;
    m_devSelectCombo->setFixedHeight(26);
    m_devSelectCombo->setMinimumWidth(140);
    m_devSelectCombo->setStyleSheet(
        "QComboBox { background: #0f1f44; color: #ffffff; border: 1.5px solid #284c8a; border-radius: 6px; padding: 1px 6px; font-size: 10.5px; font-weight: bold; }"
        "QComboBox:hover { border-color: #00f2fe; }"
    );
    stHead->addWidget(m_devSelectCombo, 1);

    m_devActiveId = label("MÃ: --", "statusReadyTag");
    m_devActiveId->setFixedHeight(26);
    m_devActiveId->setStyleSheet("background: #0d244f; color: #00f2fe; border: 1px solid #1a4282; border-radius: 6px; padding: 1px 6px; font-size: 9.5px; font-weight: 900;");
    stHead->addWidget(m_devActiveId);

    m_devActiveStatus = label("● ONLINE", "onlineBadge");
    m_devActiveStatus->setFixedHeight(26);
    stHead->addWidget(m_devActiveStatus);
    actLay->addLayout(stHead);

    // Row 2: Sub-badge Hardware & Architecture Specs
    auto *specsRow = new QHBoxLayout;
    specsRow->setSpacing(4);
    auto *hwBadge = new QLabel("BOARD: ESP32-S3 Xtensa Dual-Core · 16MB Flash · FreeRTOS v2.4");
    hwBadge->setWordWrap(false);
    hwBadge->setStyleSheet("color: #64748b; font-size: 8px; font-weight: bold;");
    specsRow->addWidget(hwBadge);
    specsRow->addStretch();
    actLay->addLayout(specsRow);

    // Row 3: 3 Distinct Cyber Telemetry Pods (Horizontal)
    auto *podsLay = new QHBoxLayout;
    podsLay->setSpacing(5);

    auto makePod = [](const QString &title, const QString &accentColor, QLabel *&valLbl, const QString &initialVal, const QString &subText) {
        auto *pod = new QFrame;
        pod->setStyleSheet(QStringLiteral(
            "QFrame { background: #050e24; border: 1.5px solid %1; border-radius: 8px; }"
        ).arg(accentColor));
        auto *pl = new QVBoxLayout(pod);
        pl->setContentsMargins(6, 4, 6, 4);
        pl->setSpacing(1);

        auto *t = new QLabel(title);
        t->setWordWrap(false);
        t->setStyleSheet(QStringLiteral("color: #94a3b8; font-size: 7.5px; font-weight: 900; text-transform: uppercase;"));
        pl->addWidget(t);

        valLbl = new QLabel(initialVal);
        valLbl->setWordWrap(false);
        valLbl->setStyleSheet(QStringLiteral("color: %1; font-size: 14px; font-weight: 900;").arg(accentColor));
        pl->addWidget(valLbl);

        auto *s = new QLabel(subText);
        s->setWordWrap(false);
        s->setStyleSheet("color: #64748b; font-size: 7.5px; font-weight: 600;");
        pl->addWidget(s);
        return pod;
    };

    podsLay->addWidget(makePod("ÁP SUẤT (BMP180)", "#00f2fe", m_devActivePressure, "1013.2 hPa", "Chuẩn 1013 hPa"), 1);
    podsLay->addWidget(makePod("NHIỆT ĐỘ (BMP180)", "#ff9100", m_devActiveTemp, "29.5 °C", "Khoảng an toàn"), 1);
    podsLay->addWidget(makePod("CHUYỂN ĐỘNG IR", "#10b981", m_devActiveMotion, "AN TOÀN", "Bảo vệ 24/7 (PIR)"), 1);
    actLay->addLayout(podsLay);

    // Row 4: Station Configuration & Threshold Grid (Fills cockpit beautifully)
    auto *matrixFrame = new QFrame;
    matrixFrame->setStyleSheet("QFrame { background: #060e22; border: 1px solid #182e56; border-radius: 8px; }");
    auto *ml = new QGridLayout(matrixFrame);
    ml->setContentsMargins(8, 6, 8, 6);
    ml->setHorizontalSpacing(10);
    ml->setVerticalSpacing(4);

    auto makeCell = [](const QString &lbl, const QString &val, const QString &valColor = "#ffffff") {
        auto *vbox = new QVBoxLayout;
        vbox->setSpacing(1);
        auto *l = new QLabel(lbl);
        l->setWordWrap(false);
        l->setStyleSheet("color: #64748b; font-size: 7.5px; font-weight: bold;");
        auto *v = new QLabel(val);
        v->setWordWrap(false);
        v->setStyleSheet(QStringLiteral("color: %1; font-size: 8.5px; font-weight: bold;").arg(valColor));
        vbox->addWidget(l);
        vbox->addWidget(v);
        return vbox;
    };

    ml->addLayout(makeCell("Khí Áp An Toàn:", "950 — 1050 hPa", "#38bdf8"), 0, 0);
    ml->addLayout(makeCell("Ngưỡng Báo Cháy:", "> 45.0 °C (Còi Báo)", "#fb923c"), 0, 1);
    ml->addLayout(makeCell("Cảm Biến PIR:", "Còi Buzzer & Đèn LED", "#34d399"), 1, 0);
    ml->addLayout(makeCell("Kênh MQTT:", "/esp32s3/telemetry (1s)", "#a78bfa"), 1, 1);

    actLay->addWidget(matrixFrame);

    // Row 5: Technical Details Strip
    auto *infoFrame = new QFrame;
    infoFrame->setStyleSheet("QFrame { background: #050c1e; border: 1px solid #142548; border-radius: 6px; }");
    auto *ifl = new QHBoxLayout(infoFrame);
    ifl->setContentsMargins(8, 4, 8, 4);
    ifl->setSpacing(4);
    m_devActiveDetails = new QLabel("Cập nhật tức thời qua MQTT 1883 · Đồng bộ CSDL SQLite");
    m_devActiveDetails->setWordWrap(false);
    m_devActiveDetails->setStyleSheet("color: #38bdf8; font-size: 8px; font-weight: bold;");
    ifl->addWidget(m_devActiveDetails);
    ifl->addStretch();
    auto *syncTag = new QLabel("● SYNCED");
    syncTag->setWordWrap(false);
    syncTag->setStyleSheet("color: #10b981; font-size: 7.5px; font-weight: 900;");
    ifl->addWidget(syncTag);
    actLay->addWidget(infoFrame);

    // Row 6: Bottom Action Row
    auto *actRow = new QHBoxLayout;
    actRow->setSpacing(6);

    m_devConfigBtn = button("⚙ CÀI ĐẶT NGƯỠNG BÁO ĐỘNG", "cyanBtn");
    m_devConfigBtn->setFixedHeight(32);
    m_devConfigBtn->setCursor(Qt::PointingHandCursor);
    m_devConfigBtn->setStyleSheet("QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #2563eb); color: #03081a; font-weight: 900; font-size: 10px; border: none; border-radius: 6px; } QPushButton:hover { background: #38bdf8; }");

    m_devReleaseBtn = button("✕ GỠ BỎ", "danger");
    m_devReleaseBtn->setFixedHeight(32);
    m_devReleaseBtn->setCursor(Qt::PointingHandCursor);
    m_devReleaseBtn->setStyleSheet("QPushButton { background: rgba(225, 29, 72, 0.15); border: 1.5px solid #e11d48; color: #fb7185; font-weight: 900; font-size: 10px; border-radius: 6px; padding: 2px 12px; } QPushButton:hover { background: #e11d48; color: #ffffff; }");

    actRow->addWidget(m_devConfigBtn, 1);
    actRow->addWidget(m_devReleaseBtn);
    actLay->addLayout(actRow);

    m_devLeftStack->addWidget(m_devLeftActiveView);

    // View 1: Empty Station View
    m_devLeftEmptyView = new QWidget;
    auto *empLay = new QVBoxLayout(m_devLeftEmptyView);
    empLay->setContentsMargins(16, 24, 16, 16);
    empLay->setSpacing(10);
    empLay->setAlignment(Qt::AlignCenter);

    auto *eTitle = label("CHƯA CÓ TRẠM ĐO NÀO ĐƯỢC LIÊN KẾT", "loginHeroTitle");
    eTitle->setAlignment(Qt::AlignCenter);
    eTitle->setStyleSheet("color: #00f2fe; font-size: 14px; font-weight: 900;");

    auto *eSub = label("Hệ thống chưa kết nối trạm cảm biến nào. Hãy chọn trạm phát hiện bên phải hoặc bấm nút bên dưới để ghép nối.", "loginHeroSub");
    eSub->setAlignment(Qt::AlignCenter);
    eSub->setStyleSheet("color: #94a3b8; font-size: 10px;");

    auto *eBtn = button("+ Ghép Nối Trạm Mới Ngay >>", "primaryBtn");
    eBtn->setFixedSize(240, 36);
    connect(eBtn, &QPushButton::clicked, this, &MainWindow::openAddDeviceDialog);

    empLay->addWidget(eTitle);
    empLay->addWidget(eSub);
    empLay->addWidget(eBtn, 0, Qt::AlignCenter);
    m_devLeftStack->addWidget(m_devLeftEmptyView);

    lpLay->addWidget(m_devLeftStack, 1);
    body->addWidget(leftPanel, 4);

    // =========================================================================
    // RIGHT COCKPIT: PHÁT HIỆN & GHÉP NỐI TRẠM MỚI (Flex 3)
    // =========================================================================
    auto *rightPanel = new QFrame;
    rightPanel->setObjectName("deviceCockpitRight");
    rightPanel->setStyleSheet(
        "QFrame#deviceCockpitRight { background: #070e24; border: 1.5px solid #1a3466; border-radius: 10px; }"
    );
    auto *rpLay = new QVBoxLayout(rightPanel);
    rpLay->setContentsMargins(10, 8, 10, 8);
    rpLay->setSpacing(6);

    auto *rpHead = new QHBoxLayout;
    auto *rpTitle = label("📡 RADAR PHÁT HIỆN TRẠM");
    rpTitle->setWordWrap(false);
    rpTitle->setStyleSheet("color: #00f2fe; font-size: 10px; font-weight: 900;");
    auto *rpPort = label("● BROKER: 1883", "onlineBadge");
    rpPort->setStyleSheet("background: #0284c7; color: #ffffff; border-radius: 4px; padding: 2px 6px; font-size: 8px; font-weight: bold;");
    rpHead->addWidget(rpTitle);
    rpHead->addStretch();
    rpHead->addWidget(rpPort);
    rpLay->addLayout(rpHead);

    m_availStack = new QStackedWidget(rightPanel);

    // Page 0: Có trạm mới đang chờ ghép nối
    auto *availCard = new QFrame;
    availCard->setStyleSheet("QFrame { background: #0a183d; border: 1.5px solid #00f2fe; border-radius: 8px; }");
    auto *acl = new QVBoxLayout(availCard);
    acl->setContentsMargins(8, 6, 8, 6);
    acl->setSpacing(4);

    auto *atRow = new QHBoxLayout;
    atRow->addWidget(label("[TRẠM MỚI PHÁT HIỆN]", "discoveredBadge"));
    atRow->addStretch();
    auto *stReady = label("● SẴN SÀNG GHÉP", "onlineBadge");
    atRow->addWidget(stReady);
    acl->addLayout(atRow);

    m_availDevId = label("MÃ THIẾT BỊ: --", "deviceName");
    m_availDevId->setStyleSheet("color: #ffffff; font-size: 12px; font-weight: 900;");
    acl->addWidget(m_availDevId);

    m_availDevDesc = label("Cảm biến BMP180 + HC-SR501 PIR · ESP32-S3", "metricValSub");
    m_availDevDesc->setStyleSheet("color: #94a3b8; font-size: 8.5px; font-weight: 600;");
    acl->addWidget(m_availDevDesc);

    // Hardware Details Box inside Discovered Card
    auto *dSpecBox = new QFrame;
    dSpecBox->setStyleSheet("QFrame { background: #060e22; border: 1px solid #162c58; border-radius: 6px; }");
    auto *dsl = new QVBoxLayout(dSpecBox);
    dsl->setContentsMargins(6, 4, 6, 4);
    dsl->setSpacing(2);

    auto makeSpecRow = [](const QString &k, const QString &v, const QString &vc = "#38bdf8") {
        auto *r = new QHBoxLayout;
        auto *kl = new QLabel(k);
        kl->setWordWrap(false);
        kl->setStyleSheet("color: #64748b; font-size: 7.5px; font-weight: bold;");
        auto *vl = new QLabel(v);
        vl->setWordWrap(false);
        vl->setStyleSheet(QStringLiteral("color: %1; font-size: 8px; font-weight: bold;").arg(vc));
        r->addWidget(kl);
        r->addStretch();
        r->addWidget(vl);
        return r;
    };

    dsl->addLayout(makeSpecRow("Board:", "ESP32-S3 Xtensa Dual-Core"));
    dsl->addLayout(makeSpecRow("Cảm biến:", "BMP180 + PIR Hồng Ngoại"));
    dsl->addLayout(makeSpecRow("Sóng Wi-Fi:", "RSSI -52 dBm (Rất mạnh)", "#10b981"));
    dsl->addLayout(makeSpecRow("Giao thức:", "MQTT Broadcast (1883)"));
    acl->addWidget(dSpecBox);

    // Signal Strength Bar Graphic
    auto *sigBox = new QFrame;
    sigBox->setStyleSheet("QFrame { background: #081636; border-radius: 4px; padding: 2px; }");
    auto *sbl = new QHBoxLayout(sigBox);
    sbl->setContentsMargins(6, 3, 6, 3);
    auto *sigLbl = new QLabel("📶 TÍN HIỆU SÓNG:");
    sigLbl->setStyleSheet("color: #00f2fe; font-size: 8px; font-weight: 900;");
    sbl->addWidget(sigLbl);
    sbl->addStretch();
    auto *bars = new QLabel("■■■■ CỰC MẠNH (-52dBm)");
    bars->setStyleSheet("color: #10b981; font-size: 8px; font-weight: 900;");
    sbl->addWidget(bars);
    acl->addWidget(sigBox);

    acl->addStretch();

    m_availClaimBtn = button("+ GHÉP NỐI TRẠM NÀY NGAY >>", "addDeviceBtn");
    m_availClaimBtn->setFixedHeight(34);
    m_availClaimBtn->setCursor(Qt::PointingHandCursor);
    m_availClaimBtn->setStyleSheet("QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff6b00, stop:1 #ff3800); color: #ffffff; font-weight: 900; font-size: 10.5px; border: none; border-radius: 6px; } QPushButton:hover { background: #ff7711; }");
    acl->addWidget(m_availClaimBtn);

    m_availStack->addWidget(availCard);

    // Page 1: Không có trạm mới (Radar scanning radar beacon)
    auto *scanCard = new QFrame;
    scanCard->setStyleSheet("QFrame { background: #060e22; border: 1px dashed #1e3a6c; border-radius: 8px; }");
    auto *scl = new QVBoxLayout(scanCard);
    scl->setContentsMargins(10, 10, 10, 10);
    scl->setSpacing(6);
    scl->setAlignment(Qt::AlignCenter);

    auto *scanPulseLbl = label("[⟳ SÓNG RADAR ĐANG QUÉT MẠNG]");
    scanPulseLbl->setAlignment(Qt::AlignCenter);
    scanPulseLbl->setStyleSheet("color: #00f2fe; font-size: 10px; font-weight: 900;");

    auto *scanDescLbl = label("Đang tự động lắng nghe gói tin phát hiện trạm (MQTT Broadcast) từ các board ESP32-S3 trong mạng Wi-Fi...");
    scanDescLbl->setAlignment(Qt::AlignCenter);
    scanDescLbl->setStyleSheet("color: #64748b; font-size: 8px; font-weight: 600;");

    auto *scanTipBox = new QFrame;
    scanTipBox->setStyleSheet("background: #081533; border: 1px solid #162c5a; border-radius: 6px; padding: 4px;");
    auto *stl = new QVBoxLayout(scanTipBox);
    auto *tipText = new QLabel("💡 Mẹo: Bật nguồn board ESP32-S3 và kết nối cùng Wi-Fi để trạm tự động xuất hiện trên Radar.");
    tipText->setWordWrap(true);
    tipText->setStyleSheet("color: #94a3b8; font-size: 7.5px;");
    stl->addWidget(tipText);

    auto *scanManualBtn = button("+ Ghép Nối Bằng Mã Thủ Công", "ghost");
    scanManualBtn->setFixedHeight(30);
    scanManualBtn->setCursor(Qt::PointingHandCursor);
    scanManualBtn->setStyleSheet("QPushButton { background: #13244e; border: 1px solid #284488; color: #38bdf8; font-weight: bold; font-size: 9.5px; border-radius: 6px; padding: 2px 10px; } QPushButton:hover { background: #1c3674; color: #ffffff; }");
    connect(scanManualBtn, &QPushButton::clicked, this, &MainWindow::openAddDeviceDialog);

    scl->addWidget(scanPulseLbl);
    scl->addWidget(scanDescLbl);
    scl->addWidget(scanTipBox);
    scl->addWidget(scanManualBtn);
    m_availStack->addWidget(scanCard);

    rpLay->addWidget(m_availStack, 1);
    body->addWidget(rightPanel, 3);

    root->addLayout(body, 1);
    m_pages->addWidget(page);

    connect(m_devSelectCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        renderDevices();
    });
}

static QFrame *createTelemetryKpiCard(const QString &title, QLabel *&valLbl, const QString &subText, const QString &accentColor, const QString &initialVal = "--")
{
    auto *card = new QFrame;
    card->setFixedHeight(48);
    card->setStyleSheet(QStringLiteral(
        "QFrame { background: #0b1528; border: 1px solid %1; border-radius: 6px; padding: 1px 4px; }"
    ).arg(accentColor));

    auto *lay = new QVBoxLayout(card);
    lay->setContentsMargins(6, 2, 6, 2);
    lay->setSpacing(0);

    auto *tLbl = new QLabel(title);
    tLbl->setWordWrap(false);
    tLbl->setStyleSheet(QStringLiteral("color: #93c5fd; font-size: 8px; font-weight: bold; text-transform: uppercase;"));
    lay->addWidget(tLbl);

    valLbl = new QLabel(initialVal);
    valLbl->setWordWrap(false);
    valLbl->setStyleSheet(QStringLiteral("color: %1; font-size: 12px; font-weight: 900;").arg(accentColor));
    lay->addWidget(valLbl);

    auto *sLbl = new QLabel(subText);
    sLbl->setWordWrap(false);
    sLbl->setStyleSheet(QStringLiteral("color: #64748b; font-size: 7.5px; font-weight: 600;"));
    lay->addWidget(sLbl);

    return card;
}

void MainWindow::buildHistory()
{
    auto *page = new QWidget;
    page->setStyleSheet("background-color: #060b17;");
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(2, 2, 2, 2);
    root->setSpacing(4);

    // Row 1: Filters & Date Navigator (Height: 26px, Compact for 7-inch display)
    auto *row1 = new QHBoxLayout;
    row1->setSpacing(4);
    auto *titleLbl = label("LỊCH SỬ ĐO", "pageTitle");
    titleLbl->setStyleSheet("color: #00f2fe; font-size: 11px; font-weight: 900;");
    row1->addWidget(titleLbl);

    m_historyDevice = new QComboBox;
    m_historyDevice->setFixedWidth(135);
    m_historyDevice->setFixedHeight(26);

    m_historyPeriod = new QComboBox;
    m_historyPeriod->addItem("Theo Ngày", "day");
    m_historyPeriod->addItem("Theo Tháng", "month");
    m_historyPeriod->addItem("Theo Năm", "year");
    m_historyPeriod->setFixedWidth(102);
    m_historyPeriod->setFixedHeight(26);

    m_histDatePrev = new QPushButton("<");
    m_histDatePrev->setFixedSize(22, 26);
    m_histDatePrev->setCursor(Qt::PointingHandCursor);
    m_histDatePrev->setToolTip("Lùi thời gian");
    m_histDatePrev->setStyleSheet("QPushButton { background: #0e1b38; border: 1px solid #1e3a6c; border-radius: 4px; color: #00f2fe; font-weight: 900; } QPushButton:hover { background: #162a56; }");

    m_histDateDisplay = new QLabel;
    m_histDateDisplay->setAlignment(Qt::AlignCenter);
    m_histDateDisplay->setFixedSize(82, 26);
    m_histDateDisplay->setStyleSheet("QLabel { background: #071022; border: 1px solid #1e3a6c; border-radius: 4px; color: #00f2fe; font-size: 10px; font-weight: bold; padding: 1px 4px; }");
    updateHistoryDateDisplay();

    m_histDateNext = new QPushButton(">");
    m_histDateNext->setFixedSize(22, 26);
    m_histDateNext->setCursor(Qt::PointingHandCursor);
    m_histDateNext->setToolTip("Tiến thời gian");
    m_histDateNext->setStyleSheet("QPushButton { background: #0e1b38; border: 1px solid #1e3a6c; border-radius: 4px; color: #00f2fe; font-weight: 900; } QPushButton:hover { background: #162a56; }");

    m_histDateToday = new QPushButton("Hiện Tại");
    m_histDateToday->setFixedHeight(26);
    m_histDateToday->setCursor(Qt::PointingHandCursor);
    m_histDateToday->setToolTip("Về ngày hôm nay");
    m_histDateToday->setStyleSheet("QPushButton { background: #0e1b38; border: 1px solid #1e3a6c; border-radius: 4px; color: #93c5fd; font-size: 10px; font-weight: bold; padding: 1px 6px; } QPushButton:hover { border-color: #00f2fe; color: #ffffff; }");

    auto *search = button("Làm Mới");
    search->setStyleSheet("QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #3b82f6); color: #03081a; font-weight: 900; font-size: 10px; border: none; border-radius: 4px; padding: 2px 8px; } QPushButton:hover { background: #38bdf8; }");
    search->setFixedHeight(26);

    row1->addWidget(m_historyDevice);
    row1->addWidget(m_historyPeriod);
    row1->addWidget(m_histDatePrev);
    row1->addWidget(m_histDateDisplay);
    row1->addWidget(m_histDateNext);
    row1->addWidget(m_histDateToday);
    row1->addWidget(search);
    row1->addStretch();
    root->addLayout(row1);

    // Row 2: 5 Mode Toggle Buttons (Evenly stretched across 7-inch width)
    auto *row2 = new QHBoxLayout;
    row2->setSpacing(4);
    m_histBtnPressure = new QPushButton("1. Khí Áp (hPa)");
    m_histBtnMotion   = new QPushButton("2. Chuyển Động (IR)");
    m_histBtnTemp     = new QPushButton("3. Nhiệt Độ (°C)");
    m_histBtnMulti    = new QPushButton("4. Toàn Cảnh 3 Kênh");
    m_histBtnTable    = new QPushButton("5. Sổ Đo Dữ Liệu");

    QList<QPushButton*> tabBtns = { m_histBtnPressure, m_histBtnMotion, m_histBtnTemp, m_histBtnMulti, m_histBtnTable };
    for (auto *b : tabBtns) {
        b->setFixedHeight(26);
        b->setCursor(Qt::PointingHandCursor);
        row2->addWidget(b, 1);
    }
    root->addLayout(row2);

    // Main Stack: 5 Dedicated Sub-pages
    m_historyStack = new QStackedWidget;
    m_historyStack->setStyleSheet("background-color: #060b17;");

    auto applyBtnStyles = [tabBtns](int activeIdx) {
        for (int i = 0; i < tabBtns.size(); ++i) {
            bool active = (i == activeIdx);
            tabBtns[i]->setStyleSheet(active
                ? QStringLiteral("QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1a3a78, stop:1 #0f2248); border: 1.5px solid #00f2fe; border-radius: 5px; color: #00f2fe; font-size: 10px; font-weight: 900; padding: 2px 8px; }")
                : QStringLiteral("QPushButton { background: #0c1833; border: 1px solid #1a2f58; border-radius: 5px; color: #94a3b8; font-size: 10px; font-weight: bold; padding: 2px 8px; } QPushButton:hover { border-color: #38bdf8; color: #ffffff; }")
            );
        }
    };

    auto switchSubTab = [this, applyBtnStyles](int idx) {
        applyBtnStyles(idx);
        m_historyStack->setCurrentIndex(idx);
    };

    connect(m_histBtnPressure, &QPushButton::clicked, this, [=]{ switchSubTab(0); });
    connect(m_histBtnMotion, &QPushButton::clicked, this, [=]{ switchSubTab(1); });
    connect(m_histBtnTemp, &QPushButton::clicked, this, [=]{ switchSubTab(2); });
    connect(m_histBtnMulti, &QPushButton::clicked, this, [=]{ switchSubTab(3); });
    connect(m_histBtnTable, &QPushButton::clicked, this, [=]{ switchSubTab(4); });

    // PAGE 0: ÁP SUẤT KHÍ QUYỂN (BAROGRAM)
    auto *pagePressure = new QWidget;
    auto *layP = new QVBoxLayout(pagePressure);
    layP->setContentsMargins(2, 2, 2, 2);
    layP->setSpacing(4);

    auto *kpiRowP = new QHBoxLayout;
    kpiRowP->setSpacing(4);
    kpiRowP->addWidget(createTelemetryKpiCard("Áp Suất Tức Thời", m_histPressCur, "Khí áp kế BMP180", "#00f2fe", "-- hPa"), 1);
    kpiRowP->addWidget(createTelemetryKpiCard("Xu Hướng 3H (ΔP)", m_histPressDelta, "Biến thiên khí áp", "#38bdf8", "-- hPa"), 1);
    kpiRowP->addWidget(createTelemetryKpiCard("Dự Báo Khí Tượng", m_histPressEval, "Trạng thái khí quyển", "#10b981", "Chuẩn 1.0 ATM"), 1);
    kpiRowP->addWidget(createTelemetryKpiCard("Độ Cao Khí Áp", m_histPressAlt, "Ước tính theo ASL", "#a78bfa", "-- m"), 1);
    layP->addLayout(kpiRowP);

    auto *chartFrameP = new QFrame;
    chartFrameP->setStyleSheet("QFrame { background: #070e20; border: 1px solid #1a2f58; border-radius: 8px; }");
    auto *cLayP = new QVBoxLayout(chartFrameP);
    cLayP->setContentsMargins(8, 4, 8, 4);
    cLayP->setSpacing(2);

    auto *headP = new QHBoxLayout;
    auto *tP = new QLabel("ĐƯỜNG CONG ÁP SUẤT KHÍ QUYỂN LIÊN TỤC (BAROGRAM) - ĐƠN VỊ: hPa");
    tP->setStyleSheet("color: #00f2fe; font-size: 11px; font-weight: 900;");
    headP->addWidget(tP);
    headP->addStretch();
    auto *zoomP = new QPushButton("Phóng To >>");
    zoomP->setStyleSheet("QPushButton { background: #13224a; border: 1px solid #283e78; border-radius: 5px; color: #00f2fe; font-size: 10px; font-weight: bold; padding: 2px 8px; } QPushButton:hover { background: #00f2fe; color: #030818; }");
    connect(zoomP, &QPushButton::clicked, this, [this]{ showChartZoomDialog("pressure_hpa"); });
    headP->addWidget(zoomP);
    cLayP->addLayout(headP);

    m_histPressView = new QChartView;
    m_histPressView->setStyleSheet("background: transparent; border: none;");
    m_histPressView->setRenderHint(QPainter::Antialiasing);
    cLayP->addWidget(m_histPressView, 1);

    m_histPressRange = new QLabel("Đang tải dữ liệu khí áp trạm...");
    m_histPressRange->setStyleSheet("color: #93c5fd; font-size: 9px; font-weight: bold; background: #091326; border-radius: 4px; padding: 2px 8px;");
    cLayP->addWidget(m_histPressRange);
    layP->addWidget(chartFrameP, 1);
    m_historyStack->addWidget(pagePressure);

    // PAGE 1: XUNG CHUYỂN ĐỘNG THỜI GIAN THỰC (PIR)
    auto *pageMotion = new QWidget;
    auto *layM = new QVBoxLayout(pageMotion);
    layM->setContentsMargins(2, 2, 2, 2);
    layM->setSpacing(4);

    auto *kpiRowM = new QHBoxLayout;
    kpiRowM->setSpacing(4);
    kpiRowM->addWidget(createTelemetryKpiCard("Trạng Thái Hiện Tại", m_histMotionCur, "Hồng ngoại PIR HC-SR501", "#10b981", "AN TOÀN [OK]"), 1);
    kpiRowM->addWidget(createTelemetryKpiCard("Tổng Lần Phát Hiện", m_histMotionCount, "Số sự kiện kích hoạt", "#ff1744", "0 LẦN"), 1);
    kpiRowM->addWidget(createTelemetryKpiCard("Lần Kích Hoạt Cuối", m_histMotionLast, "Mốc thời gian gần nhất", "#f59e0b", "--:--:--"), 1);
    kpiRowM->addWidget(createTelemetryKpiCard("Mật Độ Chuyển Động", m_histMotionDuty, "Tỷ lệ thời gian có người", "#38bdf8", "0.0%"), 1);
    layM->addLayout(kpiRowM);

    auto *chartFrameM = new QFrame;
    chartFrameM->setStyleSheet("QFrame { background: #070e20; border: 1px solid #1a2f58; border-radius: 8px; }");
    auto *cLayM = new QVBoxLayout(chartFrameM);
    cLayM->setContentsMargins(8, 4, 8, 4);
    cLayM->setSpacing(2);

    auto *headM = new QHBoxLayout;
    auto *tM = new QLabel("NHẬT KÝ XUNG KÍCH HOẠT CHUYỂN ĐỘNG (PIR LOGIC TIMELINE)");
    tM->setStyleSheet("color: #ff1744; font-size: 11px; font-weight: 900;");
    headM->addWidget(tM);
    headM->addStretch();
    auto *zoomM = new QPushButton("Phóng To >>");
    zoomM->setStyleSheet("QPushButton { background: #13224a; border: 1px solid #283e78; border-radius: 5px; color: #00f2fe; font-size: 10px; font-weight: bold; padding: 2px 8px; } QPushButton:hover { background: #00f2fe; color: #030818; }");
    connect(zoomM, &QPushButton::clicked, this, [this]{ showChartZoomDialog("ir_detected"); });
    headM->addWidget(zoomM);
    cLayM->addLayout(headM);

    m_histMotionView = new QChartView;
    m_histMotionView->setStyleSheet("background: transparent; border: none;");
    m_histMotionView->setRenderHint(QPainter::Antialiasing);
    cLayM->addWidget(m_histMotionView, 1);

    m_histMotionSummary = new QLabel("Đang tải dữ liệu chuyển động hồng ngoại...");
    m_histMotionSummary->setStyleSheet("color: #93c5fd; font-size: 9px; font-weight: bold; background: #091326; border-radius: 4px; padding: 2px 8px;");
    cLayM->addWidget(m_histMotionSummary);
    layM->addWidget(chartFrameM, 1);
    m_historyStack->addWidget(pageMotion);

    // PAGE 2: BIẾN THIÊN NHIỆT ĐỘ (THERMOGRAM)
    auto *pageTemp = new QWidget;
    auto *layT = new QVBoxLayout(pageTemp);
    layT->setContentsMargins(2, 2, 2, 2);
    layT->setSpacing(4);

    auto *kpiRowT = new QHBoxLayout;
    kpiRowT->setSpacing(4);
    kpiRowT->addWidget(createTelemetryKpiCard("Nhiệt Độ Môi Trường", m_histTempCur, "Đo từ BMP180", "#ff9100", "--.- °C"), 1);
    kpiRowT->addWidget(createTelemetryKpiCard("Nhiệt Độ Thấp Nhất", m_histTempMin, "Min trong chu kỳ", "#38bdf8", "--.- °C"), 1);
    kpiRowT->addWidget(createTelemetryKpiCard("Nhiệt Độ Cao Nhất", m_histTempMax, "Max trong chu kỳ", "#f43f5e", "--.- °C"), 1);
    kpiRowT->addWidget(createTelemetryKpiCard("Trung Bình & Tiện Nghi", m_histTempAvg, "Đánh giá tiện nghi", "#10b981", "--.- °C"), 1);
    layT->addLayout(kpiRowT);

    auto *chartFrameT = new QFrame;
    chartFrameT->setStyleSheet("QFrame { background: #070e20; border: 1px solid #1a2f58; border-radius: 8px; }");
    auto *cLayT = new QVBoxLayout(chartFrameT);
    cLayT->setContentsMargins(8, 4, 8, 4);
    cLayT->setSpacing(2);

    auto *headT = new QHBoxLayout;
    auto *tT = new QLabel("BIẾN THIÊN NHIỆT ĐỘ MÔI TRƯỜNG (THERMOGRAM) - ĐƠN VỊ: °C");
    tT->setStyleSheet("color: #ff9100; font-size: 11px; font-weight: 900;");
    headT->addWidget(tT);
    headT->addStretch();
    auto *zoomT = new QPushButton("Phóng To >>");
    zoomT->setStyleSheet("QPushButton { background: #13224a; border: 1px solid #283e78; border-radius: 5px; color: #00f2fe; font-size: 10px; font-weight: bold; padding: 2px 8px; } QPushButton:hover { background: #00f2fe; color: #030818; }");
    connect(zoomT, &QPushButton::clicked, this, [this]{ showChartZoomDialog("temperature_c"); });
    headT->addWidget(zoomT);
    cLayT->addLayout(headT);

    m_histTempView = new QChartView;
    m_histTempView->setStyleSheet("background: transparent; border: none;");
    m_histTempView->setRenderHint(QPainter::Antialiasing);
    cLayT->addWidget(m_histTempView, 1);

    m_histTempSummary = new QLabel("Đang tải dữ liệu nhiệt độ môi trường...");
    m_histTempSummary->setStyleSheet("color: #93c5fd; font-size: 9px; font-weight: bold; background: #091326; border-radius: 4px; padding: 2px 8px;");
    cLayT->addWidget(m_histTempSummary);
    layT->addWidget(chartFrameT, 1);
    m_historyStack->addWidget(pageTemp);

    // PAGE 3: TOÀN CẢNH 3 KÊNH ĐỒNG BỘ
    auto *pageMulti = new QWidget;
    auto *layMulti = new QVBoxLayout(pageMulti);
    layMulti->setContentsMargins(2, 2, 2, 2);
    layMulti->setSpacing(4);

    auto makeStreamStrip = [](const QString &title, const QString &color, QChartView *&view, auto zoomCb) {
        auto *strip = new QFrame;
        strip->setStyleSheet("QFrame { background: #070e20; border: 1px solid #1a2f58; border-radius: 6px; }");
        auto *sLay = new QVBoxLayout(strip);
        sLay->setContentsMargins(6, 2, 6, 2);
        sLay->setSpacing(1);

        auto *sHead = new QHBoxLayout;
        auto *sTitle = new QLabel(title);
        sTitle->setStyleSheet(QStringLiteral("color: %1; font-size: 10px; font-weight: 900;").arg(color));
        sHead->addWidget(sTitle);
        sHead->addStretch();
        auto *zBtn = new QPushButton("Chi tiết >>");
        zBtn->setStyleSheet("QPushButton { background: #13224a; border: 1px solid #283e78; border-radius: 4px; color: #00f2fe; font-size: 9px; font-weight: bold; padding: 1px 6px; }");
        QObject::connect(zBtn, &QPushButton::clicked, zoomCb);
        sHead->addWidget(zBtn);
        sLay->addLayout(sHead);

        view = new QChartView;
        view->setStyleSheet("background: transparent; border: none;");
        view->setRenderHint(QPainter::Antialiasing);
        sLay->addWidget(view, 1);
        return strip;
    };

    layMulti->addWidget(makeStreamStrip("1. ÁP SUẤT KHÍ QUYỂN (hPa)", "#00f2fe", m_histMultiPressView, [this]{ showChartZoomDialog("pressure_hpa"); }), 1);
    layMulti->addWidget(makeStreamStrip("2. XUNG CHUYỂN ĐỘNG THEO THỜI GIAN THỰC (PIR 0/1)", "#ff1744", m_histMultiMotionView, [this]{ showChartZoomDialog("ir_detected"); }), 1);
    layMulti->addWidget(makeStreamStrip("3. NHIỆT ĐỘ KHÍ QUYỂN (°C)", "#ff9100", m_histMultiTempView, [this]{ showChartZoomDialog("temperature_c"); }), 1);
    m_historyStack->addWidget(pageMulti);

    // PAGE 4: SỔ ĐO DỮ LIỆU SỐ
    m_historyTable = new QTableWidget;
    m_historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_historyTable->verticalHeader()->hide();
    m_historyStack->addWidget(m_historyTable);

    root->addWidget(m_historyStack, 1);

    // Initial state: Tab 0 (Pressure)
    switchSubTab(0);

    connect(search, &QPushButton::clicked, this, &MainWindow::refreshHistory);
    connect(m_historyDevice, &QComboBox::currentIndexChanged, this, [this](int) { refreshHistory(); });

    connect(m_historyPeriod, &QComboBox::currentIndexChanged, this, [this](int) {
        updateHistoryDateDisplay();
        refreshHistory();
    });

    connect(m_histDatePrev, &QPushButton::clicked, this, [this] {
        const QString period = m_historyPeriod->currentData().toString();
        if (period == "month") m_historyDate = m_historyDate.addMonths(-1);
        else if (period == "year") m_historyDate = m_historyDate.addYears(-1);
        else m_historyDate = m_historyDate.addDays(-1);
        updateHistoryDateDisplay();
        refreshHistory();
    });

    connect(m_histDateNext, &QPushButton::clicked, this, [this] {
        const QString period = m_historyPeriod->currentData().toString();
        if (period == "month") m_historyDate = m_historyDate.addMonths(1);
        else if (period == "year") m_historyDate = m_historyDate.addYears(1);
        else m_historyDate = m_historyDate.addDays(1);
        updateHistoryDateDisplay();
        refreshHistory();
    });

    connect(m_histDateToday, &QPushButton::clicked, this, [this] {
        m_historyDate = QDate::currentDate();
        updateHistoryDateDisplay();
        refreshHistory();
    });

    m_pages->addWidget(page);
}

void MainWindow::updateHistoryDateDisplay()
{
    if (!m_histDateDisplay || !m_historyPeriod) return;
    const QString period = m_historyPeriod->currentData().toString();
    const QDate today = QDate::currentDate();
    const bool isFuture = m_historyDate > today;

    QString text;
    if (period == "month") {
        text = m_historyDate.toString("MM/yyyy");
    } else if (period == "year") {
        text = m_historyDate.toString("yyyy");
    } else {
        text = m_historyDate.toString("dd/MM/yyyy");
    }

    m_histDateDisplay->setText(text);
    if (isFuture) {
        m_histDateDisplay->setStyleSheet("background: #240a15; color: #fb7185; border: 1px solid #f43f5e; border-radius: 4px; padding: 2px 4px; font-weight: 900; font-size: 10px;");
        m_histDateDisplay->setToolTip(tr("Mốc thời gian tương lai: Chưa có dữ liệu (0 bản ghi)"));
    } else {
        m_histDateDisplay->setStyleSheet("background: #0f1c3d; color: #00f2fe; border: 1px solid #1e3a6c; border-radius: 4px; padding: 2px 4px; font-weight: 900; font-size: 10px;");
        m_histDateDisplay->setToolTip(tr("Mốc thời gian tra cứu"));
    }
}

void MainWindow::buildUsers()
{
    auto *page = new QWidget;
    page->setStyleSheet("background-color: #060b17;");
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(8);

    // Top control bar
    auto *top = new QHBoxLayout;
    top->addWidget(label("QUẢN TRỊ HỆ THỐNG & NHẬT KÝ", "pageTitle"));
    top->addSpacing(15);

    // Subtab buttons
    m_tabUsersBtn = button("👥 Tài khoản", "hudNav");
    m_tabUsersBtn->setCheckable(true);
    m_tabUsersBtn->setChecked(true);
    m_tabUsersBtn->setFixedHeight(30);

    m_tabLoginBtn = button("🔑 Lịch sử đăng nhập", "hudNav");
    m_tabLoginBtn->setCheckable(true);
    m_tabLoginBtn->setFixedHeight(30);

    m_tabAuditBtn = button("⚡ Lịch sử điều khiển", "hudNav");
    m_tabAuditBtn->setCheckable(true);
    m_tabAuditBtn->setFixedHeight(30);

    top->addWidget(m_tabUsersBtn);
    top->addWidget(m_tabLoginBtn);
    top->addWidget(m_tabAuditBtn);
    top->addStretch();

    m_addUserBtn = button("+ Tạo tài khoản mới", "cyanBtn");
    m_addUserBtn->setFixedHeight(30);
    top->addWidget(m_addUserBtn);

    auto *refreshBtn = button("🔄 Làm mới", "hudNav");
    refreshBtn->setFixedHeight(30);
    top->addWidget(refreshBtn);

    root->addLayout(top);

    // Multi-page subtab stack
    m_usersStack = new QStackedWidget;

    // --- Subtab 0: Users Table ---
    auto *usersWidget = new QWidget;
    auto *usersLayout = new QVBoxLayout(usersWidget);
    usersLayout->setContentsMargins(0, 0, 0, 0);
    m_usersTable = new QTableWidget(0, 4);
    m_usersTable->setHorizontalHeaderLabels({"Tài khoản", "Quyền hạn", "Trạm phụ trách", "Trạng thái"});
    m_usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_usersTable->verticalHeader()->hide();
    usersLayout->addWidget(m_usersTable);
    m_usersStack->addWidget(usersWidget);

    // --- Subtab 1: Login History Table ---
    auto *loginWidget = new QWidget;
    auto *loginLayout = new QVBoxLayout(loginWidget);
    loginLayout->setContentsMargins(0, 0, 0, 0);
    loginLayout->setSpacing(6);
    m_loginSummaryLabel = label("Danh sách phiên đăng nhập của tất cả các tài khoản vào hệ thống", "metricValSub");
    loginLayout->addWidget(m_loginSummaryLabel);
    m_loginTable = new QTableWidget(0, 5);
    m_loginTable->setHorizontalHeaderLabels({"Thời gian", "Tài khoản", "Quyền", "Địa chỉ IP", "Trạng thái"});
    m_loginTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_loginTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_loginTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_loginTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_loginTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_loginTable->verticalHeader()->hide();
    loginLayout->addWidget(m_loginTable);
    m_usersStack->addWidget(loginWidget);

    // --- Subtab 2: Audit Logs Table ---
    auto *auditWidget = new QWidget;
    auto *auditLayout = new QVBoxLayout(auditWidget);
    auditLayout->setContentsMargins(0, 0, 0, 0);
    auditLayout->setSpacing(6);

    auto *auditBar = new QHBoxLayout;
    m_auditSummaryLabel = label("Nhật ký điều khiển thiết bị & thao tác cấu hình", "metricValSub");
    auditBar->addWidget(m_auditSummaryLabel);
    auditBar->addStretch();
    auditBar->addWidget(label("Tìm kiếm:", "metricValSub"));
    m_auditSearchEdit = new QLineEdit;
    m_auditSearchEdit->setPlaceholderText("Lọc theo tài khoản, hành động...");
    m_auditSearchEdit->setFixedWidth(220);
    m_auditSearchEdit->setStyleSheet("background-color: #0c1729; color: #e2e8f0; border: 1px solid #1e293b; border-radius: 4px; padding: 4px;");
    VirtualKeyboardDialog::attachToLineEdit(m_auditSearchEdit, "Tìm kiếm nhật ký");
    auditBar->addWidget(m_auditSearchEdit);
    auditLayout->addLayout(auditBar);

    m_auditTable = new QTableWidget(0, 5);
    m_auditTable->setHorizontalHeaderLabels({"Thời gian", "Tài khoản", "Hành động", "Thiết bị / Đối tượng", "Chi tiết thao tác"});
    m_auditTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_auditTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_auditTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_auditTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_auditTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_auditTable->verticalHeader()->hide();
    auditLayout->addWidget(m_auditTable);
    m_usersStack->addWidget(auditWidget);

    root->addWidget(m_usersStack, 1);

    // Wire subtab switching
    auto updateSubtabs = [this](int idx) {
        m_usersStack->setCurrentIndex(idx);
        m_tabUsersBtn->setChecked(idx == 0);
        m_tabLoginBtn->setChecked(idx == 1);
        m_tabAuditBtn->setChecked(idx == 2);
        if (m_addUserBtn) m_addUserBtn->setVisible(idx == 0 && m_role == "admin");
        if (idx == 0 && m_role == "admin") refreshUsers();
        else if (idx == 1 && m_role == "admin") refreshLoginHistory();
        else if (idx == 2) refreshAuditLogs();
    };

    connect(m_tabUsersBtn, &QPushButton::clicked, this, [updateSubtabs] { updateSubtabs(0); });
    connect(m_tabLoginBtn, &QPushButton::clicked, this, [updateSubtabs] { updateSubtabs(1); });
    connect(m_tabAuditBtn, &QPushButton::clicked, this, [updateSubtabs] { updateSubtabs(2); });

    connect(m_addUserBtn, &QPushButton::clicked, this, &MainWindow::createUserDialog);
    connect(refreshBtn, &QPushButton::clicked, this, [this] {
        if (m_usersStack->currentIndex() == 0 && m_role == "admin") refreshUsers();
        else if (m_usersStack->currentIndex() == 1 && m_role == "admin") refreshLoginHistory();
        else if (m_usersStack->currentIndex() == 2) refreshAuditLogs();
    });

    connect(m_usersTable, &QTableWidget::cellClicked, this, [this](int row, int) {
        if (row >= 0 && row < m_users.size()) editUserDialog(m_users.at(row).toObject());
    });

    connect(m_auditSearchEdit, &QLineEdit::textChanged, this, [this] {
        renderAuditLogs();
    });

    m_pages->addWidget(page);
}

QString MainWindow::deviceIcon(const QString &type)
{
    if (type == "weather_pressure") return "[RADAR]";
    if (type == "temperature_sound") return "[TEMP]";
    if (type == "uv_pressure") return "[UV]";
    if (type == "electric_power") return "[POWER]";
    if (type == "water_flow_pump") return "[FLOW]";
    return "[NODE]";
}

QString MainWindow::deviceTypeName(const QString &type)
{
    if (type == "weather_pressure") return "Trạm Khí Quyển & Chuyển Động";
    if (type == "temperature_sound") return "Trạm Nhiệt Độ & Âm Thanh";
    if (type == "uv_pressure") return "Trạm UV & Áp Suất";
    if (type == "electric_power") return "Trạm Đo Điện Năng";
    if (type == "water_flow_pump") return "Trạm Bơm & Lưu Lượng";
    return "Trạm Cảm Biến IoT";
}

QString MainWindow::metricText(const QJsonObject &d)
{
    const QString type = d.value("device_type").toString();
    const auto m = d.value("metrics").toObject();
    QStringList items;

    if (type == "weather_pressure") {
        if (m.value("pressure_hpa").isDouble())
            items << QStringLiteral("Áp suất: %1 hPa").arg(m.value("pressure_hpa").toDouble(), 0, 'f', 0);
        if (m.value("temperature_c").isDouble())
            items << QStringLiteral("Nhiệt độ: %1°C").arg(m.value("temperature_c").toDouble(), 0, 'f', 1);
        if (m.contains("ir_detected")) {
            const bool detected = m.value("ir_detected").toDouble() >= 0.5;
            items << (detected ? QStringLiteral("Chuyển động: [CÓ]") : QStringLiteral("Chuyển động: [Không]"));
        }
        if (m.value("lux").isDouble())
            items << QStringLiteral("Ánh sáng: %1 Lux").arg(m.value("lux").toDouble(), 0, 'f', 0);
    } else if (type == "temperature_sound") {
        if (m.value("temperature_c").isDouble())
            items << QStringLiteral("Nhiệt độ: %1°C").arg(m.value("temperature_c").toDouble(), 0, 'f', 1);
        if (m.value("sound_vpp").isDouble())
            items << QStringLiteral("Âm thanh: %1 V").arg(m.value("sound_vpp").toDouble(), 0, 'f', 2);
    } else if (type == "uv_pressure") {
        if (m.value("uv_index").isDouble())
            items << QStringLiteral("UV: %1").arg(m.value("uv_index").toDouble(), 0, 'f', 1);
        if (m.value("lux").isDouble())
            items << QStringLiteral("Ánh sáng: %1 Lux").arg(m.value("lux").toDouble(), 0, 'f', 0);
        if (m.value("pressure_hpa").isDouble())
            items << QStringLiteral("Áp suất: %1 hPa").arg(m.value("pressure_hpa").toDouble(), 0, 'f', 0);
    } else if (type == "electric_power") {
        if (m.value("voltage_v").isDouble())
            items << QStringLiteral("Điện áp: %1 V").arg(m.value("voltage_v").toDouble(), 0, 'f', 1);
        if (m.value("current_a").isDouble())
            items << QStringLiteral("Dòng điện: %1 A").arg(m.value("current_a").toDouble(), 0, 'f', 2);
    } else if (type == "water_flow_pump") {
        if (m.value("flow_l_min").isDouble())
            items << QStringLiteral("Lưu lượng: %1 L/m").arg(m.value("flow_l_min").toDouble(), 0, 'f', 1);
        if (m.value("total_liters").isDouble())
            items << QStringLiteral("Tổng: %1 L").arg(m.value("total_liters").toDouble(), 0, 'f', 1);
    }

    if (m.contains("ir_detected") && type != "weather_pressure") {
        const bool detected = m.value("ir_detected").toDouble() >= 0.5;
        items << (detected ? QStringLiteral("Chuyển động: [CÓ]") : QStringLiteral("Chuyển động: [Không]"));
    }

    if (items.isEmpty()) {
        for (auto it = m.begin(); it != m.end(); ++it) {
            if (it.value().isDouble() && it.key() != "ir_detected") {
                items << QStringLiteral("%1: %2").arg(it.key()).arg(it.value().toDouble(), 0, 'f', 1);
            }
        }
    }

    return items.isEmpty() ? QStringLiteral("--") : items.join("  |  ");
}

void MainWindow::refreshAll()
{
    refreshDevices();
    if (m_pages && m_pages->currentIndex() == 3) {
        if (m_role == "admin") {
            refreshUsers();
            refreshLoginHistory();
        }
        refreshAuditLogs();
    }
    if (m_pages && m_pages->currentIndex() == 2) refreshHistory();
}

void MainWindow::refreshDevices()
{
    get("/api/devices/me", [this](QJsonObject o) {
        m_devices = o.value("data").toArray();
        renderDevices();
        refreshAvailable();
    });
}

void MainWindow::refreshAvailable()
{
    get("/api/devices/available", [this](QJsonObject o) {
        m_available = o.value("data").toArray();
        renderAvailable();
    });
}

void MainWindow::refreshUsers()
{
    if (m_role != "admin") return;
    get("/api/admin/users", [this](QJsonObject o) {
        m_users = o.value("data").toArray();
        renderUsers();
    });
}

void MainWindow::refreshLoginHistory()
{
    if (m_role != "admin") return;
    get("/api/admin/login-history?limit=200", [this](QJsonObject o) {
        m_loginHistory = o.value("data").toArray();
        renderLoginHistory();
    });
}

void MainWindow::refreshAuditLogs()
{
    QString path = "/api/audit/logs?limit=200";
    get(path, [this](QJsonObject o) {
        m_auditLogs = o.value("data").toArray();
        renderAuditLogs();
    });
}

void MainWindow::renderDevices()
{
    m_kpiDevices->setText(QStringLiteral("%1 TRẠM").arg(m_devices.size()));
    int onlineCount = 0;

    for (const auto &v : m_devices) {
        auto d = v.toObject();
        if (isDeviceOnline(d)) onlineCount++;
    }

    m_kpiOnline->setText(onlineCount > 0 ? QStringLiteral("%1 ONLINE").arg(onlineCount) : QStringLiteral("OFFLINE"));
    m_kpiType->setText("THỜI GIAN THỰC");
    m_status->setText(QStringLiteral("● LIVE: %1").arg(QTime::currentTime().toString("HH:mm:ss")));

    // 1. Update Live Telemetry Hub on Dashboard
    if (m_devices.isEmpty()) {
        m_homeStack->setCurrentIndex(1); // Show Empty state
    } else {
        m_homeStack->setCurrentIndex(0); // Show Live Telemetry state
        const auto primaryDevice = m_devices.first().toObject();
        const auto metrics = primaryDevice.value("metrics").toObject();
        const QString devId = primaryDevice.value("device_id").toString();
        const QString devName = primaryDevice.value("name").toString(deviceTypeName(primaryDevice.value("device_type").toString()));
        const bool isOnline = isDeviceOnline(primaryDevice);

        // Update Station Labels
        m_liveStationName->setText(devName);
        m_liveStationId->setText(QStringLiteral("MÃ TRẠM: %1").arg(devId));
        m_liveStationStatus->setText(isOnline ? "ONLINE" : "OFFLINE");
        m_liveStationStatus->setObjectName(isOnline ? "onlineBadge" : "offlineBadge");
        m_liveStationStatus->style()->unpolish(m_liveStationStatus);
        m_liveStationStatus->style()->polish(m_liveStationStatus);

        double curTemp = 0.0;
        double curPressure = 0.0;

        // Update Temperature
        if (metrics.value("temperature_c").isDouble()) {
            curTemp = metrics.value("temperature_c").toDouble();
            m_liveTemp->setText(QStringLiteral("%1 °C").arg(curTemp, 0, 'f', 1));
        } else {
            m_liveTemp->setText(QStringLiteral("-- °C"));
        }

        // Update Pressure
        if (metrics.value("pressure_hpa").isDouble()) {
            curPressure = metrics.value("pressure_hpa").toDouble();
            m_livePressure->setText(QStringLiteral("%1 hPa").arg(curPressure, 0, 'f', 0));
        } else {
            m_livePressure->setText(QStringLiteral("-- hPa"));
        }

        // Update Visual Gauges
        if (m_baroDial && curPressure > 0.0) {
            m_baroDial->setPressure(curPressure);
        }
        if (m_thermalMeter && curTemp > 0.0) {
            m_thermalMeter->setTemperature(curTemp);
        }

        // Update Motion Sensor & Radar Animation
        const bool motionDetected = metrics.contains("ir_detected") && (metrics.value("ir_detected").toDouble() >= 0.5);
        if (m_radarWidget) {
            m_radarWidget->setMotionDetected(motionDetected);
        }

        if (motionDetected) {
            m_liveMotionCard->setObjectName("motionAlertCard");
            m_liveMotion->setText("[!] CẢNH BÁO: PHÁT HIỆN CÓ CHUYỂN ĐỘNG!");
            m_liveMotion->setObjectName("motionAlertText");
            m_liveMotionSub->setText("Cảm biến hồng ngoại IR phát hiện mục tiêu trong vùng quan trắc");
        } else {
            m_liveMotionCard->setObjectName("motionSafeCard");
            m_liveMotion->setText("[OK] VÙNG AN TOÀN · KHÔNG CÓ CHUYỂN ĐỘNG");
            m_liveMotion->setObjectName("motionSafeText");
            m_liveMotionSub->setText("Cảm biến hồng ngoại IR sẵn sàng nhận diện tức thời");
        }
        m_liveMotionCard->style()->unpolish(m_liveMotionCard);
        m_liveMotionCard->style()->polish(m_liveMotionCard);
        m_liveMotion->style()->unpolish(m_liveMotion);
        m_liveMotion->style()->polish(m_liveMotion);

        // Disconnect old button connections to prevent duplicates
        m_liveConfigBtn->disconnect();
        m_liveReleaseBtn->disconnect();
        connect(m_liveConfigBtn, &QPushButton::clicked, this, [this, devId] { openDeviceConfigDialog(devId); });
        connect(m_liveReleaseBtn, &QPushButton::clicked, this, [this, devId] { releaseDevice(devId); });
    }

    // 2. Render Device Manager Cockpit ("TRẠM ĐO" Page)
    if (m_devices.isEmpty()) {
        if (m_devLeftStack) m_devLeftStack->setCurrentIndex(1); // Show Empty state
    } else {
        if (m_devLeftStack) m_devLeftStack->setCurrentIndex(0); // Show Active cockpit state

        // Populate / update station selector combo
        if (m_devSelectCombo) {
            const QString currentDevId = m_devSelectCombo->currentData().toString();
            m_devSelectCombo->blockSignals(true);
            m_devSelectCombo->clear();
            for (const auto &v : m_devices) {
                const auto dev = v.toObject();
                const QString id = dev.value("device_id").toString();
                const QString name = dev.value("name").toString(id);
                m_devSelectCombo->addItem(name, id);
            }
            int idx = m_devSelectCombo->findData(currentDevId);
            if (idx >= 0) m_devSelectCombo->setCurrentIndex(idx);
            m_devSelectCombo->blockSignals(false);
        }

        // Target device currently selected in combo or first
        QJsonObject targetDevice = m_devices.first().toObject();
        if (m_devSelectCombo && m_devSelectCombo->currentIndex() >= 0) {
            const QString activeId = m_devSelectCombo->currentData().toString();
            for (const auto &v : m_devices) {
                if (v.toObject().value("device_id").toString() == activeId) {
                    targetDevice = v.toObject();
                    break;
                }
            }
        }

        const auto m = targetDevice.value("metrics").toObject();
        const QString activeId = targetDevice.value("device_id").toString();
        const bool activeOnline = isDeviceOnline(targetDevice);

        if (m_devActiveId) m_devActiveId->setText(QStringLiteral("MÃ: %1").arg(activeId));
        if (m_devActiveStatus) {
            m_devActiveStatus->setText(activeOnline ? "● ONLINE" : "● OFFLINE");
            m_devActiveStatus->setObjectName(activeOnline ? "onlineBadge" : "offlineBadge");
            m_devActiveStatus->style()->unpolish(m_devActiveStatus);
            m_devActiveStatus->style()->polish(m_devActiveStatus);
        }

        // Update 3 Cockpit Telemetry Pods
        if (m_devActivePressure) {
            if (m.value("pressure_hpa").isDouble() && m.value("pressure_hpa").toDouble() > 0) {
                m_devActivePressure->setText(QStringLiteral("%1 hPa").arg(m.value("pressure_hpa").toDouble(), 0, 'f', 1));
            } else {
                m_devActivePressure->setText(QStringLiteral("1013.2 hPa"));
            }
        }

        if (m_devActiveTemp) {
            if (m.value("temperature_c").isDouble() && m.value("temperature_c").toDouble() > 0) {
                m_devActiveTemp->setText(QStringLiteral("%1 °C").arg(m.value("temperature_c").toDouble(), 0, 'f', 1));
            } else {
                m_devActiveTemp->setText(QStringLiteral("29.5 °C"));
            }
        }

        if (m_devActiveMotion) {
            const bool mot = m.contains("ir_detected") && (m.value("ir_detected").toDouble() >= 0.5);
            m_devActiveMotion->setText(mot ? "PHÁT HIỆN!" : "AN TOÀN");
            m_devActiveMotion->setStyleSheet(mot ? "color: #ff1744; font-size: 16px; font-weight: 900;" : "color: #10b981; font-size: 16px; font-weight: 900;");
        }

        if (m_devActiveDetails) {
            m_devActiveDetails->setText(QStringLiteral("Loại trạm: %1 · Cổng MQTT: 1883 · Kết nối: Wi-Fi ổn định").arg(deviceTypeName(targetDevice.value("device_type").toString())));
        }

        if (m_devConfigBtn) {
            disconnect(m_devConfigBtn, nullptr, nullptr, nullptr);
            connect(m_devConfigBtn, &QPushButton::clicked, this, [this, activeId] { openDeviceConfigDialog(activeId); });
        }
        if (m_devReleaseBtn) {
            disconnect(m_devReleaseBtn, nullptr, nullptr, nullptr);
            connect(m_devReleaseBtn, &QPushButton::clicked, this, [this, activeId] { releaseDevice(activeId); });
        }
    }

    const QString selectedHistoryDevice = m_historyDevice->currentData().toString();
    m_historyDevice->blockSignals(true);
    m_historyDevice->clear();
    for (const auto &v : m_devices) {
        auto d = v.toObject();
        QString dName = d.value("name").toString().trimmed();
        QString dId = d.value("device_id").toString().trimmed();
        QString dLabel = dName.isEmpty() ? dId : (dName == dId ? dName : (dName + " (" + dId + ")"));
        m_historyDevice->addItem(dLabel, dId);
    }
    if (!selectedHistoryDevice.isEmpty()) {
        const int keepIndex = m_historyDevice->findData(selectedHistoryDevice);
        if (keepIndex >= 0) m_historyDevice->setCurrentIndex(keepIndex);
    }
    m_historyDevice->blockSignals(false);
}

void MainWindow::renderAvailable()
{
    if (!m_availStack) return;

    if (m_available.isEmpty()) {
        m_availStack->setCurrentIndex(1); // Show listening/scanning radar card
    } else {
        m_availStack->setCurrentIndex(0); // Show discovered station claim card
        const auto firstAvail = m_available.first().toObject();
        const QString devId = firstAvail.value("device_id").toString();
        const QString devType = firstAvail.value("device_type").toString("weather_pressure");

        if (m_availDevId) {
            m_availDevId->setText(QStringLiteral("MÃ THIẾT BỊ: %1").arg(devId));
        }
        if (m_availDevDesc) {
            m_availDevDesc->setText(QStringLiteral("Cảm biến: BMP180 + PIR Hồng Ngoại · %1").arg(deviceTypeName(devType)));
        }
        if (m_availClaimBtn) {
            disconnect(m_availClaimBtn, nullptr, nullptr, nullptr);
            connect(m_availClaimBtn, &QPushButton::clicked, this, [this, firstAvail] {
                openClaimDeviceDialog(firstAvail);
            });
        }
    }
}

void MainWindow::claimDevice(const QString &id, const QString &name)
{
    post("/api/devices/claim", {{"device_id", id}, {"name", name}}, [this](QJsonObject) {
        refreshDevices();
        QTimer::singleShot(350, this, &MainWindow::refreshDevices);
    });
}

void MainWindow::releaseDevice(const QString &id)
{
    post("/api/devices/release", {{"device_id", id}}, [this](QJsonObject) { refreshDevices(); });
}

void MainWindow::toggleRelay(const QString &id, bool state)
{
    post("/api/devices/relay", {{"device_id", id}, {"state", state}}, [this](QJsonObject) { refreshDevices(); });
}

void MainWindow::refreshHistory()
{
    const QString id = m_historyDevice->currentData().toString();
    if (id.isEmpty()) return;
    const QString period = m_historyPeriod ? m_historyPeriod->currentData().toString() : QStringLiteral("day");
    QUrlQuery q;
    q.addQueryItem("device_id", id);
    q.addQueryItem("period", period.isEmpty() ? QStringLiteral("day") : period);
    q.addQueryItem("date", m_historyDate.toString(Qt::ISODate));
    int limit = 160;
    if (period == "month") limit = 300;
    else if (period == "year") limit = 500;
    q.addQueryItem("limit", QString::number(limit));
    get("/api/devices/history?" + q.toString(QUrl::FullyEncoded), [this](QJsonObject o) { renderHistory(o); });
}

void MainWindow::renderHistory(const QJsonObject &h)
{
    m_lastHistory = h;
    const auto rows = h.value("data").toArray();
    const QString period = h.value("period").toString(m_historyPeriod ? m_historyPeriod->currentData().toString() : QStringLiteral("day"));

    // Sync current date if server reported auto-fallback prefix
    const QString retDate = h.value("selected_date").toString();
    if (!retDate.isEmpty()) {
        QDate parsed = QDate::fromString(retDate.left(10), Qt::ISODate);
        if (!parsed.isValid() && retDate.length() == 7) {
            parsed = QDate::fromString(retDate + "-01", Qt::ISODate);
        } else if (!parsed.isValid() && retDate.length() == 4) {
            parsed = QDate::fromString(retDate + "-01-01", Qt::ISODate);
        }
        if (parsed.isValid() && m_historyDate != parsed) {
            m_historyDate = parsed;
            updateHistoryDateDisplay();
        }
    }

    QString periodTitle;
    if (period == "year") periodTitle = QStringLiteral("Năm %1").arg(m_historyDate.toString("yyyy"));
    else if (period == "month") periodTitle = QStringLiteral("Tháng %1").arg(m_historyDate.toString("MM/yyyy"));
    else periodTitle = QStringLiteral("Ngày %1").arg(m_historyDate.toString("dd/MM/yyyy"));

    // 1. Update Table with custom high-contrast formatting for Atmospheric and Motion telemetry
    QStringList headers{
        QStringLiteral("Thời Gian Ghi Nhận"),
        QStringLiteral("Áp Suất (hPa)"),
        QStringLiteral("Nhiệt Độ (°C)"),
        QStringLiteral("Chuyển Động (IR)"),
        QStringLiteral("Đánh Giá Khí Tượng")
    };
    m_historyTable->setColumnCount(headers.size());
    m_historyTable->setHorizontalHeaderLabels(headers);

    if (rows.isEmpty()) {
        m_historyTable->setRowCount(0);
    } else {
        m_historyTable->setRowCount(rows.size());
        for (int r = 0; r < rows.size(); ++r) {
            auto e = rows[r].toObject();
            QDateTime dt = QDateTime::fromString(e.value("recorded_at").toString(), Qt::ISODateWithMs).toLocalTime();
            QString timeStr;
            if (period == "day") {
                timeStr = dt.isValid() ? dt.toString("HH:mm:ss (dd/MM)") : e.value("recorded_at").toString();
            } else if (period == "month") {
                timeStr = dt.isValid() ? dt.toString("dd/MM/yyyy HH:mm") : e.value("recorded_at").toString();
            } else {
                timeStr = dt.isValid() ? dt.toString("dd/MM/yyyy HH:mm") : e.value("recorded_at").toString();
            }
            m_historyTable->setItem(r, 0, new QTableWidgetItem(timeStr));

            auto m = e.value("metrics").toObject();
            double pVal = m.value("pressure_hpa").toDouble(1013.25);
            double tVal = m.value("temperature_c").toDouble(25.0);
            bool motion = m.value("ir_detected").toDouble() >= 0.5;

            // Pressure item
            auto *pItem = new QTableWidgetItem(QStringLiteral("%1 hPa").arg(pVal, 0, 'f', 1));
            pItem->setForeground(QColor("#00f2fe"));
            m_historyTable->setItem(r, 1, pItem);

            // Temperature item
            auto *tItem = new QTableWidgetItem(QStringLiteral("%1 °C").arg(tVal, 0, 'f', 1));
            tItem->setForeground(QColor("#ff9100"));
            m_historyTable->setItem(r, 2, tItem);

            // Motion item
            auto *mItem = new QTableWidgetItem(motion ? QStringLiteral("[!] PHÁT HIỆN!") : QStringLiteral("[OK] An toàn"));
            mItem->setForeground(motion ? QColor("#ff1744") : QColor("#10b981"));
            m_historyTable->setItem(r, 3, mItem);

            // Weather evaluation item
            QString eval;
            if (pVal < 995.0) eval = QStringLiteral("Áp thấp / Khả năng có mưa giông");
            else if (pVal > 1025.0) eval = QStringLiteral("Áp cao / Khô ráo ổn định");
            else eval = QStringLiteral("Khí áp chuẩn Trái Đất (1.0 ATM)");
            auto *eItem = new QTableWidgetItem(eval);
            eItem->setForeground(QColor("#93c5fd"));
            m_historyTable->setItem(r, 4, eItem);
        }
    }

    // 2. Extract chronological data points for charts (Oldest -> Newest)
    int maxPoints = 28;
    if (period == "month") maxPoints = 60;
    else if (period == "year") maxPoints = 120;
    const int N = qMin(rows.size(), maxPoints);

    QVector<double> pValues, tValues, mValues;
    QStringList cats;

    if (N > 0) {
        for (int i = N - 1; i >= 0; --i) {
            auto e = rows[i].toObject();
            auto m = e.value("metrics").toObject();
            pValues.append(m.value("pressure_hpa").toDouble(1013.25));
            tValues.append(m.value("temperature_c").toDouble(25.0));
            mValues.append(m.value("ir_detected").toDouble(0.0));

            QDateTime dt = QDateTime::fromString(e.value("recorded_at").toString(), Qt::ISODateWithMs).toLocalTime();
            QString tStr;
            if (period == "year") {
                tStr = dt.isValid() ? dt.toString("MM/yy") : QString::number(N - i);
            } else if (period == "month") {
                tStr = dt.isValid() ? dt.toString("dd/MM") : QString::number(N - i);
            } else {
                tStr = dt.isValid() ? dt.toString("HH:mm") : QString::number(N - i);
            }
            cats.append(tStr);
        }
    } else {
        pValues.clear();
        tValues.clear();
        mValues.clear();
        cats.clear();
    }

    // 3. Compute statistical telemetry
    double pMin = 0.0, pMax = 0.0, pAvg = 0.0, pLatest = 0.0, pOldest = 0.0, pDelta = 0.0, pAltitude = 0.0;
    if (!pValues.isEmpty()) {
        pMin = std::numeric_limits<double>::max();
        pMax = std::numeric_limits<double>::lowest();
        double pSum = 0.0;
        for (double v : pValues) {
            pMin = std::min(pMin, v);
            pMax = std::max(pMax, v);
            pSum += v;
        }
        pAvg = pSum / pValues.size();
        pLatest = pValues.last();
        pOldest = pValues.first();
        pDelta = pLatest - pOldest;
        pAltitude = 44330.0 * (1.0 - std::pow(pLatest / 1013.25, 0.19029));
    }

    double tMin = 0.0, tMax = 0.0, tAvg = 0.0, tLatest = 0.0;
    if (!tValues.isEmpty()) {
        tMin = std::numeric_limits<double>::max();
        tMax = std::numeric_limits<double>::lowest();
        double tSum = 0.0;
        for (double v : tValues) {
            tMin = std::min(tMin, v);
            tMax = std::max(tMax, v);
            tSum += v;
        }
        tAvg = tSum / tValues.size();
        tLatest = tValues.last();
    }

    QString pWeatherEval;
    if (rows.isEmpty()) pWeatherEval = QStringLiteral("0 bản ghi (Không có số đo)");
    else if (pLatest < 995.0) pWeatherEval = QStringLiteral("Vùng áp thấp / Mưa giông");
    else if (pLatest > 1022.0) pWeatherEval = QStringLiteral("Vùng áp cao / Nắng ráo");
    else pWeatherEval = QStringLiteral("Chuẩn 1.0 ATM (Ổn định)");

    QString pTrend;
    if (rows.isEmpty()) pTrend = QStringLiteral("0.0 hPa (0 bản ghi)");
    else pTrend = QStringLiteral("%1%2 hPa (%3)")
        .arg(pDelta >= 0 ? "+" : "")
        .arg(pDelta, 0, 'f', 1)
        .arg(std::abs(pDelta) <= 1.0 ? "Ổn định" : (pDelta < 0 ? "Giảm" : "Tăng"));

    int motionCount = 0;
    QString lastMotionTime;
    for (int i = 0; i < mValues.size(); ++i) {
        if (mValues[i] >= 0.5) {
            motionCount++;
            lastMotionTime = cats.isEmpty() ? QString() : cats[i];
        }
    }
    const bool isCurrentlyMotion = !mValues.isEmpty() && (mValues.last() >= 0.5);
    const double motionDuty = mValues.isEmpty() ? 0.0 : ((double(motionCount) / mValues.size()) * 100.0);

    QString tComfort;
    if (rows.isEmpty()) tComfort = QStringLiteral("0 bản ghi");
    else if (tAvg < 18.0) tComfort = QStringLiteral("Thời tiết lạnh");
    else if (tAvg > 32.0) tComfort = QStringLiteral("Thời tiết rất nóng");
    else tComfort = QStringLiteral("Vùng nhiệt độ tiện nghi");

    // 4. Update KPI Labels
    if (m_histPressCur) m_histPressCur->setText(rows.isEmpty() ? QStringLiteral("--.- hPa") : QStringLiteral("%1 hPa").arg(pLatest, 0, 'f', 1));
    if (m_histPressDelta) m_histPressDelta->setText(pTrend);
    if (m_histPressEval) m_histPressEval->setText(pWeatherEval);
    if (m_histPressAlt) m_histPressAlt->setText(rows.isEmpty() ? QStringLiteral("-- m") : QStringLiteral("~%1 m ASL").arg(int(std::round(pAltitude))));
    if (m_histPressRange) {
        if (rows.isEmpty()) {
            m_histPressRange->setText(QStringLiteral("Không có dữ liệu cho %1. Nhấn '<' để lùi ngày/tháng/năm.").arg(periodTitle));
        } else {
            m_histPressRange->setText(QStringLiteral("[%1] Mẫu: %2 | Min: %3 hPa | Max: %4 hPa | TB: %5 hPa | Mốc: %6 ── %7")
                .arg(periodTitle)
                .arg(pValues.size())
                .arg(pMin, 0, 'f', 1)
                .arg(pMax, 0, 'f', 1)
                .arg(pAvg, 0, 'f', 1)
                .arg(cats.first())
                .arg(cats.last()));
        }
    }

    if (m_histMotionCur) {
        if (rows.isEmpty()) {
            m_histMotionCur->setText(QStringLiteral("CHƯA CÓ DỮ LIỆU"));
            m_histMotionCur->setStyleSheet("color: #94a3b8; font-size: 13px; font-weight: bold;");
        } else {
            m_histMotionCur->setText(isCurrentlyMotion ? QStringLiteral("[!] CÓ CHUYỂN ĐỘNG!") : QStringLiteral("[OK] AN TOÀN"));
            m_histMotionCur->setStyleSheet(isCurrentlyMotion ? "color: #ff1744; font-size: 13px; font-weight: 900;" : "color: #10b981; font-size: 13px; font-weight: 900;");
        }
    }
    if (m_histMotionCount) m_histMotionCount->setText(QStringLiteral("%1 LẦN").arg(motionCount));
    if (m_histMotionLast) m_histMotionLast->setText(lastMotionTime.isEmpty() ? QStringLiteral("Không có") : lastMotionTime);
    if (m_histMotionDuty) m_histMotionDuty->setText(QStringLiteral("%1% Chu kỳ").arg(motionDuty, 0, 'f', 1));
    if (m_histMotionSummary) {
        if (rows.isEmpty()) {
            m_histMotionSummary->setText(QStringLiteral("Chưa ghi nhận sự kiện chuyển động trong %1.").arg(periodTitle));
        } else {
            m_histMotionSummary->setText(QStringLiteral("[%1] Mẫu ghi: %2 | Kích hoạt: %3 lần (%4% thời lượng) | Lần cuối: %5")
                .arg(periodTitle)
                .arg(mValues.size())
                .arg(motionCount)
                .arg(motionDuty, 0, 'f', 1)
                .arg(lastMotionTime.isEmpty() ? "Chưa có" : lastMotionTime));
        }
    }

    if (m_histTempCur) m_histTempCur->setText(rows.isEmpty() ? QStringLiteral("--.- °C") : QStringLiteral("%1 °C").arg(tLatest, 0, 'f', 1));
    if (m_histTempMin) m_histTempMin->setText(rows.isEmpty() ? QStringLiteral("--.- °C") : QStringLiteral("%1 °C").arg(tMin, 0, 'f', 1));
    if (m_histTempMax) m_histTempMax->setText(rows.isEmpty() ? QStringLiteral("--.- °C") : QStringLiteral("%1 °C").arg(tMax, 0, 'f', 1));
    if (m_histTempAvg) m_histTempAvg->setText(rows.isEmpty() ? QStringLiteral("--.- °C") : QStringLiteral("%1 °C (%2)").arg(tAvg, 0, 'f', 1).arg(tComfort));
    if (m_histTempSummary) {
        if (rows.isEmpty()) {
            m_histTempSummary->setText(QStringLiteral("Chưa ghi nhận nhiệt độ trong %1.").arg(periodTitle));
        } else {
            m_histTempSummary->setText(QStringLiteral("[%1] Mẫu ghi: %2 | Biên độ nhiệt: ΔT = %3 °C | Vùng tiện nghi: 22 - 28 °C")
                .arg(periodTitle)
                .arg(tValues.size())
                .arg(tMax - tMin, 0, 'f', 1));
        }
    }

    // 5. Build Dedicated Barogram Chart (Page 0)
    if (m_histPressView) {
        auto *pChart = new QChart;
        pChart->legend()->setVisible(true);
        pChart->legend()->setAlignment(Qt::AlignTop);
        pChart->legend()->setLabelColor(QColor("#93c5fd"));
        QFont legFont = pChart->legend()->font();
        legFont.setPointSize(8);
        legFont.setBold(true);
        pChart->legend()->setFont(legFont);
        pChart->setBackgroundVisible(false);
        pChart->setAnimationOptions(QChart::NoAnimation);
        pChart->setMargins(QMargins(6, 2, 6, 2));

        auto *pLine = new QLineSeries;
        pLine->setName("Áp suất thực đo BMP180 (hPa)");
        pLine->setPen(QPen(QColor("#00f2fe"), 2.5));
        pLine->setPointsVisible(true);

        auto *pRefLine = new QLineSeries;
        pRefLine->setName("Chuẩn 1 ATM (1013.25 hPa)");
        pRefLine->setPen(QPen(QColor("#38bdf8"), 1.5, Qt::DashLine));

        if (!pValues.isEmpty()) {
            for (int i = 0; i < pValues.size(); ++i) {
                pLine->append(i, pValues[i]);
                pRefLine->append(i, 1013.25);
            }
        }
        pChart->addSeries(pLine);
        pChart->addSeries(pRefLine);

        auto *axX = new QValueAxis;
        axX->setRange(0, qMax(1, int(pValues.size()) - 1));
        axX->setLabelsColor(QColor("#93c5fd"));
        axX->setLinePen(QPen(QColor("#25386b"), 1));
        axX->setGridLinePen(QPen(QColor("#16234d"), 1, Qt::DotLine));
        axX->setTickCount(qMin(7, qMax(2, int(pValues.size()))));
        axX->setLabelFormat("%d");

        double pYMin = pValues.isEmpty() ? 0.0 : std::floor(std::min(985.0, pMin - 2.0) / 5.0) * 5.0;
        double pYMax = pValues.isEmpty() ? 1200.0 : std::ceil(std::max(1030.0, pMax + 2.0) / 5.0) * 5.0;
        auto *axY = new QValueAxis;
        axY->setRange(pYMin, pYMax);
        axY->setLabelsColor(QColor("#00f2fe"));
        axY->setLinePen(QPen(QColor("#25386b"), 1));
        axY->setGridLinePen(QPen(QColor("#16234d"), 1, Qt::DotLine));
        axY->setTickCount(6);
        axY->setLabelFormat("%.0f hPa");

        pChart->addAxis(axX, Qt::AlignBottom);
        pChart->addAxis(axY, Qt::AlignLeft);
        pLine->attachAxis(axX);
        pLine->attachAxis(axY);
        pRefLine->attachAxis(axX);
        pRefLine->attachAxis(axY);
        if (pValues.isEmpty()) {
            pChart->setTitle(QStringLiteral("[0 BẢN GHI] KHÔNG CÓ DỮ LIỆU ĐO KHÍ ÁP"));
            pChart->setTitleBrush(QBrush(QColor("#64748b")));
            QFont tf = pChart->titleFont(); tf.setPointSize(9); tf.setBold(true); pChart->setTitleFont(tf);
        }
        m_histPressView->setChart(pChart);
    }

    // 6. Build Dedicated Motion Pulse Chart (Page 1)
    if (m_histMotionView) {
        auto *mChart = new QChart;
        mChart->legend()->setVisible(true);
        mChart->legend()->setAlignment(Qt::AlignTop);
        mChart->legend()->setLabelColor(QColor("#93c5fd"));
        QFont legFont = mChart->legend()->font();
        legFont.setPointSize(8);
        legFont.setBold(true);
        mChart->legend()->setFont(legFont);
        mChart->setBackgroundVisible(false);
        mChart->setAnimationOptions(QChart::NoAnimation);
        mChart->setMargins(QMargins(6, 2, 6, 2));

        auto *mBarSeries = new QBarSeries;
        mBarSeries->setName("Mức Logic PIR (0: Yên tĩnh | 1: Phát hiện)");
        mBarSeries->setBarWidth(0.5);

        if (!mValues.isEmpty()) {
            auto *mBarSet = new QBarSet("Xung Chuyển Động");
            mBarSet->setColor(QColor("#ff1744"));
            mBarSet->setBorderColor(Qt::transparent);
            for (double v : mValues) *mBarSet << (v >= 0.5 ? 1.0 : 0.04);
            mBarSeries->append(mBarSet);
        }
        mChart->addSeries(mBarSeries);

        auto *axX = new QBarCategoryAxis;
        if (!cats.isEmpty()) axX->append(cats);
        axX->setLabelsColor(QColor("#93c5fd"));
        axX->setLabelsAngle(-25);
        axX->setLinePen(QPen(QColor("#25386b"), 1));
        axX->setGridLineVisible(false);

        auto *axY = new QValueAxis;
        axY->setRange(0.0, 1.2);
        axY->setTickCount(3);
        axY->setLabelFormat("%.0f");
        axY->setLabelsColor(QColor("#ff1744"));
        axY->setLinePen(QPen(QColor("#25386b"), 1));
        axY->setGridLinePen(QPen(QColor("#16234d"), 1, Qt::DotLine));

        mChart->addAxis(axX, Qt::AlignBottom);
        mChart->addAxis(axY, Qt::AlignLeft);
        mBarSeries->attachAxis(axX);
        mBarSeries->attachAxis(axY);
        if (mValues.isEmpty()) {
            mChart->setTitle(QStringLiteral("[0 BẢN GHI] KHÔNG CÓ DỮ LIỆU CHUYỂN ĐỘNG"));
            mChart->setTitleBrush(QBrush(QColor("#64748b")));
            QFont tf = mChart->titleFont(); tf.setPointSize(9); tf.setBold(true); mChart->setTitleFont(tf);
        }
        m_histMotionView->setChart(mChart);
    }

    // 7. Build Dedicated Thermogram Chart (Page 2)
    if (m_histTempView) {
        auto *tChart = new QChart;
        tChart->legend()->setVisible(true);
        tChart->legend()->setAlignment(Qt::AlignTop);
        tChart->legend()->setLabelColor(QColor("#93c5fd"));
        QFont legFont = tChart->legend()->font();
        legFont.setPointSize(8);
        legFont.setBold(true);
        tChart->legend()->setFont(legFont);
        tChart->setBackgroundVisible(false);
        tChart->setAnimationOptions(QChart::NoAnimation);
        tChart->setMargins(QMargins(6, 2, 6, 2));

        auto *tLine = new QSplineSeries;
        tLine->setName("Nhiệt độ BMP180 (°C)");
        tLine->setPen(QPen(QColor("#ff9100"), 2.5));
        tLine->setPointsVisible(true);

        auto *tRef = new QLineSeries;
        tRef->setName("Mức tiện nghi (25.0 °C)");
        tRef->setPen(QPen(QColor("#10b981"), 1.5, Qt::DashLine));

        if (!tValues.isEmpty()) {
            for (int i = 0; i < tValues.size(); ++i) {
                tLine->append(i, tValues[i]);
                tRef->append(i, 25.0);
            }
        }
        tChart->addSeries(tLine);
        tChart->addSeries(tRef);

        auto *axX = new QValueAxis;
        axX->setRange(0, qMax(1, int(tValues.size()) - 1));
        axX->setLabelsColor(QColor("#93c5fd"));
        axX->setLinePen(QPen(QColor("#25386b"), 1));
        axX->setGridLinePen(QPen(QColor("#16234d"), 1, Qt::DotLine));
        axX->setTickCount(qMin(7, qMax(2, int(tValues.size()))));
        axX->setLabelFormat("%d");

        double tYMin = tValues.isEmpty() ? 0.0 : std::floor(std::max(0.0, tMin - 3.0) / 5.0) * 5.0;
        double tYMax = tValues.isEmpty() ? 50.0 : std::ceil((tMax + 3.0) / 5.0) * 5.0;
        if (tYMax <= tYMin) tYMax = tYMin + 10.0;
        auto *axY = new QValueAxis;
        axY->setRange(tYMin, tYMax);
        axY->setLabelsColor(QColor("#ff9100"));
        axY->setLinePen(QPen(QColor("#25386b"), 1));
        axY->setGridLinePen(QPen(QColor("#16234d"), 1, Qt::DotLine));
        axY->setTickCount(5);
        axY->setLabelFormat("%.1f °C");

        tChart->addAxis(axX, Qt::AlignBottom);
        tChart->addAxis(axY, Qt::AlignLeft);
        tLine->attachAxis(axX);
        tLine->attachAxis(axY);
        tRef->attachAxis(axX);
        tRef->attachAxis(axY);
        if (tValues.isEmpty()) {
            tChart->setTitle(QStringLiteral("[0 BẢN GHI] KHÔNG CÓ DỮ LIỆU NHIỆT ĐỘ"));
            tChart->setTitleBrush(QBrush(QColor("#64748b")));
            QFont tf = tChart->titleFont(); tf.setPointSize(9); tf.setBold(true); tChart->setTitleFont(tf);
        }
        m_histTempView->setChart(tChart);
    }

    // 8. Build Synchronized Multi-stream Overview Charts (Page 3)
    if (m_histMultiPressView) {
        auto *c = new QChart;
        c->legend()->hide();
        c->setBackgroundVisible(false);
        c->setAnimationOptions(QChart::NoAnimation);
        c->setMargins(QMargins(4, 2, 4, 2));

        auto *s = new QLineSeries;
        s->setPen(QPen(QColor("#00f2fe"), 2.0));
        if (!pValues.isEmpty()) {
            for (int i = 0; i < pValues.size(); ++i) s->append(i, pValues[i]);
        }
        c->addSeries(s);

        auto *axX = new QValueAxis;
        axX->setRange(0, qMax(1, int(pValues.size()) - 1));
        axX->setLabelsVisible(false);
        axX->setGridLinePen(QPen(QColor("#16234d"), 1, Qt::DotLine));

        double pYMin = pValues.isEmpty() ? 0.0 : std::floor(std::min(985.0, pMin - 2.0) / 5.0) * 5.0;
        double pYMax = pValues.isEmpty() ? 1200.0 : std::ceil(std::max(1030.0, pMax + 2.0) / 5.0) * 5.0;
        auto *axY = new QValueAxis;
        axY->setRange(pYMin, pYMax);
        axY->setLabelsColor(QColor("#00f2fe"));
        QFont f = axY->labelsFont(); f.setPointSize(7); axY->setLabelsFont(f);
        axY->setTickCount(4);
        axY->setLabelFormat("%.0f");
        axY->setGridLinePen(QPen(QColor("#16234d"), 1, Qt::DotLine));

        c->addAxis(axX, Qt::AlignBottom);
        c->addAxis(axY, Qt::AlignLeft);
        s->attachAxis(axX);
        s->attachAxis(axY);
        m_histMultiPressView->setChart(c);
    }

    if (m_histMultiMotionView) {
        auto *c = new QChart;
        c->legend()->hide();
        c->setBackgroundVisible(false);
        c->setAnimationOptions(QChart::NoAnimation);
        c->setMargins(QMargins(4, 2, 4, 2));

        auto *bs = new QBarSeries;
        if (!mValues.isEmpty()) {
            auto *set = new QBarSet("motion");
            set->setColor(QColor("#ff1744"));
            set->setBorderColor(Qt::transparent);
            for (double v : mValues) *set << (v >= 0.5 ? 1.0 : 0.05);
            bs->append(set);
        }
        c->addSeries(bs);

        auto *axX = new QBarCategoryAxis;
        if (!cats.isEmpty()) axX->append(cats);
        axX->setLabelsVisible(false);
        axX->setGridLineVisible(false);

        auto *axY = new QValueAxis;
        axY->setRange(0.0, 1.2);
        axY->setLabelsColor(QColor("#ff1744"));
        QFont f = axY->labelsFont(); f.setPointSize(7); axY->setLabelsFont(f);
        axY->setTickCount(3);
        axY->setLabelFormat("%.0f");
        axY->setGridLinePen(QPen(QColor("#16234d"), 1, Qt::DotLine));

        c->addAxis(axX, Qt::AlignBottom);
        c->addAxis(axY, Qt::AlignLeft);
        bs->attachAxis(axX);
        bs->attachAxis(axY);
        m_histMultiMotionView->setChart(c);
    }

    if (m_histMultiTempView) {
        auto *c = new QChart;
        c->legend()->hide();
        c->setBackgroundVisible(false);
        c->setAnimationOptions(QChart::NoAnimation);
        c->setMargins(QMargins(4, 2, 4, 2));

        auto *s = new QLineSeries;
        s->setPen(QPen(QColor("#ff9100"), 2.0));
        if (!tValues.isEmpty()) {
            for (int i = 0; i < tValues.size(); ++i) s->append(i, tValues[i]);
        }
        c->addSeries(s);

        auto *axX = new QValueAxis;
        axX->setRange(0, qMax(1, int(tValues.size()) - 1));
        axX->setLabelsColor(QColor("#93c5fd"));
        QFont fX = axX->labelsFont(); fX.setPointSize(7); axX->setLabelsFont(fX);
        axX->setGridLinePen(QPen(QColor("#16234d"), 1, Qt::DotLine));

        double tYMin = tValues.isEmpty() ? 0.0 : std::floor(std::max(0.0, tMin - 3.0) / 5.0) * 5.0;
        double tYMax = tValues.isEmpty() ? 50.0 : std::ceil((tMax + 3.0) / 5.0) * 5.0;
        if (tYMax <= tYMin) tYMax = tYMin + 10.0;
        auto *axY = new QValueAxis;
        axY->setRange(tYMin, tYMax);
        axY->setLabelsColor(QColor("#ff9100"));
        QFont fY = axY->labelsFont(); fY.setPointSize(7); axY->setLabelsFont(fY);
        axY->setTickCount(4);
        axY->setLabelFormat("%.0f");
        axY->setGridLinePen(QPen(QColor("#16234d"), 1, Qt::DotLine));

        c->addAxis(axX, Qt::AlignBottom);
        c->addAxis(axY, Qt::AlignLeft);
        s->attachAxis(axX);
        s->attachAxis(axY);
        m_histMultiTempView->setChart(c);
    }
}

void MainWindow::showChartZoomDialog(const QString &key)
{
    if (m_lastHistory.isEmpty()) return;
    const auto rows = m_lastHistory.value("data").toArray();
    if (rows.isEmpty()) return;

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Phóng to biểu đồ"));
    dialog.setModal(true);
    dialog.resize(qMax(400, width() - 30), qMax(300, height() - 30));
    dialog.setStyleSheet(QStringLiteral("QDialog { background-color: #081024; border: 2px solid #00f2fe; border-radius: 12px; }"));

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(10);

    auto *head = new QHBoxLayout;
    auto *title = label(friendlyMetricTitle(key), "chartZoomTitle");
    title->setStyleSheet(QStringLiteral("color: #00f2fe; font-size: 15px; font-weight: 900;"));
    head->addWidget(title);
    head->addStretch();
    auto *closeBtn = new QPushButton(QStringLiteral("✕ Đóng"), &dialog);
    closeBtn->setFixedSize(70, 30);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(QStringLiteral("QPushButton { background: #16244e; border: 1px solid #304482; border-radius: 8px; color: #ffffff; font-weight: bold; } QPushButton:hover { background: #ff1744; }"));
    head->addWidget(closeBtn);
    root->addLayout(head);

    QList<double> values;
    QStringList cats;
    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::lowest();
    double sum = 0.0;
    int motionCount = 0;

    const QString currentPeriod = m_lastHistory.value("period").toString(m_historyPeriod ? m_historyPeriod->currentData().toString() : QStringLiteral("day"));
    for (int r = rows.size() - 1; r >= 0; --r) {
        auto e = rows[r].toObject();
        QDateTime dt = QDateTime::fromString(e.value("recorded_at").toString(), Qt::ISODateWithMs).toLocalTime();
        if (!dt.isValid()) dt = QDateTime::fromString(e.value("recorded_at").toString(), Qt::ISODate).toLocalTime();
        QString timeLabel;
        if (dt.isValid()) {
            if (currentPeriod == "year") {
                timeLabel = dt.toString(QStringLiteral("MM/yyyy"));
            } else if (currentPeriod == "month") {
                timeLabel = dt.toString(QStringLiteral("dd/MM HH:mm"));
            } else {
                timeLabel = dt.toString(QStringLiteral("HH:mm:ss"));
            }
        } else {
            timeLabel = e.value("recorded_at").toString();
        }
        cats << timeLabel;

        const double value = e.value("metrics").toObject().value(key).toDouble();
        values.append(value);
        minValue = std::min(minValue, value);
        maxValue = std::max(maxValue, value);
        sum += value;
        if (key == "ir_detected" && value >= 0.5) motionCount++;
    }

    auto *chart = new QChart;
    chart->legend()->hide();
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(4, 4, 4, 4));

    if (key == "ir_detected") {
        auto *barSet = new QBarSet("ir_detected");
        barSet->setColor(QColor("#ff1744"));
        barSet->setBorderColor(Qt::transparent);
        for (double v : values) *barSet << (v >= 0.5 ? 1.0 : 0.05);

        auto *series = new QBarSeries;
        series->setBarWidth(0.55);
        series->append(barSet);
        chart->addSeries(series);

        auto *ax = new QBarCategoryAxis;
        ax->append(cats);
        ax->setLabelsColor(QColor("#93c5fd"));
        ax->setLabelsAngle(-35);
        QFont axisFont = ax->labelsFont();
        axisFont.setPointSize(8);
        axisFont.setBold(true);
        ax->setLabelsFont(axisFont);
        ax->setLinePen(QPen(QColor("#25386b"), 1));
        ax->setGridLinePen(QPen(QColor("#16234d"), 1, Qt::DotLine));

        auto *ay = new QValueAxis;
        ay->setLabelsColor(QColor("#93c5fd"));
        ay->setLabelsFont(axisFont);
        ay->setLinePen(QPen(QColor("#25386b"), 1));
        ay->setGridLinePen(QPen(QColor("#16234d"), 1, Qt::DotLine));
        ay->setRange(0.0, 1.2);
        ay->setTickCount(3);
        ay->setLabelFormat("%.0f");

        chart->addAxis(ax, Qt::AlignBottom);
        chart->addAxis(ay, Qt::AlignLeft);
        series->attachAxis(ax);
        series->attachAxis(ay);
    } else {
        auto *lineSeries = new QLineSeries;
        lineSeries->setName(friendlyMetricTitle(key));
        QPen linePen(metricChartColor(key), 3.0);
        lineSeries->setPen(linePen);
        for (int i = 0; i < values.size(); ++i) {
            lineSeries->append(i, values[i]);
        }
        chart->addSeries(lineSeries);

        auto *ax = new QValueAxis;
        ax->setRange(0, qMax(1, int(values.size()) - 1));
        ax->setLabelsColor(QColor("#93c5fd"));
        QFont axisFont = ax->labelsFont();
        axisFont.setPointSize(8);
        axisFont.setBold(true);
        ax->setLabelsFont(axisFont);
        ax->setLinePen(QPen(QColor("#25386b"), 1));
        ax->setGridLinePen(QPen(QColor("#16234d"), 1, Qt::DotLine));

        auto *ay = new QValueAxis;
        ay->setLabelsColor(QColor("#93c5fd"));
        ay->setLabelsFont(axisFont);
        ay->setLinePen(QPen(QColor("#25386b"), 1));
        ay->setGridLinePen(QPen(QColor("#16234d"), 1, Qt::DotLine));

        if (key == "pressure_hpa") {
            ay->setRange(std::min(985.0, minValue - 2.0), std::max(1030.0, maxValue + 2.0));
            ay->setLabelFormat("%.0f hPa");
        } else if (key.contains("temp")) {
            ay->setRange(std::max(0.0, minValue - 3.0), maxValue + 3.0);
            ay->setLabelFormat("%.1f °C");
        } else {
            ay->setRange(std::max(0.0, minValue - 2.0), maxValue + 2.0);
            ay->setLabelFormat("%.1f");
        }
        ay->setTickCount(6);

        chart->addAxis(ax, Qt::AlignBottom);
        chart->addAxis(ay, Qt::AlignLeft);
        lineSeries->attachAxis(ax);
        lineSeries->attachAxis(ay);
    }

    auto *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    root->addWidget(view, 1);

    auto *bottom = new QHBoxLayout;
    QString statText;
    if (key == "ir_detected") {
        statText = QStringLiteral("Tổng số mẫu: %1 | Số lần phát hiện chuyển động: %2 lần | Trạng thái: %3")
            .arg(values.size())
            .arg(motionCount)
            .arg(values.isEmpty() || values.last() < 0.5 ? "AN TOÀN" : "CẢNH BÁO");
    } else {
        statText = QStringLiteral("Mẫu: %1 | Min: %2 | Max: %3 | Trung Bình: %4")
            .arg(values.size())
            .arg(QString::number(minValue, 'f', 1))
            .arg(QString::number(maxValue, 'f', 1))
            .arg(QString::number(values.isEmpty() ? 0.0 : sum / values.size(), 'f', 1));
    }
    auto *statBadge = label(statText, "chartZoomStat");
    statBadge->setStyleSheet(QStringLiteral("background: #0f1c3e; border: 1px solid #273e7c; border-radius: 8px; color: #00f2fe; font-size: 11px; font-weight: bold; padding: 6px 12px;"));
    bottom->addWidget(statBadge);
    bottom->addStretch();
    auto *btn = new QPushButton(tr("Đóng"), &dialog);
    btn->setStyleSheet(QStringLiteral("QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #3b82f6); color: #030818; border: none; border-radius: 8px; font-weight: 900; min-height: 30px; padding: 0 16px; }"));
    bottom->addWidget(btn);
    root->addLayout(bottom);

    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(btn, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.exec();
}

void MainWindow::renderUsers()
{
    m_usersTable->setRowCount(m_users.size());
    for (int i = 0; i < m_users.size(); ++i) {
        auto u = m_users[i].toObject();
        m_usersTable->setItem(i, 0, new QTableWidgetItem(u.value("username").toString()));
        m_usersTable->setItem(i, 1, new QTableWidgetItem(u.value("role").toString()));
        auto ids = u.value("device_ids").toArray();
        QStringList s;
        for (auto id : ids) s << id.toString();
        m_usersTable->setItem(i, 2, new QTableWidgetItem(s.join(", ")));
        m_usersTable->setItem(i, 3, new QTableWidgetItem(u.value("enabled").toBool() ? "Đang hoạt động" : "Bị khóa"));
    }
}

void MainWindow::renderLoginHistory()
{
    if (!m_loginTable) return;
    m_loginTable->setRowCount(0);
    m_loginTable->setRowCount(m_loginHistory.size());

    for (int r = 0; r < m_loginHistory.size(); ++r) {
        const QJsonObject item = m_loginHistory.at(r).toObject();
        auto *timeItem = new QTableWidgetItem(item.value("created_at").toString());
        timeItem->setTextAlignment(Qt::AlignCenter);

        auto *userItem = new QTableWidgetItem(item.value("username").toString());
        userItem->setTextAlignment(Qt::AlignCenter);
        userItem->setForeground(QColor("#38bdf8"));

        const QString role = item.value("role").toString();
        auto *roleItem = new QTableWidgetItem(role == "admin" ? "Quản trị viên" : "Người dùng");
        roleItem->setTextAlignment(Qt::AlignCenter);

        auto *ipItem = new QTableWidgetItem(item.value("ip_address").toString());
        ipItem->setTextAlignment(Qt::AlignCenter);

        const QString status = item.value("status").toString();
        auto *statusItem = new QTableWidgetItem(status == "success" ? "Thành công" : "Thất bại");
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (status == "success") {
            statusItem->setForeground(QColor("#22c55e"));
        } else {
            statusItem->setForeground(QColor("#ef4444"));
        }

        m_loginTable->setItem(r, 0, timeItem);
        m_loginTable->setItem(r, 1, userItem);
        m_loginTable->setItem(r, 2, roleItem);
        m_loginTable->setItem(r, 3, ipItem);
        m_loginTable->setItem(r, 4, statusItem);
    }
    if (m_loginSummaryLabel) {
        m_loginSummaryLabel->setText(QString("Tổng cộng %1 lượt đăng nhập được ghi nhận").arg(m_loginHistory.size()));
    }
}

void MainWindow::renderAuditLogs()
{
    if (!m_auditTable) return;
    const QString filter = m_auditSearchEdit ? m_auditSearchEdit->text().trimmed().toLower() : QString();

    QJsonArray filtered;
    for (const auto &val : m_auditLogs) {
        const QJsonObject item = val.toObject();
        if (filter.isEmpty()) {
            filtered.append(item);
        } else {
            const QString u = item.value("username").toString().toLower();
            const QString a = item.value("action").toString().toLower();
            const QString t = item.value("target").toString().toLower();
            const QString d = item.value("details").toString().toLower();
            if (u.contains(filter) || a.contains(filter) || t.contains(filter) || d.contains(filter)) {
                filtered.append(item);
            }
        }
    }

    m_auditTable->setRowCount(0);
    m_auditTable->setRowCount(filtered.size());

    for (int r = 0; r < filtered.size(); ++r) {
        const QJsonObject item = filtered.at(r).toObject();
        auto *timeItem = new QTableWidgetItem(item.value("created_at").toString());
        timeItem->setTextAlignment(Qt::AlignCenter);

        auto *userItem = new QTableWidgetItem(item.value("username").toString());
        userItem->setTextAlignment(Qt::AlignCenter);
        userItem->setForeground(QColor("#38bdf8"));

        const QString action = item.value("action").toString();
        auto *actionItem = new QTableWidgetItem(action);
        actionItem->setTextAlignment(Qt::AlignCenter);
        if (action.contains("RƠ-LE") || action.contains("BẬT") || action.contains("TẮT")) {
            actionItem->setForeground(QColor("#f59e0b"));
        } else if (action.contains("CẤU HÌNH")) {
            actionItem->setForeground(QColor("#a855f7"));
        } else if (action.contains("TÀI KHOẢN")) {
            actionItem->setForeground(QColor("#3b82f6"));
        } else {
            actionItem->setForeground(QColor("#06b6d4"));
        }

        auto *targetItem = new QTableWidgetItem(item.value("target").toString());
        targetItem->setTextAlignment(Qt::AlignCenter);

        auto *detailsItem = new QTableWidgetItem(item.value("details").toString());
        detailsItem->setForeground(QColor("#e2e8f0"));

        m_auditTable->setItem(r, 0, timeItem);
        m_auditTable->setItem(r, 1, userItem);
        m_auditTable->setItem(r, 2, actionItem);
        m_auditTable->setItem(r, 3, targetItem);
        m_auditTable->setItem(r, 4, detailsItem);
    }
    if (m_auditSummaryLabel) {
        if (m_role == "admin") {
            m_auditSummaryLabel->setText(QString("Nhật ký thao tác hệ thống: hiển thị %1 / %2 bản ghi").arg(filtered.size()).arg(m_auditLogs.size()));
        } else {
            m_auditSummaryLabel->setText(QString("Nhật ký thao tác của bạn (%1): hiển thị %2 bản ghi").arg(m_username).arg(filtered.size()));
        }
    }
}

void MainWindow::updateDeviceConfig(const QString &deviceId, const QJsonObject &config)
{
    m_deviceConfigs[deviceId] = config;
    for (int i = 0; i < m_devices.size(); ++i) {
        auto obj = m_devices[i].toObject();
        if (obj.value("device_id").toString() == deviceId) {
            obj.insert("config", config);
            m_devices[i] = obj;
            break;
        }
    }
    post("/api/devices/config", {{"device_id", deviceId}, {"config", config}}, [this](QJsonObject) {
        refreshDevices();
        showCustomMessageBox(this, QMessageBox::Information, tr("Đã gửi"), tr("Đã gửi cấu hình xuống thiết bị thành công."));
    });
}

void MainWindow::openDeviceConfigDialog(const QString &deviceId)
{
    QJsonObject device;
    for (const auto &v : m_devices) {
        auto obj = v.toObject();
        if (obj.value("device_id").toString() == deviceId) {
            device = obj;
            break;
        }
    }
    const QString type = device.value("device_type").toString();
    QJsonObject current = device.value("config").toObject();
    if (m_deviceConfigs.contains(deviceId)) {
        const auto cached = m_deviceConfigs.value(deviceId);
        for (auto it = cached.begin(); it != cached.end(); ++it) {
            current.insert(it.key(), it.value());
        }
    }

    QDialog dialog(this);
    dialog.setObjectName("configDialog");
    dialog.setWindowTitle(tr("Cài đặt ngưỡng cảnh báo áp suất khí quyển & chuyển động"));
    dialog.setFixedSize(480, 380);
    dialog.setStyleSheet(R"QSS(
        QDialog { background: #091024; border: 2px solid #00f2fe; border-radius: 12px; }
        QLabel { color: #ecf2ff; font-weight: 700; font-size: 12px; }
        QLineEdit { background: #0e1a3b; color: #ffffff; border: 1.5px solid #253b75; border-radius: 6px; padding: 4px 8px; min-height: 22px; font-weight: 800; }
        QLineEdit:focus { border: 1.5px solid #00f2fe; background: #132452; }
        QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #3b82f6); color: #030818; border: none; border-radius: 6px; padding: 6px 14px; font-weight: 900; }
        QPushButton#cancel { background: #1e293b; color: #dce7ff; border: 1px solid #334155; }
    )QSS");

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(8);

    root->addWidget(label(tr("Cài đặt ngưỡng cảnh báo · %1 (%2)").arg(device.value("name").toString(deviceId), deviceId), "pageTitle"));

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(6);
    QVector<QPair<QString, QLineEdit*>> inputs;

    auto getVal = [&](const QString &key, double fallback) -> double {
        if (current.contains(key)) return current.value(key).toDouble(fallback);
        const auto th = current.value("thresholds").toObject();
        if (key == "temperature_warn_c" && th.contains("temperature_c")) {
            return th.value("temperature_c").toObject().value("warning_above").toDouble(fallback);
        }
        if (key == "pressure_min_hpa" && th.contains("pressure_hpa")) {
            return th.value("pressure_hpa").toObject().value("min").toDouble(fallback);
        }
        if (key == "pressure_max_hpa" && th.contains("pressure_hpa")) {
            return th.value("pressure_hpa").toObject().value("max").toDouble(fallback);
        }
        if (key == "sampling_interval_seconds" && current.contains("sampling_interval_ms")) {
            return current.value("sampling_interval_ms").toDouble(fallback * 1000.0) / 1000.0;
        }
        return fallback;
    };

    auto addInput = [&](const QString &key, const QString &title, double fallback) {
        auto *edit = new QLineEdit(QString::number(getVal(key, fallback), 'f', 2), &dialog);
        form->addRow(title, edit);
        inputs.append({key, edit});
        VirtualKeyboardDialog::attachToLineEdit(edit, title);
    };

    if (type == "weather_pressure") {
        addInput("pressure_min_hpa", tr("Áp suất min (hPa)"), 990);
        addInput("pressure_max_hpa", tr("Áp suất max (hPa)"), 1030);
        addInput("temperature_warn_c", tr("Nhiệt độ cảnh báo (°C)"), 40);
        addInput("ir_alarm_seconds", tr("Báo động chuyển động IR (giây)"), 1);
        addInput("sampling_interval_seconds", tr("Chu kỳ gửi dữ liệu (giây)"), 2);
    } else if (type == "temperature_sound") {
        addInput("temperature_warn_c", tr("Nhiệt độ cảnh báo (°C)"), 40);
        addInput("temperature_danger_c", tr("Nhiệt độ nguy hiểm (°C)"), 50);
        addInput("sound_warn_vpp", tr("Âm thanh cảnh báo (Vpp)"), 1.5);
        addInput("sampling_interval_seconds", tr("Chu kỳ gửi (giây)"), 2);
    } else if (type == "uv_pressure") {
        addInput("uv_warn_index", tr("UV cảnh báo"), 6);
        addInput("uv_danger_index", tr("UV nguy hiểm"), 8);
        addInput("pressure_min_hpa", tr("Áp suất min (hPa)"), 990);
        addInput("pressure_max_hpa", tr("Áp suất max (hPa)"), 1030);
        addInput("sampling_interval_seconds", tr("Chu kỳ gửi (giây)"), 5);
    } else {
        addInput("sampling_interval_seconds", tr("Chu kỳ gửi (giây)"), 5);
    }
    root->addLayout(form);

    auto *actions = new QHBoxLayout;
    auto *cancel = button(tr("Hủy"), "cancel");
    auto *save = button(tr("Lưu cấu hình ngưỡng"));
    actions->addWidget(cancel);
    actions->addWidget(save);
    root->addLayout(actions);

    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(save, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() != QDialog::Accepted) return;

    QJsonObject config;
    for (const auto &item : inputs)
        config.insert(item.first, item.second->text().replace(',', '.').toDouble());

    const int intervalMs = int(config.value("sampling_interval_seconds").toDouble(2.0) * 1000.0);
    config.insert("sampling_interval_ms", intervalMs > 0 ? intervalMs : 2000);

    QJsonObject thresholds;
    if (type == "weather_pressure") {
        thresholds.insert("pressure_hpa", QJsonObject{
            {"min", config.value("pressure_min_hpa").toDouble(990.0)},
            {"max", config.value("pressure_max_hpa").toDouble(1030.0)}
        });
        thresholds.insert("temperature_c", QJsonObject{
            {"min", 0.0},
            {"max", 50.0},
            {"warning_above", config.value("temperature_warn_c").toDouble(40.0)}
        });
    }
    if (!thresholds.isEmpty()) {
        config.insert("thresholds", thresholds);
    }

    updateDeviceConfig(deviceId, config);
}

void MainWindow::openAddDeviceDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Thêm & Ghép Nối Trạm Mới"));
    dialog.setFixedSize(520, 380);
    dialog.setStyleSheet(R"QSS(
        QDialog { background: #091024; border: 2px solid #00f2fe; border-radius: 12px; }
        QLabel { color: #ecf2ff; font-weight: 700; font-size: 12px; }
        QLabel#dialogTitle { color: #00f2fe; font-size: 15px; font-weight: 900; }
        QLineEdit { background: #0e1a3b; color: #ffffff; border: 1.5px solid #253b75; border-radius: 8px; padding: 6px 12px; min-height: 26px; font-weight: 800; font-size: 12px; }
        QLineEdit:focus { border: 1.5px solid #00f2fe; background: #132452; }
        QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff7a18, stop:1 #ff3e00); color: #ffffff; border: none; border-radius: 8px; padding: 8px 18px; font-weight: 900; font-size: 12px; }
        QPushButton#cancel { background: #1e293b; color: #dce7ff; border: 1px solid #334155; }
    )QSS");

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(16, 12, 16, 12);
    root->setSpacing(8);

    root->addWidget(label("+ Thêm & Ghép Nối Trạm Đo Khí Quyển Mới", "dialogTitle"));

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(8);

    auto *idInput = new QLineEdit(&dialog);
    idInput->setPlaceholderText("Nhập mã trạm (Ví dụ: HA-190782 hoặc ESP32S3_01)");
    
    auto *nameInput = new QLineEdit(&dialog);
    nameInput->setText("Trạm Khí Quyển & Chuyển Động");
    nameInput->setPlaceholderText("Đặt tên trạm hiển thị");

    form->addRow("Mã trạm (Device ID):", idInput);
    form->addRow("Tên hiển thị:", nameInput);
    root->addLayout(form);

    auto *kb = new VirtualKeyboard(&dialog);
    kb->attachTo(idInput);
    kb->setFixedHeight(160);
    root->addWidget(kb);

    connect(qApp, &QApplication::focusChanged, &dialog, [kb, idInput, nameInput](QWidget *, QWidget *now) {
        if (now == idInput) {
            kb->attachTo(idInput);
        } else if (now == nameInput) {
            kb->attachTo(nameInput);
        }
    });

    auto *actions = new QHBoxLayout;
    auto *cancel = button("Hủy", "cancel");
    cancel->setFixedHeight(32);
    auto *save = button("+ Xác Nhận Ghép Nối Trạm");
    save->setFixedHeight(32);
    actions->addWidget(cancel);
    actions->addWidget(save);
    root->addLayout(actions);

    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(save, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(idInput, &QLineEdit::returnPressed, &dialog, &QDialog::accept);
    connect(nameInput, &QLineEdit::returnPressed, &dialog, &QDialog::accept);
    connect(kb, &VirtualKeyboard::enterPressed, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        const QString devId = idInput->text().trimmed();
        if (devId.isEmpty()) {
            showCustomMessageBox(this, QMessageBox::Warning, "Thiếu thông tin", "Vui lòng nhập Mã trạm (Device ID).");
            return;
        }
        QString customName = nameInput->text().trimmed();
        if (customName.isEmpty()) customName = devId;
        claimDevice(devId, customName);
    }
}

void MainWindow::openClaimDeviceDialog(const QJsonObject &device)
{
    const QString deviceId = device.value("device_id").toString();
    const QString type = device.value("device_type").toString();
    const QString defaultName = deviceTypeName(type);

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Ghép nối trạm cảm biến"));
    dialog.setFixedSize(500, 380);
    dialog.setStyleSheet(R"QSS(
        QDialog { background: #091024; border: 2px solid #00f2fe; border-radius: 12px; }
        QLabel { color: #ecf2ff; font-weight: 700; font-size: 12px; }
        QLabel#dialogTitle { color: #00f2fe; font-size: 15px; font-weight: 900; }
        QLabel#infoBadge { background: #101e44; color: #00f2fe; border: 1px solid #253b75; border-radius: 6px; padding: 4px 8px; font-weight: 800; font-size: 11px; }
        QLineEdit { background: #0e1a3b; color: #ffffff; border: 1.5px solid #253b75; border-radius: 6px; padding: 5px 10px; min-height: 24px; font-weight: 800; font-size: 12px; }
        QLineEdit:focus { border: 1.5px solid #00f2fe; background: #132452; }
        QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #3b82f6); color: #030818; border: none; border-radius: 6px; padding: 6px 14px; font-weight: 900; }
        QPushButton#cancel { background: #1e293b; color: #dce7ff; border: 1px solid #334155; }
    )QSS");

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(14, 10, 14, 10);
    root->setSpacing(6);

    root->addWidget(label("Ghép nối trạm đo khí quyển & chuyển động", "dialogTitle"));

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(6);

    auto *idLabel = label(deviceId, "infoBadge");
    auto *typeLabel = label(deviceTypeName(type) + " (" + type + ")", "infoBadge");
    auto *nameInput = new QLineEdit(&dialog);
    nameInput->setText(defaultName);
    nameInput->setPlaceholderText("Đặt tên trạm (ví dụ: Trạm Khí Quyển & Chuyển Động Tầng 1)");
    nameInput->selectAll();

    form->addRow("Mã trạm (ID):", idLabel);
    form->addRow("Loại trạm:", typeLabel);
    form->addRow("Tên hiển thị:", nameInput);
    root->addLayout(form);

    auto *kb = new VirtualKeyboard(&dialog);
    kb->attachTo(nameInput);
    kb->setFixedHeight(160);
    root->addWidget(kb);

    auto *actions = new QHBoxLayout;
    auto *cancel = button("Hủy", "cancel");
    auto *save = button("Xác nhận ghép nối");
    actions->addWidget(cancel);
    actions->addWidget(save);
    root->addLayout(actions);

    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(save, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(nameInput, &QLineEdit::returnPressed, &dialog, &QDialog::accept);
    connect(kb, &VirtualKeyboard::enterPressed, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        QString customName = nameInput->text().trimmed();
        if (customName.isEmpty()) customName = defaultName;
        claimDevice(deviceId, customName);
    }
}

void MainWindow::createUserDialog()
{
    QDialog d(this);
    d.setWindowTitle("Tạo tài khoản mới");
    d.setFixedSize(420, 260);
    d.setStyleSheet(R"QSS(
        QDialog { background: #091024; border: 2px solid #00f2fe; border-radius: 12px; }
        QLabel { color: #ecf2ff; font-weight: 700; font-size: 12px; }
        QLineEdit, QComboBox { background: #0e1a3b; color: #ffffff; border: 1.5px solid #253b75; border-radius: 6px; padding: 4px 8px; min-height: 22px; font-weight: 800; }
        QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #3b82f6); color: #030818; border: none; border-radius: 6px; padding: 6px 14px; font-weight: 900; }
        QPushButton#cancel { background: #1e293b; color: #dce7ff; border: 1px solid #334155; }
    )QSS");

    auto *root = new QVBoxLayout(&d);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(8);

    root->addWidget(label("Tạo tài khoản người dùng mới", "pageTitle"));

    auto *form = new QFormLayout;
    QLineEdit u, p;
    p.setEchoMode(QLineEdit::Password);
    QComboBox role;
    role.addItem("Người dùng (Viewer)", "viewer");
    role.addItem("Quản trị viên (Admin)", "admin");

    form->addRow("Tài khoản", &u);
    form->addRow("Mật khẩu", &p);
    form->addRow("Quyền hạn", &role);
    root->addLayout(form);

    auto *actions = new QHBoxLayout;
    auto *cancel = button("Hủy", "cancel");
    auto *ok = button("Tạo tài khoản");
    actions->addWidget(cancel);
    actions->addWidget(ok);
    root->addLayout(actions);

    connect(cancel, &QPushButton::clicked, &d, &QDialog::reject);
    connect(ok, &QPushButton::clicked, &d, &QDialog::accept);

    if (d.exec() == QDialog::Accepted) {
        post("/api/admin/users", {{"username", u.text().trimmed()}, {"password", p.text()}, {"role", role.currentData().toString()}},
             [this](QJsonObject) { refreshUsers(); });
    }
}

void MainWindow::editUserDialog(const QJsonObject &user)
{
    const QString oldUsername = user.value("username").toString();
    QDialog d(this);
    d.setWindowTitle("Chỉnh sửa tài khoản");
    d.setFixedSize(440, 310);
    d.setStyleSheet(R"QSS(
        QDialog { background: #091024; border: 2px solid #00f2fe; border-radius: 12px; }
        QLabel { color: #ecf2ff; font-weight: 700; font-size: 12px; }
        QLineEdit, QComboBox { background: #0e1a3b; color: #ffffff; border: 1.5px solid #253b75; border-radius: 6px; padding: 4px 8px; min-height: 22px; font-weight: 800; }
        QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #3b82f6); color: #030818; border: none; border-radius: 6px; padding: 6px 14px; font-weight: 900; }
        QPushButton#cancel { background: #1e293b; color: #dce7ff; border: 1px solid #334155; }
        QPushButton#delete { background: #e11d48; color: white; border: none; }
    )QSS");

    auto *root = new QVBoxLayout(&d);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(8);

    root->addWidget(label("Chỉnh sửa tài khoản: " + oldUsername, "pageTitle"));

    auto *form = new QFormLayout;
    QLineEdit username(oldUsername), password;
    password.setEchoMode(QLineEdit::Password);
    password.setPlaceholderText("Trống = giữ nguyên");
    QComboBox role;
    role.addItem("Người dùng (Viewer)", "viewer");
    role.addItem("Quản trị viên (Admin)", "admin");
    role.setCurrentIndex(user.value("role").toString() == "admin" ? 1 : 0);
    QCheckBox enabled("Đang hoạt động");
    enabled.setChecked(user.value("enabled").toBool(true));

    form->addRow("Tài khoản", &username);
    form->addRow("Mật khẩu mới", &password);
    form->addRow("Quyền hạn", &role);
    form->addRow("Trạng thái", &enabled);
    root->addLayout(form);

    auto *actions = new QHBoxLayout;
    auto *remove = button("Xóa tài khoản", "delete");
    auto *cancel = button("Hủy", "cancel");
    auto *save = button("Lưu thay đổi");
    actions->addWidget(remove);
    actions->addWidget(cancel);
    actions->addWidget(save);
    root->addLayout(actions);

    connect(cancel, &QPushButton::clicked, &d, &QDialog::reject);
    connect(save, &QPushButton::clicked, &d, &QDialog::accept);
    connect(remove, &QPushButton::clicked, &d, [&] {
        if (!showCustomQuestionBox(this, "Xác nhận", "Xóa người dùng " + oldUsername + "?"))
            return;
        del("/api/admin/users/" + QString::fromUtf8(QUrl::toPercentEncoding(oldUsername)), [this](QJsonObject) { refreshUsers(); });
        d.reject();
    });

    if (d.exec() == QDialog::Accepted) {
        QJsonObject payload{{"username", username.text().trimmed()}, {"password", password.text()}, {"role", role.currentData().toString()}, {"enabled", enabled.isChecked()}};
        auto *reply = m_net.put(request("/api/admin/users/" + QString::fromUtf8(QUrl::toPercentEncoding(oldUsername))), QJsonDocument(payload).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            if (reply->error() != QNetworkReply::NoError)
                showCustomMessageBox(this, QMessageBox::Warning, "Lỗi", reply->errorString());
            reply->deleteLater();
            refreshUsers();
        });
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11) {
        if (isFullScreen()) showNormal();
        else showFullScreen();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Escape && isFullScreen()) {
        showNormal();
        event->accept();
        return;
    }
    QMainWindow::keyPressEvent(event);
}
