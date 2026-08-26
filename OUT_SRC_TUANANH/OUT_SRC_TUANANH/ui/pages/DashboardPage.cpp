#include "DashboardPage.h"
#include "ui_DashboardPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPushButton>
#include <QSettings>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QtMath>

// ============================================================================
// 1. VerticalLuxBarWidget (Segmented LED Power Tube)
// ============================================================================
VerticalLuxBarWidget::VerticalLuxBarWidget(QWidget *parent) : QWidget(parent)
{
    setFixedWidth(24);
    setMinimumHeight(140);
}

void VerticalLuxBarWidget::setValue(double val, double maxVal)
{
    m_value = val;
    m_max = maxVal;
    update();
}

void VerticalLuxBarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int totalSegments = 16;
    const double pct = (m_value > 0) ? qBound(0.0, m_value / m_max, 1.0) : 0.0;
    const int activeSegments = static_cast<int>(pct * totalSegments);
    const int segH = (height() - 4) / totalSegments;

    // Draw bezel container
    p.setPen(QPen(QColor(QStringLiteral("#1e293b")), 1));
    p.setBrush(QColor(QStringLiteral("#070b10")));
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 4, 4);

    for (int i = 0; i < totalSegments; ++i) {
        int segIdxFromBottom = totalSegments - 1 - i;
        int y = 2 + i * segH;
        QRect segRect(3, y + 1, width() - 6, segH - 2);

        if (segIdxFromBottom < activeSegments) {
            // Gradient from Cyber Emerald to Cyan to Amber Warning at top
            QColor c;
            if (segIdxFromBottom > 12) c = QColor(QStringLiteral("#f59e0b")); // high
            else if (segIdxFromBottom > 6) c = QColor(QStringLiteral("#00f0ff")); // mid
            else c = QColor(QStringLiteral("#10b981")); // low

            p.setBrush(c);
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(segRect, 1, 1);
        } else {
            p.setBrush(QColor(QStringLiteral("#111827")));
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(segRect, 1, 1);
        }
    }
}

// ============================================================================
// 2. TacticalRadarWidget (360 Sci-Fi Radar Scope)
// ============================================================================
TacticalRadarWidget::TacticalRadarWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(150, 150);
}

void TacticalRadarWidget::setDetected(bool detected)
{
    m_detected = detected;
    update();
}

void TacticalRadarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int side = qMin(width(), height()) - 8;
    const int cx = width() / 2;
    const int cy = height() / 2;
    const QRectF scopeRect(cx - side/2, cy - side/2, side, side);

    // Background dark scope
    QRadialGradient scopeGrad(cx, cy, side/2);
    scopeGrad.setColorAt(0.0, QColor(QStringLiteral("#0b171c")));
    scopeGrad.setColorAt(1.0, QColor(QStringLiteral("#04080a")));
    p.setBrush(scopeGrad);
    p.setPen(QPen(QColor(QStringLiteral("#164e63")), 1.5));
    p.drawEllipse(scopeRect);

    // Concentric grid rings
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(QStringLiteral("#0e7490")), 1, Qt::DotLine));
    p.drawEllipse(QPointF(cx, cy), side * 0.35, side * 0.35);
    p.drawEllipse(QPointF(cx, cy), side * 0.20, side * 0.20);

    // Tactical crosshairs & angles
    p.setPen(QPen(QColor(QStringLiteral("#155e75")), 1, Qt::SolidLine));
    p.drawLine(cx - side/2, cy, cx + side/2, cy);
    p.drawLine(cx, cy - side/2, cx, cy + side/2);

    // Compass markings
    p.setPen(QColor(QStringLiteral("#06b6d4")));
    QFont f = p.font();
    f.setPointSize(6);
    f.setBold(true);
    p.setFont(f);
    p.drawText(cx - 10, cy - side/2 + 10, 20, 10, Qt::AlignCenter, QStringLiteral("N"));
    p.drawText(cx - 10, cy + side/2 - 14, 20, 10, Qt::AlignCenter, QStringLiteral("S"));
    p.drawText(cx + side/2 - 14, cy - 5, 10, 10, Qt::AlignCenter, QStringLiteral("E"));
    p.drawText(cx - side/2 + 4, cy - 5, 10, 10, Qt::AlignCenter, QStringLiteral("W"));

    if (m_detected) {
        // Target Locked Pulse
        p.setBrush(QColor(16, 185, 129, 60));
        p.setPen(QPen(QColor(QStringLiteral("#10b981")), 2));
        p.drawEllipse(QPointF(cx + 20, cy - 15), 18, 18);

        p.setBrush(QColor(QStringLiteral("#10b981")));
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(cx + 20, cy - 15), 5, 5);

        // Target Label
        p.setPen(QColor(QStringLiteral("#10b981")));
        QFont tf = p.font();
        tf.setPointSize(7);
        tf.setBold(true);
        p.setFont(tf);
        p.drawText(cx + 28, cy - 20, QStringLiteral("TARGET [PIR]"));
    } else {
        // Standby sweep line
        p.setPen(QPen(QColor(6, 182, 212, 100), 1.5));
        p.drawLine(cx, cy, cx + side * 0.4 * qCos(1.2), cy - side * 0.4 * qSin(1.2));
    }
}

