#include "DashboardPage.h"
#include "ui_DashboardPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QtMath>

// ============================================================================
// 1. CircularGaugeWidget (Card 1)
// ============================================================================
CircularGaugeWidget::CircularGaugeWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(110, 110);
    setCursor(Qt::PointingHandCursor);
}

void CircularGaugeWidget::setValue(double val, double minVal, double maxVal, const QString &unit)
{
    m_value = val;
    m_min = minVal;
    m_max = maxVal;
    m_unit = unit;
    update();
}

void CircularGaugeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int side = qMin(width(), height()) - 16;
    const int x = (width() - side) / 2;
    const int y = (height() - side) / 2;
    const QRectF rect(x, y, side, side);

    // Track Background
    QPen trackPen(QColor(QStringLiteral("#1e1b4b")), 7, Qt::SolidLine, Qt::RoundCap);
    p.setPen(trackPen);
    p.drawEllipse(rect);

    // Active Glowing Ring
    double pct = (m_value > 0) ? qBound(0.0, (m_value - m_min) / (m_max - m_min), 1.0) : 0.0;
    int spanAngle = static_cast<int>(pct * 360 * 16);

    if (spanAngle > 0) {
        QLinearGradient grad(rect.topLeft(), rect.bottomRight());
        grad.setColorAt(0.0, QColor(QStringLiteral("#38bdf8")));
        grad.setColorAt(1.0, QColor(QStringLiteral("#818cf8")));
        QPen activePen(grad, 7, Qt::SolidLine, Qt::RoundCap);
        p.setPen(activePen);
        p.drawArc(rect, 90 * 16, -spanAngle);
    }

    // Text in center
    p.setPen(QColor(QStringLiteral("#ffffff")));
    QFont font = p.font();
    font.setPointSize(14);
    font.setBold(true);
    p.setFont(font);

    if (m_value > 0) {
        p.drawText(rect, Qt::AlignCenter, QStringLiteral("%1\n%2").arg(QString::number(m_value, 'f', 1), m_unit));
    } else {
        p.setPen(QColor(QStringLiteral("#94a3b8")));
        p.drawText(rect, Qt::AlignCenter, QStringLiteral("--\n%1").arg(m_unit));
    }
}

// ============================================================================
// 2. SemiCircleGaugeWidget (Card 6)
// ============================================================================
SemiCircleGaugeWidget::SemiCircleGaugeWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(130, 80);
    setCursor(Qt::PointingHandCursor);
}

void SemiCircleGaugeWidget::setValue(double val, double maxVal, const QString &unit)
{
    m_value = val;
    m_max = maxVal;
    m_unit = unit;
    update();
}

void SemiCircleGaugeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int side = qMin(width(), height() * 2) - 16;
    const int x = (width() - side) / 2;
    const int y = 6;
    const QRectF rect(x, y, side, side);

    // Track Arc
    QPen trackPen(QColor(QStringLiteral("#1e1b4b")), 8, Qt::SolidLine, Qt::RoundCap);
    p.setPen(trackPen);
    p.drawArc(rect, 0 * 16, 180 * 16);

    // Value Arc
    double pct = (m_value > 0) ? qBound(0.0, m_value / m_max, 1.0) : 0.0;
    int spanAngle = static_cast<int>(pct * 180 * 16);

    if (spanAngle > 0) {
        QLinearGradient grad(rect.bottomLeft(), rect.topRight());
        grad.setColorAt(0.0, QColor(QStringLiteral("#10b981")));
        grad.setColorAt(1.0, QColor(QStringLiteral("#6366f1")));
        QPen activePen(grad, 8, Qt::SolidLine, Qt::RoundCap);
        p.setPen(activePen);
        p.drawArc(rect, 180 * 16, -spanAngle);
    }

    // Center Value
    p.setPen(QColor(QStringLiteral("#ffffff")));
    QFont font = p.font();
    font.setPointSize(18);
    font.setBold(true);
    p.setFont(font);

    if (m_value > 0) {
        p.drawText(QRectF(0, y + side / 4, width(), side / 2), Qt::AlignCenter, QString::number(static_cast<int>(m_value)));
    } else {
        p.setPen(QColor(QStringLiteral("#94a3b8")));
        p.drawText(QRectF(0, y + side / 4, width(), side / 2), Qt::AlignCenter, QStringLiteral("--"));
    }
}