// ============================================================================
// 3. CyberWaveformWidget (Oscilloscope Strip)
// ============================================================================
CyberWaveformWidget::CyberWaveformWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(44);
}

void CyberWaveformWidget::addSample(double val)
{
    m_samples.append(val);
    if (m_samples.size() > 30) m_samples.removeFirst();
    update();
}

void CyberWaveformWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();

    // Background
    p.setBrush(QColor(QStringLiteral("#070b10")));
    p.setPen(QPen(QColor(QStringLiteral("#1e293b")), 1));
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 4, 4);

    if (m_samples.size() < 2) return;

    QPainterPath path;
    const double stepX = static_cast<double>(w - 8) / (m_samples.size() - 1);

    for (int i = 0; i < m_samples.size(); ++i) {
        double px = 4 + i * stepX;
        double py = (h - 6) - (qBound(0.0, m_samples[i], 1000.0) / 1000.0) * (h - 12);
        if (i == 0) path.moveTo(px, py);
        else path.lineTo(px, py);
    }

    p.setPen(QPen(QColor(QStringLiteral("#00f0ff")), 1.8, Qt::SolidLine, Qt::RoundCap));
    p.drawPath(path);
}

// ============================================================================
// 4. DashboardPage HUD Main Implementation
// ============================================================================
DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::DashboardPage)
{
    ui->setupUi(this);
    setStyleSheet("background-color: #06090e; color: #e2e8f0; font-family: 'Segoe UI', 'Roboto', sans-serif;");

    QSettings settings(QStringLiteral("ICTU"), QStringLiteral("TuanAnhApp"));
    m_autoModeActive = settings.value(QStringLiteral("auto_mode_active"), false).toBool();

    m_relayPendingTimer = new QTimer(this);
    m_relayPendingTimer->setSingleShot(true);
    connect(m_relayPendingTimer, &QTimer::timeout, this, [this] {
        m_isRelayPending = false;
        updateHudState();
    });

    m_autoOffTimer = new QTimer(this);
    m_autoOffTimer->setSingleShot(true);
    connect(m_autoOffTimer, &QTimer::timeout, this, [this] {
        if (m_autoModeActive && !m_curMotion && m_relayState && !m_isRelayPending && !m_deviceId.isEmpty() && m_hasDevice) {
            m_isRelayPending = true;
            m_pendingRelayState = false;
            m_relayPendingTimer->start(3500);
            updateHudState();
            emit relayControlRequested(m_deviceId, false);
        }
    });

    setupHudDashboard();
    updateHudState();
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
    updateHudState();
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
            if (st.contains(QStringLiteral("relay"))) {
                m_relayState = st.value(QStringLiteral("relay")).toBool();
            }
        }
        if (first.contains(QStringLiteral("metrics"))) {
            updateDeviceMetrics(first.value(QStringLiteral("metrics")).toObject());
        }
    } else {
        m_hasDevice = false;
        m_deviceId = "";
        m_curLux = 0.0;
        m_curMotion = false;
    }
    updateHudState();
}

void DashboardPage::updateReading(const SensorReading &reading)
{
    if (reading.pressureHpa > 0) m_curLux = reading.pressureHpa;
    if (m_luxBar) m_luxBar->setValue(m_curLux, 2000.0);
    if (m_waveWidget) m_waveWidget->addSample(m_curLux);
}

void DashboardPage::updateDeviceMetrics(const QJsonObject &metrics)
{
    double valLux = 0.0;
    if (metrics.contains(QStringLiteral("light_lux"))) {
        valLux = metrics.value(QStringLiteral("light_lux")).toDouble();
    } else if (metrics.contains(QStringLiteral("lux")) && metrics.value(QStringLiteral("lux")).toDouble() > 0) {
        valLux = metrics.value(QStringLiteral("lux")).toDouble();
    } else if (metrics.contains(QStringLiteral("detech")) && metrics.value(QStringLiteral("detech")).toDouble() > 1.0) {
        // Firmware telemetry assigned lux reading to "detech"
        valLux = metrics.value(QStringLiteral("detech")).toDouble();
    }
    if (valLux > 0) m_curLux = valLux;

    if (metrics.contains(QStringLiteral("motion_detected"))) {
        m_curMotion = metrics.value(QStringLiteral("motion_detected")).toBool();
    } else if (metrics.contains(QStringLiteral("pir"))) {
        m_curMotion = (metrics.value(QStringLiteral("pir")).toDouble() > 0.5 || metrics.value(QStringLiteral("pir")).toBool());
    } else if (metrics.contains(QStringLiteral("detech")) && metrics.value(QStringLiteral("detech")).toDouble() <= 1.0) {
        m_curMotion = (metrics.value(QStringLiteral("detech")).toDouble() > 0.5);
    }

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

    // Auto Trigger logic: triggers when motion detected OR light level is dark (<= min threshold)
    if (m_autoModeActive && !m_deviceId.isEmpty() && m_hasDevice) {
        const bool isDark = (m_curLux > 0 && m_curLux <= m_minLuxThreshold);
        const bool shouldTurnOn = m_curMotion || isDark;

        if (shouldTurnOn) {
            if (m_autoOffTimer && m_autoOffTimer->isActive()) {
                m_autoOffTimer->stop();
            }
            if (!m_relayState && !m_isRelayPending) {
                m_isRelayPending = true;
                m_pendingRelayState = true;
                m_relayPendingTimer->start(3500);
                updateHudState();
                emit relayControlRequested(m_deviceId, true);
            }
        } else {
            // If it is bright (>= max threshold) or neither motion nor dark, countdown to auto-off
            const bool isTooBright = (m_curLux >= m_maxLuxThreshold && m_maxLuxThreshold > 0);
            if (m_relayState && !m_isRelayPending) {
                if (isTooBright) {
                    m_isRelayPending = true;
                    m_pendingRelayState = false;
                    m_relayPendingTimer->start(3500);
                    updateHudState();
                    emit relayControlRequested(m_deviceId, false);
                } else if (m_autoOffTimer && !m_autoOffTimer->isActive()) {
                    m_autoOffTimer->start(6000);
                }
            }
        }
    } else {
        if (m_autoOffTimer && m_autoOffTimer->isActive()) {
            m_autoOffTimer->stop();
        }
    }

    const QDateTime now = QDateTime::currentDateTime();
    if (m_curLux > 0) m_luxHistory.append({now, m_curLux});
    if (m_luxHistory.size() > 100) m_luxHistory.removeFirst();

    if (m_luxBar) m_luxBar->setValue(m_curLux, 2000.0);
    if (m_waveWidget) m_waveWidget->addSample(m_curLux);
    if (m_radarWidget) m_radarWidget->setDetected(m_curMotion);

    updateHudState();
}