// ============================================================================
// 3. NeonAreaChartWidget (Card 3)
// ============================================================================
NeonAreaChartWidget::NeonAreaChartWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(220, 110);
    setCursor(Qt::PointingHandCursor);
}

void NeonAreaChartWidget::addPoint(double val1, double val2)
{
    m_data1.append(val1);
    m_data2.append(val2);
    if (m_data1.size() > 24) m_data1.removeFirst();
    if (m_data2.size() > 24) m_data2.removeFirst();
    update();
}

void NeonAreaChartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();
    const int padBottom = 16;
    const int padLeft = 24;
    const int plotW = w - padLeft - 10;
    const int plotH = h - padBottom - 10;

    // Grid lines
    p.setPen(QPen(QColor(QStringLiteral("#282352")), 1, Qt::SolidLine));
    for (int i = 0; i <= 4; ++i) {
        int y = 10 + (plotH * i) / 4;
        p.drawLine(padLeft, y, w - 10, y);
        p.setPen(QColor(QStringLiteral("#64748b")));
        QFont f = p.font();
        f.setPointSize(7);
        p.setFont(f);
        p.drawText(QRect(0, y - 6, padLeft - 4, 12), Qt::AlignRight | Qt::AlignVCenter, QString::number(100 - i * 25));
        p.setPen(QPen(QColor(QStringLiteral("#282352")), 1, Qt::SolidLine));
    }

    if (m_data1.isEmpty()) {
        p.setPen(QColor(QStringLiteral("#64748b")));
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("Đang chờ kết nối cảm biến ESP32..."));
        return;
    }

    if (m_data1.size() < 2) return;

    // 1. Draw Area Fill for Data 1
    QPainterPath areaPath;
    const double stepX = static_cast<double>(plotW) / (m_data1.size() - 1);

    areaPath.moveTo(padLeft, h - padBottom);
    for (int i = 0; i < m_data1.size(); ++i) {
        double px = padLeft + i * stepX;
        double py = (h - padBottom) - (qBound(0.0, m_data1[i], 100.0) / 100.0) * plotH;
        areaPath.lineTo(px, py);
    }
    areaPath.lineTo(padLeft + (m_data1.size() - 1) * stepX, h - padBottom);
    areaPath.closeSubpath();

    QLinearGradient areaGrad(0, 0, 0, h);
    areaGrad.setColorAt(0.0, QColor(QStringLiteral("#6366f1")));
    areaGrad.setColorAt(1.0, QColor(QStringLiteral("#1b173d")));
    p.fillPath(areaPath, areaGrad);

    // 2. Draw Wave Line
    QPainterPath wavePath;
    int peakIdx = 0;
    double maxVal = 0;

    for (int i = 0; i < m_data1.size(); ++i) {
        double px = padLeft + i * stepX;
        double py = (h - padBottom) - (qBound(0.0, m_data1[i], 100.0) / 100.0) * plotH;
        if (i == 0) wavePath.moveTo(px, py);
        else wavePath.lineTo(px, py);

        if (m_data1[i] > maxVal) {
            maxVal = m_data1[i];
            peakIdx = i;
        }
    }
    p.setPen(QPen(QColor(QStringLiteral("#10b981")), 2.2, Qt::SolidLine, Qt::RoundCap));
    p.drawPath(wavePath);

    // 3. Peak Badges
    double peakX = padLeft + peakIdx * stepX;
    double peakY = (h - padBottom) - (qBound(0.0, maxVal, 100.0) / 100.0) * plotH;

    p.setBrush(QColor(QStringLiteral("#6366f1")));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(QRectF(peakX - 10, peakY - 24, 20, 12), 3, 3);
    p.setPen(QColor(QStringLiteral("#ffffff")));
    QFont f = p.font();
    f.setPointSize(7);
    f.setBold(true);
    p.setFont(f);
    p.drawText(QRectF(peakX - 10, peakY - 24, 20, 12), Qt::AlignCenter, QString::number(static_cast<int>(maxVal)));

    p.setBrush(QColor(QStringLiteral("#10b981")));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(QRectF(peakX - 10, peakY - 10, 20, 12), 3, 3);
    p.setPen(QColor(QStringLiteral("#ffffff")));
    p.drawText(QRectF(peakX - 10, peakY - 10, 20, 12), Qt::AlignCenter, QString::number(static_cast<int>(maxVal * 0.7)));
}

// ============================================================================
// 4. DashboardPage Implementation
// ============================================================================
DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::DashboardPage)
{
    ui->setupUi(this);
    setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1a1638, stop:1 #120e2e); color: #ecf2ff; font-family: sans-serif;");

    setupCustomDashboard();
    updateDeviceCardState();
    updateSensorStatusBadges();
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

void DashboardPage::setUsername(const QString &username)
{
    m_username = username;
}

void DashboardPage::setDeviceId(const QString &deviceId)
{
    m_deviceId = deviceId;
    m_hasDevice = !deviceId.isEmpty();
    updateDeviceCardState();
    updateSensorStatusBadges();
}

void DashboardPage::setAvailableDevices(const QJsonArray &devices)
{
    m_availableDevices = devices;
    if (m_currentSelectDialog) {
        m_currentSelectDialog->updateAvailableDevices(devices);
    }
}

void DashboardPage::setOwnedDevices(const QJsonArray &devices)
{
    if (!devices.isEmpty()) {
        const auto first = devices.first().toObject();
        m_deviceId = first.value(QStringLiteral("device_id")).toString();
        m_deviceName = first.value(QStringLiteral("name")).toString(m_deviceId);
        m_isOnline = first.value(QStringLiteral("is_online")).toBool(true);
        m_hasDevice = true;

        if (first.contains(QStringLiteral("metrics"))) {
            updateDeviceMetrics(first.value(QStringLiteral("metrics")).toObject());
        }
    } else {
        m_hasDevice = false;
        m_deviceId = "";
        m_curVoltage = 0.0;
        m_curCurrent = 0.0;
        m_curPower = 0.0;
    }
    updateDeviceCardState();
    updateSensorStatusBadges();
}

void DashboardPage::updateReading(const SensorReading &reading)
{
    if (reading.pressureHpa > 0) m_curVoltage = reading.pressureHpa;
    if (reading.distanceCm > 0) m_curCurrent = reading.distanceCm;
    m_curPower = m_curVoltage * m_curCurrent;

    const QDateTime now = QDateTime::currentDateTime();
    m_voltageHistory.append({now, m_curVoltage});
    m_currentHistory.append({now, m_curCurrent});
    m_powerHistory.append({now, m_curPower});

    if (m_voltageHistory.size() > 100) m_voltageHistory.removeFirst();
    if (m_currentHistory.size() > 100) m_currentHistory.removeFirst();
    if (m_powerHistory.size() > 100) m_powerHistory.removeFirst();

    if (m_circularGauge) m_circularGauge->setValue(m_curVoltage, 0, 300, QStringLiteral("V"));
    if (m_semiCircleGauge) m_semiCircleGauge->setValue(m_curPower, 2000, QStringLiteral("W"));
    if (m_areaChart) {
        m_areaChart->addPoint(qBound(0.0, (m_curPower / 2000.0) * 100.0, 100.0),
                              qBound(0.0, (m_curCurrent / 10.0) * 100.0, 100.0));
    }
    updateSensorStatusBadges();
}

void DashboardPage::updateDeviceMetrics(const QJsonObject &metrics)
{
    if (metrics.contains(QStringLiteral("voltage_v"))) {
        m_curVoltage = metrics.value(QStringLiteral("voltage_v")).toDouble();
    } else if (metrics.contains(QStringLiteral("pressure_hpa"))) {
        m_curVoltage = metrics.value(QStringLiteral("pressure_hpa")).toDouble();
    }

    if (metrics.contains(QStringLiteral("current_a"))) {
        m_curCurrent = metrics.value(QStringLiteral("current_a")).toDouble();
    } else if (metrics.contains(QStringLiteral("distance_cm"))) {
        m_curCurrent = metrics.value(QStringLiteral("distance_cm")).toDouble();
    }

    if (metrics.contains(QStringLiteral("power_w"))) {
        m_curPower = metrics.value(QStringLiteral("power_w")).toDouble();
    } else {
        m_curPower = m_curVoltage * m_curCurrent;
    }

    if (metrics.contains(QStringLiteral("relay_on"))) {
        m_relayState = metrics.value(QStringLiteral("relay_on")).toBool();
    }

    const QDateTime now = QDateTime::currentDateTime();
    if (m_curVoltage > 0) m_voltageHistory.append({now, m_curVoltage});
    if (m_curCurrent > 0) m_currentHistory.append({now, m_curCurrent});
    if (m_curPower > 0) m_powerHistory.append({now, m_curPower});

    if (m_voltageHistory.size() > 100) m_voltageHistory.removeFirst();
    if (m_currentHistory.size() > 100) m_currentHistory.removeFirst();
    if (m_powerHistory.size() > 100) m_powerHistory.removeFirst();

    if (m_circularGauge) m_circularGauge->setValue(m_curVoltage, 0, 300, QStringLiteral("V"));
    if (m_semiCircleGauge) m_semiCircleGauge->setValue(m_curPower, 2000, QStringLiteral("W"));
    if (m_areaChart) {
        m_areaChart->addPoint(qBound(0.0, (m_curPower / 2000.0) * 100.0, 100.0),
                              qBound(0.0, (m_curCurrent / 10.0) * 100.0, 100.0));
    }

    if (m_voltageValLbl) m_voltageValLbl->setText(m_curVoltage > 0 ? QStringLiteral("%1 V").arg(QString::number(m_curVoltage, 'f', 1)) : QStringLiteral("-- V"));
    if (m_currentValLbl) m_currentValLbl->setText(m_curCurrent > 0 ? QStringLiteral("%1 A").arg(QString::number(m_curCurrent, 'f', 2)) : QStringLiteral("-- A"));
    if (m_powerBigLbl) m_powerBigLbl->setText(m_curPower > 0 ? QString::number(static_cast<int>(m_curPower)) : QStringLiteral("---"));

    if (m_devVoltageLbl) m_devVoltageLbl->setText(QStringLiteral("⚡ Điện áp: <b>%1 V</b>").arg(QString::number(m_curVoltage, 'f', 1)));
    if (m_devCurrentLbl) m_devCurrentLbl->setText(QStringLiteral("💡 Dòng tải: <b>%1 A</b>").arg(QString::number(m_curCurrent, 'f', 2)));
    if (m_relayBtn) {
        m_relayBtn->setText(m_relayState ? QStringLiteral("🔌 RƠ-LE: BẬT") : QStringLiteral("🔌 RƠ-LE: TẮT"));
        m_relayBtn->setStyleSheet(m_relayState
            ? "QPushButton { background: #10b981; color: #ffffff; border: none; border-radius: 6px; font-weight: 900; font-size: 11px; padding: 6px 12px; } QPushButton:hover { background: #059669; }"
            : "QPushButton { background: #374151; color: #9ca3af; border: 1px solid #4b5563; border-radius: 6px; font-weight: 800; font-size: 11px; padding: 6px 12px; } QPushButton:hover { background: #4b5563; }");
    }

    updateSensorStatusBadges();
}

void DashboardPage::updateSensorStatusBadges()
{
    if (!m_voltageBadge || !m_voltageSubLbl) return;

    if (!m_hasDevice || m_deviceId.isEmpty() || m_curVoltage <= 0.0) {
        m_voltageBadge->setText(QStringLiteral("🔴 CHƯA CÓ THIẾT BỊ"));
        m_voltageBadge->setStyleSheet("background: #451a24; color: #f87171; border: 1px solid #7f1d1d; font-weight: 900; font-size: 8px; border-radius: 4px; padding: 2px 6px;");
        m_voltageSubLbl->setText(QStringLiteral("Chưa nhận tín hiệu ESP32\nNhấn xem Bảng & Biểu đồ"));
    } else {
        m_voltageBadge->setText(QStringLiteral("🟢 ỔN ĐỊNH"));
        m_voltageBadge->setStyleSheet("background: #10b981; color: #ffffff; font-weight: 900; font-size: 9px; border-radius: 4px; padding: 2px 8px;");
        m_voltageSubLbl->setText(QStringLiteral("Điện áp RMS lưới điện\nNhấn xem Bảng & Biểu đồ"));
    }
}

void DashboardPage::openVoltageDetail()
{
    SensorDetailDialog dlg(QStringLiteral("Điện Áp AC (ZMPT101B)"),
                           QStringLiteral("V"),
                           QStringLiteral("#38bdf8"),
                           m_voltageHistory,
                           this,
                           180.0, 240.0);
    dlg.exec();
}

void DashboardPage::openCurrentDetail()
{
    SensorDetailDialog dlg(QStringLiteral("Dòng Điện Tải (ACS712)"),
                           QStringLiteral("A"),
                           QStringLiteral("#10b981"),
                           m_currentHistory,
                           this,
                           0.1, 15.0);
    dlg.exec();
}

void DashboardPage::openPowerDetail()
{
    SensorDetailDialog dlg(QStringLiteral("Công Suất Tiêu Thụ Tức Thời"),
                           QStringLiteral("W"),
                           QStringLiteral("#818cf8"),
                           m_powerHistory,
                           this,
                           0.0, 2500.0);
    dlg.exec();
}

void DashboardPage::openAddDeviceDialog()
{
    emit refreshDevicesRequested();
    auto *dlg = new SelectOnlineDeviceDialog(m_availableDevices, this);
    m_currentSelectDialog = dlg;

    connect(dlg, &SelectOnlineDeviceDialog::refreshRequested, this, &DashboardPage::refreshDevicesRequested);
    connect(dlg, &SelectOnlineDeviceDialog::deviceSelected, this, [this](const QString &devId, const QString &devName) {
        emit claimDeviceRequested(devId, devName);
    });
    dlg->exec();
    delete dlg;
}

void DashboardPage::updateDeviceCardState()
{
    if (!m_deviceCardStack) return;

    if (!m_hasDevice || m_deviceId.isEmpty()) {
        m_deviceCardStack->setCurrentIndex(0);
    } else {
        m_deviceCardStack->setCurrentIndex(1);
        if (m_devIdLbl) m_devIdLbl->setText(QStringLiteral("🖲 Thiết Bị: %1").arg(m_deviceId));
        if (m_devOnlineBadge) {
            m_devOnlineBadge->setText(m_isOnline ? QStringLiteral("🟢 ONLINE") : QStringLiteral("🔴 OFFLINE"));
            m_devOnlineBadge->setStyleSheet(m_isOnline
                ? "color: #10b981; font-size: 8px; font-weight: 900; background: rgba(16, 185, 129, 0.15); border-radius: 3px; padding: 1px 5px;"
                : "color: #ef4444; font-size: 8px; font-weight: 900; background: rgba(239, 68, 68, 0.15); border-radius: 3px; padding: 1px 5px;");
        }
    }
}

void DashboardPage::setupCustomDashboard()
{
    while (QLayoutItem *item = ui->verticalLayout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    auto *mainLayout = ui->verticalLayout;
    mainLayout->setContentsMargins(10, 8, 10, 8);
    mainLayout->setSpacing(6);

    auto makeCard = [](const QString &title = QString()) {
        auto *c = new QFrame;
        c->setStyleSheet(
            "QFrame { "
            "  background-color: rgba(30, 26, 68, 0.85); "
            "  border: 1px solid #312966; "
            "  border-radius: 8px; "
            "} "
            "QFrame:hover { "
            "  border-color: #4c3f99; "
            "}"
        );
        c->setCursor(Qt::PointingHandCursor);
        auto *l = new QVBoxLayout(c);
        l->setContentsMargins(8, 6, 8, 6);
        l->setSpacing(4);

        if (!title.isEmpty()) {
            auto *tRow = new QHBoxLayout;
            auto *tLbl = new QLabel(title);
            tLbl->setStyleSheet("color: #cbd5e1; font-size: 10px; font-weight: 700; background: transparent;");
            auto *xLbl = new QLabel(QStringLiteral("🔍"));
            xLbl->setStyleSheet("color: #64748b; font-size: 9px; background: transparent;");
            tRow->addWidget(tLbl);
            tRow->addStretch();
            tRow->addWidget(xLbl);
            l->addLayout(tRow);
        }
        return qMakePair(c, l);
    };

    struct CardClickFilter : public QObject {
        std::function<void()> onClick;
        CardClickFilter(QObject *parent, std::function<void()> cb) : QObject(parent), onClick(cb) {}
        bool eventFilter(QObject *watched, QEvent *event) override {
            if (event->type() == QEvent::MouseButtonRelease) {
                if (onClick) onClick();
                return true;
            }
            return QObject::eventFilter(watched, event);
        }
    };

    // ==========================================
    // MAIN 2-ROW GRID
    // ==========================================
    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(6);
    grid->setVerticalSpacing(6);

    // --- CARD 1 (Top Left): Voltage Circle Gauge ---
    auto c1 = makeCard("Điện Áp ZMPT101B (Xem Chi Tiết)");
    m_circularGauge = new CircularGaugeWidget;
    m_circularGauge->setValue(0.0, 0, 300, QStringLiteral("V"));
    c1.second->addWidget(m_circularGauge, 1, Qt::AlignCenter);

    m_voltageSubLbl = new QLabel(QStringLiteral("Chưa nhận tín hiệu ESP32\nNhấn xem Bảng & Biểu đồ"));
    m_voltageSubLbl->setAlignment(Qt::AlignCenter);
    m_voltageSubLbl->setStyleSheet("color: #94a3b8; font-size: 8px; background: transparent;");
    c1.second->addWidget(m_voltageSubLbl);

    m_voltageBadge = new QLabel(QStringLiteral("🔴 CHƯA CÓ THIẾT BỊ"));
    m_voltageBadge->setAlignment(Qt::AlignCenter);
    m_voltageBadge->setStyleSheet("background: #451a24; color: #f87171; border: 1px solid #7f1d1d; font-weight: 900; font-size: 8px; border-radius: 4px; padding: 2px 6px;");
    c1.second->addWidget(m_voltageBadge, 0, Qt::AlignCenter);

    c1.first->installEventFilter(new CardClickFilter(c1.first, [this] { openVoltageDetail(); }));
    grid->addWidget(c1.first, 0, 0, 2, 1);

    // --- CARD 2 (Top Middle): Cras iaculis + Wave Chart ---
    auto c2 = makeCard("Dòng Điện Tải ACS712 & Sóng Neon (Xem Chi Tiết)");
    auto *statRow = new QHBoxLayout;
    auto *iconGroup = new QLabel(QStringLiteral("⚡"));
    iconGroup->setStyleSheet("font-size: 18px; color: #10b981; background: transparent;");
    m_powerBigLbl = new QLabel(QStringLiteral("---"));
    m_powerBigLbl->setStyleSheet("color: #ffffff; font-size: 22px; font-weight: 900; background: transparent;");
    statRow->addWidget(iconGroup);
    statRow->addWidget(m_powerBigLbl);
    statRow->addStretch();

    auto *pillsCol1 = new QVBoxLayout;
    pillsCol1->setSpacing(1);
    auto *p1 = new QLabel(QStringLiteral("👤 ACS712   ↗ Live"));
    p1->setStyleSheet("color: #10b981; font-size: 9px; font-weight: 700; background: transparent;");
    auto *p2 = new QLabel(QStringLiteral("🏠 ESP32    📡 MQTT"));
    p2->setStyleSheet("color: #818cf8; font-size: 9px; font-weight: 700; background: transparent;");
    pillsCol1->addWidget(p1);
    pillsCol1->addWidget(p2);
    statRow->addLayout(pillsCol1);
    c2.second->addLayout(statRow);

    m_areaChart = new NeonAreaChartWidget;
    c2.second->addWidget(m_areaChart, 1);
    c2.first->installEventFilter(new CardClickFilter(c2.first, [this] { openCurrentDetail(); }));
    grid->addWidget(c2.first, 0, 1, 2, 2);

    // --- CARD 4 (Top Right): 4 Progress Bars ---
    auto c4 = makeCard("Chỉ số tải & Phân tích");
    auto addProgressItem = [&](const QString &label, int val, const QString &color) {
        auto *row = new QHBoxLayout;
        auto *lbl = new QLabel(label);
        lbl->setStyleSheet("color: #cbd5e1; font-size: 9px; font-weight: 700; background: transparent;");
        auto *valLbl = new QLabel(QStringLiteral("%1%").arg(val));
        valLbl->setStyleSheet("color: #ffffff; font-size: 9px; font-weight: 900; background: transparent;");
        row->addWidget(lbl);
        row->addStretch();
        row->addWidget(valLbl);
        c4.second->addLayout(row);

        auto *bar = new QProgressBar;
        bar->setRange(0, 100);
        bar->setValue(val);
        bar->setTextVisible(false);
        bar->setFixedHeight(8);
        bar->setStyleSheet(QStringLiteral(
            "QProgressBar { background: #1a1638; border: none; border-radius: 4px; } "
            "QProgressBar::chunk { background: %1; border-radius: 4px; }").arg(color));
        c4.second->addWidget(bar);
    };

    addProgressItem("Điện áp AC", 0, "#10b981");
    addProgressItem("Dòng điện tải", 0, "#38bdf8");
    addProgressItem("Công suất tải", 0, "#818cf8");
    addProgressItem("Hệ số an toàn", 100, "#10b981");
    grid->addWidget(c4.first, 0, 3, 2, 1);

    // --- CARD 5 (Bottom Left): Activity Log / Timeline ---
    auto c5 = makeCard("Nhật ký vận hành");
    auto addTimelineItem = [&](const QString &icon, const QString &text, const QString &time, bool checked = false) {
        auto *row = new QHBoxLayout;
        auto *chk = new QLabel(checked ? QStringLiteral("✅") : QStringLiteral("🔘"));
        chk->setStyleSheet("font-size: 10px; background: transparent;");
        auto *tCol = new QVBoxLayout;
        tCol->setSpacing(0);
        auto *t1 = new QLabel(text);
        t1->setStyleSheet("color: #ffffff; font-size: 9px; font-weight: 700; background: transparent;");
        auto *t2 = new QLabel(QStringLiteral("📍 GC   🕒 %1").arg(time));
        t2->setStyleSheet("color: #94a3b8; font-size: 8px; background: transparent;");
        tCol->addWidget(t1);
        tCol->addWidget(t2);
        row->addWidget(chk);
        row->addLayout(tCol);
        row->addStretch();
        c5.second->addLayout(row);
    };

    addTimelineItem("🔘", "Đóng Rơ-le Tải chính", "Chưa bật", false);
    addTimelineItem("🔘", "Điện áp định mức AC", "Chờ ESP32", false);
    addTimelineItem("✅", "Khởi động giao diện", "Vừa xong", true);
    grid->addWidget(c5.first, 2, 0, 1, 1);

    // --- CARD 6 (Bottom Middle): Instant Power Speedometer ---
    auto c6 = makeCard("Công Suất Tức Thời W (Xem Chi Tiết)");
    m_semiCircleGauge = new SemiCircleGaugeWidget;
    m_semiCircleGauge->setValue(0, 2000, QStringLiteral("W"));
    c6.second->addWidget(m_semiCircleGauge, 1, Qt::AlignCenter);

    auto *legRow = new QHBoxLayout;
    auto *l1 = new QLabel(QStringLiteral("🟦 Điện Áp"));
    l1->setStyleSheet("color: #818cf8; font-size: 8px; font-weight: 700; background: transparent;");
    auto *l2 = new QLabel(QStringLiteral("🟩 Dòng Điện"));
    l2->setStyleSheet("color: #10b981; font-size: 8px; font-weight: 700; background: transparent;");
    legRow->addWidget(l1);
    legRow->addWidget(l2);
    legRow->addStretch();
    c6.second->addLayout(legRow);

    c6.first->installEventFilter(new CardClickFilter(c6.first, [this] { openPowerDetail(); }));
    grid->addWidget(c6.first, 2, 1, 1, 1);

    // --- CARD 7 (Bottom Right): Add Device or Device Control ---
    auto *c7Frame = new QFrame;
    c7Frame->setStyleSheet(
        "QFrame#c7Frame { "
        "  background-color: rgba(30, 26, 68, 0.85); "
        "  border: 1px solid #312966; "
        "  border-radius: 8px; "
        "} "
        "QFrame#c7Frame:hover { "
        "  border-color: #10b981; "
        "}"
    );
    c7Frame->setObjectName(QStringLiteral("c7Frame"));
    auto *c7Layout = new QVBoxLayout(c7Frame);
    c7Layout->setContentsMargins(6, 6, 6, 6);

    m_deviceCardStack = new QStackedWidget;

    // View 0: No Device -> Big + Button
    m_noDeviceWidget = new QWidget;
    auto *noDevLayout = new QVBoxLayout(m_noDeviceWidget);
    noDevLayout->setContentsMargins(8, 4, 8, 4);
    noDevLayout->setSpacing(4);

    auto *addBtn = new QPushButton(QStringLiteral("＋"));
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setFixedSize(54, 54);
    addBtn->setStyleSheet(
        "QPushButton { "
        "  background: #15103a; "
        "  color: #10b981; "
        "  border: 2px dashed #10b981; "
        "  border-radius: 27px; "
        "  font-size: 26px; "
        "  font-weight: 900; "
        "} "
        "QPushButton:hover { "
        "  background: #10b981; "
        "  color: #ffffff; "
        "  border: 2px solid #34d399; "
        "}"
    );
    connect(addBtn, &QPushButton::clicked, this, &DashboardPage::openAddDeviceDialog);

    auto *noDevTitle = new QLabel(QStringLiteral("Chưa có thiết bị"));
    noDevTitle->setStyleSheet("color: #ffffff; font-size: 11px; font-weight: 800;");
    noDevTitle->setAlignment(Qt::AlignCenter);

    auto *noDevSub = new QLabel(QStringLiteral("Nhấn <b>＋</b> để quét và thêm thiết bị ESP32"));
    noDevSub->setStyleSheet("color: #94a3b8; font-size: 9px;");
    noDevSub->setAlignment(Qt::AlignCenter);

    noDevLayout->addStretch();
    noDevLayout->addWidget(addBtn, 0, Qt::AlignCenter);
    noDevLayout->addWidget(noDevTitle);
    noDevLayout->addWidget(noDevSub);
    noDevLayout->addStretch();

    m_deviceCardStack->addWidget(m_noDeviceWidget);

    // View 1: Has Device -> Device Control Card
    m_hasDeviceWidget = new QWidget;
    auto *hasDevLayout = new QVBoxLayout(m_hasDeviceWidget);
    hasDevLayout->setContentsMargins(6, 4, 6, 4);
    hasDevLayout->setSpacing(4);

    auto *devHeader = new QHBoxLayout;
    m_devIdLbl = new QLabel(QStringLiteral("🖲 Thiết Bị ESP32"));
    m_devIdLbl->setStyleSheet("color: #38bdf8; font-size: 11px; font-weight: 800;");
    m_devOnlineBadge = new QLabel(QStringLiteral("🟢 ONLINE"));
    devHeader->addWidget(m_devIdLbl);
    devHeader->addStretch();
    devHeader->addWidget(m_devOnlineBadge);
    hasDevLayout->addLayout(devHeader);

    m_devVoltageLbl = new QLabel(QStringLiteral("⚡ Điện áp: <b>0.0 V</b>"));
    m_devVoltageLbl->setStyleSheet("color: #cbd5e1; font-size: 10px;");
    m_devCurrentLbl = new QLabel(QStringLiteral("💡 Dòng tải: <b>0.00 A</b>"));
    m_devCurrentLbl->setStyleSheet("color: #cbd5e1; font-size: 10px;");

    hasDevLayout->addWidget(m_devVoltageLbl);
    hasDevLayout->addWidget(m_devCurrentLbl);

    m_relayBtn = new QPushButton(QStringLiteral("🔌 RƠ-LE: TẮT"));
    m_relayBtn->setCursor(Qt::PointingHandCursor);
    m_relayBtn->setStyleSheet("QPushButton { background: #374151; color: #9ca3af; border: 1px solid #4b5563; border-radius: 6px; font-weight: 800; font-size: 10px; padding: 5px; }");
    connect(m_relayBtn, &QPushButton::clicked, this, [this] {
        if (!m_deviceId.isEmpty()) {
            emit relayControlRequested(m_deviceId, !m_relayState);
        }
    });
    hasDevLayout->addWidget(m_relayBtn);

    auto *actionsRow = new QHBoxLayout;
    auto *unbindBtn = new QPushButton(QStringLiteral("✕ Gỡ bỏ"));
    unbindBtn->setCursor(Qt::PointingHandCursor);
    unbindBtn->setStyleSheet("QPushButton { background: transparent; color: #ef4444; border: none; font-size: 9px; font-weight: 700; } QPushButton:hover { color: #f87171; text-decoration: underline; }");
    connect(unbindBtn, &QPushButton::clicked, this, [this] {
        if (!m_deviceId.isEmpty()) {
            emit releaseDeviceRequested(m_deviceId);
        }
    });
    actionsRow->addStretch();
    actionsRow->addWidget(unbindBtn);
    hasDevLayout->addLayout(actionsRow);

    m_deviceCardStack->addWidget(m_hasDeviceWidget);

    c7Layout->addWidget(m_deviceCardStack);
    grid->addWidget(c7Frame, 2, 2, 1, 2);

    mainLayout->addLayout(grid, 1);
}