void DashboardPage::openLuxDetail()
{
    SensorDetailDialog dlg(QStringLiteral("BH1750 Ambient Light Sensor"),
                           QStringLiteral("Lux"),
                           QStringLiteral("#00f0ff"),
                           m_luxHistory,
                           this,
                           m_minLuxThreshold, m_maxLuxThreshold);
    connect(&dlg, &SensorDetailDialog::thresholdChanged, this, [this](double minVal, double maxVal) {
        m_minLuxThreshold = minVal;
        m_maxLuxThreshold = maxVal;

        // Automatically activate Auto Trigger mode when user saves threshold
        m_autoModeActive = true;
        QSettings settings(QStringLiteral("ICTU"), QStringLiteral("TuanAnhApp"));
        settings.setValue(QStringLiteral("auto_mode_active"), true);
        if (m_autoModeBtn) {
            m_autoModeBtn->setText(QStringLiteral("🤖 AUTO TRIGGER: ON"));
            m_autoModeBtn->setStyleSheet(QStringLiteral("QPushButton { background: #153229; color: #34d399; border: 1px solid #065f46; border-radius: 6px; font-weight: 900; font-size: 10px; padding: 10px; }"));
        }

        if (!m_deviceId.isEmpty()) {
            QJsonObject thresholds;
            QJsonObject luxObj;
            luxObj.insert(QStringLiteral("min"), minVal);
            luxObj.insert(QStringLiteral("max"), maxVal);
            luxObj.insert(QStringLiteral("warning_below"), minVal);
            luxObj.insert(QStringLiteral("warning_above"), maxVal);
            thresholds.insert(QStringLiteral("light_lux"), luxObj);
            thresholds.insert(QStringLiteral("lux"), luxObj);
            const QJsonObject config{
                {"sampling_interval_ms", 2000},
                {"auto_mode", true},
                {"thresholds", thresholds}
            };
            emit deviceConfigRequested(m_deviceId, config);
        }

        // If current lux is already <= min threshold or motion detected, immediately turn ON relay
        const bool isDark = (m_curLux > 0 && m_curLux <= m_minLuxThreshold);
        if ((m_curMotion || isDark) && !m_relayState && !m_deviceId.isEmpty() && m_hasDevice) {
            m_isRelayPending = true;
            m_pendingRelayState = true;
            m_relayPendingTimer->start(3500);
            updateHudState();
            emit relayControlRequested(m_deviceId, true);
        }
    });
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

void DashboardPage::updateHudState()
{
    if (m_luxValueLbl) {
        if (!m_hasDevice || m_curLux <= 0) {
            m_luxValueLbl->setText(QStringLiteral("--.-"));
            m_luxValueLbl->setStyleSheet("color: #475569; font-size: 28px; font-weight: 900; font-family: monospace;");
        } else {
            m_luxValueLbl->setText(QString::number(m_curLux, 'f', 1));
            m_luxValueLbl->setStyleSheet("color: #00f0ff; font-size: 28px; font-weight: 900; font-family: monospace;");
        }
    }

    if (m_luxStatusBadge) {
        if (!m_hasDevice || m_curLux <= 0) {
            m_luxStatusBadge->setText(QStringLiteral("🔴 OFFLINE / UNLINKED"));
            m_luxStatusBadge->setStyleSheet("color: #f87171; background: #3f151e; border: 1px solid #7f1d1d; border-radius: 4px; padding: 2px 6px; font-size: 8px; font-weight: 900;");
        } else if (m_curLux < 150) {
            m_luxStatusBadge->setText(QStringLiteral("🌙 NIGHT / DARKNESS"));
            m_luxStatusBadge->setStyleSheet("color: #fbbf24; background: #3c2a10; border: 1px solid #92400e; border-radius: 4px; padding: 2px 6px; font-size: 8px; font-weight: 900;");
        } else {
            m_luxStatusBadge->setText(QStringLiteral("☀️ DAYLIGHT NORMAL"));
            m_luxStatusBadge->setStyleSheet("color: #10b981; background: #0f3728; border: 1px solid #065f46; border-radius: 4px; padding: 2px 6px; font-size: 8px; font-weight: 900;");
        }
    }

    if (m_motionStatusBadge) {
        if (m_curMotion) {
            m_motionStatusBadge->setText(QStringLiteral("🚨 MOTION DETECTED [SECTOR 1]"));
            m_motionStatusBadge->setStyleSheet("color: #ffffff; background: #10b981; border: 1px solid #34d399; border-radius: 4px; padding: 3px 8px; font-size: 9px; font-weight: 900;");
            if (m_motionDetailLbl) m_motionDetailLbl->setText(QStringLiteral("Cảm biến PIR kích hoạt - Có người hiện diện"));
        } else {
            m_motionStatusBadge->setText(QStringLiteral("🛡️ PERIMETER SECURE"));
            m_motionStatusBadge->setStyleSheet("color: #94a3b8; background: #111827; border: 1px solid #1f2937; border-radius: 4px; padding: 3px 8px; font-size: 9px; font-weight: 800;");
            if (m_motionDetailLbl) m_motionDetailLbl->setText(QStringLiteral("Khu vực yên tĩnh - Đang quét an ninh"));
        }
    }

    if (m_lightSwitchBtn) {
        if (m_isRelayPending) {
            if (m_pendingRelayState) {
                m_lightSwitchBtn->setText(QStringLiteral("⏳ ĐANG BẬT ĐÈN..."));
                m_lightSwitchBtn->setStyleSheet(
                    "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #d97706, stop:1 #00f0ff); color: #000000; border: 2px solid #38bdf8; border-radius: 6px; font-weight: 900; font-size: 10px; padding: 10px; }"
                );
            } else {
                m_lightSwitchBtn->setText(QStringLiteral("⏳ ĐANG TẮT ĐÈN..."));
                m_lightSwitchBtn->setStyleSheet(
                    "QPushButton { background: #261608; color: #fbbf24; border: 1.5px solid #d97706; border-radius: 6px; font-weight: 900; font-size: 10px; padding: 10px; }"
                );
            }
        } else {
            m_lightSwitchBtn->setText(m_relayState ? QStringLiteral("💡 MAIN LIGHT: ON") : QStringLiteral("🌑 MAIN LIGHT: OFF"));
            m_lightSwitchBtn->setStyleSheet(m_relayState
                ? "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00f0ff, stop:1 #10b981); color: #000000; border: none; border-radius: 6px; font-weight: 900; font-size: 11px; padding: 10px; } QPushButton:hover { background: #38bdf8; }"
                : "QPushButton { background: #111c24; color: #64748b; border: 1.5px solid #1e293b; border-radius: 6px; font-weight: 900; font-size: 11px; padding: 10px; } QPushButton:hover { background: #1a2a38; color: #ffffff; }");
        }
    }

    if (m_devPodTitle) {
        m_devPodTitle->setText(m_hasDevice ? QStringLiteral("POD: %1").arg(m_deviceId) : QStringLiteral("HARDWARE POD: UNLINKED"));
    }

    if (m_devStatusBadge) {
        m_devStatusBadge->setText(m_hasDevice ? (m_isOnline ? QStringLiteral("🟢 ARMED & ONLINE") : QStringLiteral("🔴 LINK OFFLINE")) : QStringLiteral("⚪ NO HARDWARE"));
        m_devStatusBadge->setStyleSheet(m_hasDevice
            ? (m_isOnline ? "color: #10b981; font-size: 9px; font-weight: 900;" : "color: #ef4444; font-size: 9px; font-weight: 900;")
            : "color: #64748b; font-size: 9px; font-weight: 700;");
    }

    if (m_devActionBtn) {
        if (!m_hasDevice) {
            m_devActionBtn->setText(QStringLiteral("＋ CONNECT LINK"));
            m_devActionBtn->setStyleSheet("QPushButton { background: #0284c7; color: #ffffff; border: none; border-radius: 5px; font-size: 10px; font-weight: 900; padding: 6px; } QPushButton:hover { background: #0369a1; }");
        } else {
            m_devActionBtn->setText(QStringLiteral("✕ DISCONNECT"));
            m_devActionBtn->setStyleSheet("QPushButton { background: #3b1424; color: #f87171; border: 1px solid #7f1d1d; border-radius: 5px; font-size: 9px; font-weight: 800; padding: 5px; } QPushButton:hover { background: #dc2626; color: #fff; }");
        }
    }
}

void DashboardPage::setupHudDashboard()
{
    while (QLayoutItem *item = ui->verticalLayout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    auto *mainLayout = ui->verticalLayout;
    mainLayout->setContentsMargins(10, 8, 10, 8);
    mainLayout->setSpacing(8);

    auto makeHudPod = [](const QString &tag) {
        auto *pod = new QFrame;
        pod->setStyleSheet(
            "QFrame { "
            "  background-color: #0c1218; "
            "  border: 1px solid #162430; "
            "  border-radius: 8px; "
            "} "
            "QFrame:hover { "
            "  border-color: #00f0ff; "
            "}"
        );
        auto *l = new QVBoxLayout(pod);
        l->setContentsMargins(10, 8, 10, 8);
        l->setSpacing(6);

        auto *hRow = new QHBoxLayout;
        auto *tagLbl = new QLabel(QStringLiteral("⬢ %1").arg(tag));
        tagLbl->setStyleSheet("color: #00f0ff; font-size: 9px; font-weight: 900; letter-spacing: 0.5px; background: transparent;");
        auto *hudDot = new QLabel(QStringLiteral("●"));
        hudDot->setStyleSheet("color: #10b981; font-size: 8px; background: transparent;");
        hRow->addWidget(tagLbl);
        hRow->addStretch();
        hRow->addWidget(hudDot);
        l->addLayout(hRow);

        return qMakePair(pod, l);
    };

    struct ClickFilter : public QObject {
        std::function<void()> fn;
        ClickFilter(QObject *p, std::function<void()> cb) : QObject(p), fn(cb) {}
        bool eventFilter(QObject *w, QEvent *e) override {
            if (e->type() == QEvent::MouseButtonRelease) {
                if (fn) fn();
                return true;
            }
            return QObject::eventFilter(w, e);
        }
    };

    auto *hudRow = new QHBoxLayout;
    hudRow->setSpacing(8);

    // ========================================================================
    // ZONE 1: MASTER LUX SENSOR HUD (Left 28%)
    // ========================================================================
    auto z1 = makeHudPod("CORE SENSOR // BH1750");
    z1.first->setCursor(Qt::PointingHandCursor);

    auto *luxContent = new QHBoxLayout;
    luxContent->setSpacing(10);

    m_luxBar = new VerticalLuxBarWidget;
    luxContent->addWidget(m_luxBar);

    auto *luxDataCol = new QVBoxLayout;
    luxDataCol->setSpacing(4);

    m_luxValueLbl = new QLabel(QStringLiteral("--.-"));
    m_luxValueLbl->setStyleSheet("color: #00f0ff; font-size: 28px; font-weight: 900; font-family: monospace;");
    luxDataCol->addWidget(m_luxValueLbl);

    auto *unitLbl = new QLabel(QStringLiteral("LUX (LUMEN / M²)"));
    unitLbl->setStyleSheet("color: #64748b; font-size: 8px; font-weight: 800;");
    luxDataCol->addWidget(unitLbl);

    m_luxStatusBadge = new QLabel(QStringLiteral("🔴 OFFLINE"));
    m_luxStatusBadge->setStyleSheet("color: #f87171; background: #3f151e; border: 1px solid #7f1d1d; border-radius: 4px; padding: 2px 6px; font-size: 8px; font-weight: 900;");
    luxDataCol->addWidget(m_luxStatusBadge);

    luxDataCol->addSpacing(4);
    auto *waveTitle = new QLabel(QStringLiteral("REALTIME OSCILLOSCOPE:"));
    waveTitle->setStyleSheet("color: #475569; font-size: 7px; font-weight: 800;");
    luxDataCol->addWidget(waveTitle);

    m_waveWidget = new CyberWaveformWidget;
    luxDataCol->addWidget(m_waveWidget);

    luxContent->addLayout(luxDataCol, 1);
    z1.second->addLayout(luxContent);

    z1.first->installEventFilter(new ClickFilter(z1.first, [this] { openLuxDetail(); }));
    hudRow->addWidget(z1.first, 28);

    // ========================================================================
    // ZONE 2: TACTICAL RADAR & DUAL SWITCHES (Center 46%)
    // ========================================================================
    auto z2 = makeHudPod("TACTICAL DEFENSE // PIR SENSOR & CONTROLS");

    auto *radarRow = new QHBoxLayout;
    radarRow->setSpacing(8);

    m_radarWidget = new TacticalRadarWidget;
    radarRow->addWidget(m_radarWidget, 0, Qt::AlignCenter);

    auto *radarInfoCol = new QVBoxLayout;
    radarInfoCol->setSpacing(4);
    radarInfoCol->setAlignment(Qt::AlignVCenter);

    m_motionStatusBadge = new QLabel(QStringLiteral("🛡️ PERIMETER SECURE"));
    m_motionStatusBadge->setStyleSheet("color: #94a3b8; background: #111827; border: 1px solid #1f2937; border-radius: 4px; padding: 3px 8px; font-size: 9px; font-weight: 800;");
    radarInfoCol->addWidget(m_motionStatusBadge);

    m_motionDetailLbl = new QLabel(QStringLiteral("Khu vực an toàn - Quét 360°"));
    m_motionDetailLbl->setStyleSheet("color: #64748b; font-size: 8px;");
    radarInfoCol->addWidget(m_motionDetailLbl);

    radarRow->addLayout(radarInfoCol);
    z2.second->addLayout(radarRow);

    // Dual Tactical Switch Console
    auto *switchGrid = new QHBoxLayout;
    switchGrid->setSpacing(6);

    m_lightSwitchBtn = new QPushButton(QStringLiteral("💡 MAIN LIGHT: OFF"));
    m_lightSwitchBtn->setCursor(Qt::PointingHandCursor);
    m_lightSwitchBtn->setStyleSheet("QPushButton { background: #111c24; color: #64748b; border: 1.5px solid #1e293b; border-radius: 6px; font-weight: 900; font-size: 11px; padding: 10px; }");
    connect(m_lightSwitchBtn, &QPushButton::clicked, this, [this] {
        if (!m_deviceId.isEmpty()) {
            m_isRelayPending = true;
            m_pendingRelayState = !m_relayState;
            m_relayPendingTimer->start(3500);
            updateHudState();
            emit relayControlRequested(m_deviceId, m_pendingRelayState);
        }
    });
    switchGrid->addWidget(m_lightSwitchBtn, 1);

    m_autoModeBtn = new QPushButton(m_autoModeActive ? QStringLiteral("🤖 AUTO TRIGGER: ON") : QStringLiteral("🤖 AUTO TRIGGER: OFF"));
    m_autoModeBtn->setCursor(Qt::PointingHandCursor);
    m_autoModeBtn->setStyleSheet(m_autoModeActive
        ? "QPushButton { background: #153229; color: #34d399; border: 1px solid #065f46; border-radius: 6px; font-weight: 900; font-size: 10px; padding: 10px; }"
        : "QPushButton { background: #1c1917; color: #78716c; border: 1px solid #292524; border-radius: 6px; font-weight: 900; font-size: 10px; padding: 10px; }");
    connect(m_autoModeBtn, &QPushButton::clicked, this, [this] {
        m_autoModeActive = !m_autoModeActive;
        QSettings settings(QStringLiteral("ICTU"), QStringLiteral("TuanAnhApp"));
        settings.setValue(QStringLiteral("auto_mode_active"), m_autoModeActive);

        m_autoModeBtn->setText(m_autoModeActive ? QStringLiteral("🤖 AUTO TRIGGER: ON") : QStringLiteral("🤖 AUTO TRIGGER: OFF"));
        m_autoModeBtn->setStyleSheet(m_autoModeActive
            ? "QPushButton { background: #153229; color: #34d399; border: 1px solid #065f46; border-radius: 6px; font-weight: 900; font-size: 10px; padding: 10px; }"
            : "QPushButton { background: #1c1917; color: #78716c; border: 1px solid #292524; border-radius: 6px; font-weight: 900; font-size: 10px; padding: 10px; }");

        // Sync auto_mode state down to device config
        if (!m_deviceId.isEmpty()) {
            QJsonObject thresholds;
            QJsonObject luxObj;
            luxObj.insert(QStringLiteral("min"), m_minLuxThreshold);
            luxObj.insert(QStringLiteral("max"), m_maxLuxThreshold);
            luxObj.insert(QStringLiteral("warning_below"), m_minLuxThreshold);
            luxObj.insert(QStringLiteral("warning_above"), m_maxLuxThreshold);
            thresholds.insert(QStringLiteral("light_lux"), luxObj);
            thresholds.insert(QStringLiteral("lux"), luxObj);
            const QJsonObject config{
                {"sampling_interval_ms", 2000},
                {"auto_mode", m_autoModeActive},
                {"thresholds", thresholds}
            };
            emit deviceConfigRequested(m_deviceId, config);
        }

        // If turned ON while motion or dark is active and relay is OFF, immediately trigger relay ON
        const bool isDark = (m_curLux > 0 && m_curLux <= m_minLuxThreshold);
        if (m_autoModeActive && (m_curMotion || isDark) && !m_relayState && !m_deviceId.isEmpty() && m_hasDevice) {
            m_isRelayPending = true;
            m_pendingRelayState = true;
            m_relayPendingTimer->start(3500);
            updateHudState();
            emit relayControlRequested(m_deviceId, true);
        } else if (!m_autoModeActive && m_autoOffTimer && m_autoOffTimer->isActive()) {
            m_autoOffTimer->stop();
        }
    });
    switchGrid->addWidget(m_autoModeBtn, 1);

    z2.second->addLayout(switchGrid);
    hudRow->addWidget(z2.first, 46);

    // ========================================================================
    // ZONE 3: HARDWARE LINK POD & TELEMETRY (Right 26%)
    // ========================================================================
    auto z3 = makeHudPod("HARDWARE POD // ESP32");

    m_devPodTitle = new QLabel(QStringLiteral("POD: 150808"));
    m_devPodTitle->setStyleSheet("color: #ffffff; font-size: 11px; font-weight: 900;");
    z3.second->addWidget(m_devPodTitle);

    m_devStatusBadge = new QLabel(QStringLiteral("⚪ NO HARDWARE"));
    m_devStatusBadge->setStyleSheet("color: #64748b; font-size: 9px; font-weight: 700;");
    z3.second->addWidget(m_devStatusBadge);

    // Telemetry lines
    auto *diagBox = new QFrame;
    diagBox->setStyleSheet("background: #080c10; border: 1px solid #141f29; border-radius: 4px; padding: 4px;");
    auto *diagL = new QVBoxLayout(diagBox);
    diagL->setContentsMargins(4, 4, 4, 4);
    diagL->setSpacing(2);

    auto makeDiagRow = [](const QString &k, const QString &v, const QString &c = "#94a3b8") {
        auto *r = new QHBoxLayout;
        auto *kLbl = new QLabel(k);
        kLbl->setStyleSheet("color: #475569; font-size: 8px; font-weight: 700; background: transparent;");
        auto *vLbl = new QLabel(v);
        vLbl->setStyleSheet(QStringLiteral("color: %1; font-size: 8px; font-weight: 900; background: transparent;").arg(c));
        r->addWidget(kLbl);
        r->addStretch();
        r->addWidget(vLbl);
        return r;
    };

    diagL->addLayout(makeDiagRow("MQTT PROTOCOL", "v3.1.1", "#00f0ff"));
    diagL->addLayout(makeDiagRow("SAMPLE RATE", "2000 ms", "#38bdf8"));
    diagL->addLayout(makeDiagRow("SECURITY MESH", "ONLINE", "#10b981"));
    z3.second->addWidget(diagBox);

    z3.second->addStretch();

    m_devActionBtn = new QPushButton(QStringLiteral("＋ CONNECT LINK"));
    m_devActionBtn->setCursor(Qt::PointingHandCursor);
    connect(m_devActionBtn, &QPushButton::clicked, this, [this] {
        if (!m_hasDevice) openAddDeviceDialog();
        else emit releaseDeviceRequested(m_deviceId);
    });
    z3.second->addWidget(m_devActionBtn);

    hudRow->addWidget(z3.first, 26);

    mainLayout->addLayout(hudRow, 1);
}
