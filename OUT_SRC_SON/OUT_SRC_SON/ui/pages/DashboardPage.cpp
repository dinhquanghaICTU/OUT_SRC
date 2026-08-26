#include "DashboardPage.h"
#include "ui_DashboardPage.h"

#include <QChart>
#include <QChartView>
#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineSeries>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStackedWidget>
#include <QValueAxis>
#include <QVBoxLayout>

namespace {

QFrame *createCard(const QString &title, const QString &icon = QString())
{
    auto *card = new QFrame;
    card->setObjectName(QStringLiteral("dashCard"));
    card->setStyleSheet(
        "QFrame#dashCard { "
        "  background-color: #0d1733; "
        "  border: 1px solid #1c2b54; "
        "  border-radius: 10px; "
        "} "
        "QFrame#dashCard:hover { "
        "  border-color: #2b3d75; "
        "}"
    );

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(4);

    if (!title.isEmpty()) {
        auto *header = new QHBoxLayout;
        header->setSpacing(5);
        if (!icon.isEmpty()) {
            auto *iconLbl = new QLabel(icon);
            iconLbl->setStyleSheet("color: #38bdf8; font-size: 11px;");
            header->addWidget(iconLbl);
        }
        auto *titleLbl = new QLabel(title);
        titleLbl->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: 700; text-transform: uppercase;");
        header->addWidget(titleLbl);
        header->addStretch();
        layout->addLayout(header);
    }

    return card;
}

QChartView *buildChartView(QList<QLineSeries *> seriesList, QValueAxis **axisXOut, QValueAxis **axisYOut,
                           double minY, double maxY, const QString &yTitle)
{
    auto *chart = new QChart;
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));
    chart->legend()->hide();

    auto *axisX = new QValueAxis(chart);
    axisX->setRange(0, 30);
    axisX->setTickCount(4);
    axisX->setLabelFormat(QStringLiteral("%d"));
    axisX->setGridLineColor(QColor(QStringLiteral("#1e293b")));
    axisX->setLabelsColor(QColor(QStringLiteral("#64748b")));
    QFont axisFont;
    axisFont.setPixelSize(9);
    axisX->setLabelsFont(axisFont);

    auto *axisY = new QValueAxis(chart);
    axisY->setRange(minY, maxY);
    axisY->setTickCount(4);
    axisY->setTitleText(yTitle);
    axisY->setTitleFont(axisFont);
    axisY->setTitleBrush(QBrush(QColor(QStringLiteral("#94a3b8"))));
    axisY->setGridLineColor(QColor(QStringLiteral("#1e293b")));
    axisY->setLabelsColor(QColor(QStringLiteral("#64748b")));
    axisY->setLabelsFont(axisFont);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto *series : seriesList) {
        chart->addSeries(series);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    *axisXOut = axisX;
    *axisYOut = axisY;

    auto *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    return view;
}

} // namespace

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::DashboardPage)
{
    ui->setupUi(this);
    setStyleSheet("background-color: #070d1e; color: #ecf2ff;");

    QSettings settings(QStringLiteral("ICTU"), QStringLiteral("SonApp"));
    m_autoPumpMode = settings.value(QStringLiteral("auto_pump_mode"), false).toBool();
    m_distanceStartCm = settings.value(QStringLiteral("distance_start_cm"), 35.0).toDouble();
    m_distanceStopCm = settings.value(QStringLiteral("distance_stop_cm"), 10.0).toDouble();

    setupDashboardLayout();
    updateDisplays();
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

void DashboardPage::setUsername(const QString &username)
{
    Q_UNUSED(username);
}

void DashboardPage::setDeviceId(const QString &deviceId)
{
    if (!deviceId.isEmpty())
        m_deviceId = deviceId;
}

void DashboardPage::setHasDevice(bool hasDevice, const QString &deviceId, const QString &deviceName, bool isOnline)
{
    m_hasDevice = hasDevice;
    if (!deviceId.isEmpty())
        m_deviceId = deviceId;
    if (!deviceName.isEmpty())
        m_deviceName = deviceName;
    m_isOnline = isOnline;

    if (m_pumpCardStack) {
        m_pumpCardStack->setCurrentIndex(hasDevice ? 1 : 0);
    }

    if (m_pumpDeviceNameLbl && !m_deviceName.isEmpty()) {
        m_pumpDeviceNameLbl->setText(m_deviceName);
    }

    updateDisplays();
}

void DashboardPage::setDeviceOnline(bool online)
{
    m_isOnline = online;
    updateDisplays();
}

void DashboardPage::updateReading(const SensorReading &reading)
{
    const QDateTime now = reading.measuredAt.isValid() ? reading.measuredAt : QDateTime::currentDateTime();

    if (reading.temperatureC > 0)
        m_currentTemp = reading.temperatureC;

    if (reading.pressureHpa > 0)
        m_currentPressure = reading.pressureHpa;

    if (reading.distanceCm >= 0) {
        m_currentDistance = reading.distanceCm;
        m_distanceHistory.append({now, m_currentDistance});
        if (m_distanceHistory.size() > 300) m_distanceHistory.removeFirst();
        appendPoint(m_distanceSeries, m_distanceAxisX, m_distanceAxisY, m_currentDistance, 0.0, 60.0);
    }

    updateDisplays();
}

void DashboardPage::updateDeviceMetrics(const QJsonObject &metrics)
{
    const QDateTime now = QDateTime::currentDateTime();
    bool hasUpdate = false;

    // 1. Water Flow (flow_l_min)
    if (metrics.contains(QStringLiteral("flow_l_min"))) {
        m_currentFlow = metrics.value(QStringLiteral("flow_l_min")).toDouble();
        m_flowHistory.append({now, m_currentFlow});
        if (m_flowHistory.size() > 300) m_flowHistory.removeFirst();
        appendPoint(m_flowSeries, m_flowAxisX, m_flowAxisY, m_currentFlow, 0.0, 10.0);
        hasUpdate = true;
    }

    // 2. Distance (distance_cm from HC-SR04)
    if (metrics.contains(QStringLiteral("distance_cm"))) {
        const double rawDist = metrics.value(QStringLiteral("distance_cm")).toDouble();
        m_currentDistance = (rawDist >= 0.0) ? rawDist : 0.0;
        m_distanceHistory.append({now, m_currentDistance});
        if (m_distanceHistory.size() > 300) m_distanceHistory.removeFirst();
        appendPoint(m_distanceSeries, m_distanceAxisX, m_distanceAxisY, m_currentDistance, 0.0, 60.0);
        hasUpdate = true;
    }

    // 3. Relay Pump State
    if (metrics.contains(QStringLiteral("pump_on"))) {
        m_pumpOn = metrics.value(QStringLiteral("pump_on")).toBool();
        hasUpdate = true;
    } else if (metrics.contains(QStringLiteral("relay"))) {
        m_pumpOn = metrics.value(QStringLiteral("relay")).toBool();
        hasUpdate = true;
    }

    if (metrics.contains(QStringLiteral("total_liters"))) {
        m_totalLiters = metrics.value(QStringLiteral("total_liters")).toDouble();
        hasUpdate = true;
    }
    if (metrics.contains(QStringLiteral("temperature_c"))) {
        m_currentTemp = metrics.value(QStringLiteral("temperature_c")).toDouble();
        hasUpdate = true;
    }
    if (metrics.contains(QStringLiteral("pressure_hpa"))) {
        m_currentPressure = metrics.value(QStringLiteral("pressure_hpa")).toDouble();
        hasUpdate = true;
    }

    if (hasUpdate) {
        updateDisplays();
    }
}

void DashboardPage::appendPoint(QLineSeries *series, QValueAxis *axisX, QValueAxis *axisY,
                                double value, double minVal, double maxVal)
{
    if (!series || !axisX || !axisY)
        return;

    const int count = series->count();
    series->append(count, value);

    if (count > 30) {
        series->remove(0);
        for (int i = 0; i < series->count(); ++i) {
            series->replace(i, i, series->at(i).y());
        }
        axisX->setRange(0, 30);
    } else {
        axisX->setRange(0, qMax(10, count));
    }

    qreal currentMin = minVal;
    qreal currentMax = maxVal;
    for (const QPointF &pt : series->points()) {
        currentMin = qMin(currentMin, pt.y() - 1.0);
        currentMax = qMax(currentMax, pt.y() + 1.0);
    }
    axisY->setRange(currentMin, currentMax);
}

void DashboardPage::openSensorDetail(const QString &sensorName, const QString &unit,
                                     const QString &accentColor, const QVector<SensorDataPoint> &history)
{
    double initialMin = 0.0;
    double initialMax = 100.0;
    if (sensorName.contains(QStringLiteral("Khoảng cách"), Qt::CaseInsensitive)) {
        initialMin = m_distanceStopCm;
        initialMax = m_distanceStartCm;
    }

    SensorDetailDialog dlg(sensorName, unit, accentColor, history, this, initialMin, initialMax);
    connect(&dlg, &SensorDetailDialog::thresholdChanged, this, [this, sensorName](double minVal, double maxVal) {
        if (!m_deviceId.isEmpty()) {
            QJsonObject thresholds;
            if (sensorName.contains(QStringLiteral("Khoảng cách"), Qt::CaseInsensitive)) {
                m_distanceStopCm = minVal;
                m_distanceStartCm = maxVal;
                QJsonObject distObj;
                distObj.insert(QStringLiteral("min"), minVal);
                distObj.insert(QStringLiteral("max"), maxVal);
                distObj.insert(QStringLiteral("warning_below"), minVal);
                distObj.insert(QStringLiteral("warning_above"), maxVal);
                thresholds.insert(QStringLiteral("distance_cm"), distObj);

                QSettings settings(QStringLiteral("ICTU"), QStringLiteral("SonApp"));
                settings.setValue(QStringLiteral("distance_start_cm"), m_distanceStartCm);
                settings.setValue(QStringLiteral("distance_stop_cm"), m_distanceStopCm);

                const QJsonObject config{
                    {"auto_mode", m_autoPumpMode},
                    {"distance_start_cm", maxVal},
                    {"distance_stop_cm", minVal},
                    {"sampling_interval_ms", 2000},
                    {"thresholds", thresholds}
                };
                emit deviceConfigRequested(m_deviceId, config);
            } else if (sensorName.contains(QStringLiteral("Lưu lượng"), Qt::CaseInsensitive)) {
                QJsonObject flowObj;
                flowObj.insert(QStringLiteral("min"), minVal);
                flowObj.insert(QStringLiteral("max"), maxVal);
                flowObj.insert(QStringLiteral("warning_below"), minVal);
                flowObj.insert(QStringLiteral("warning_above"), maxVal);
                thresholds.insert(QStringLiteral("flow_l_min"), flowObj);

                const QJsonObject config{
                    {"sampling_interval_ms", 2000},
                    {"thresholds", thresholds}
                };
                emit deviceConfigRequested(m_deviceId, config);
            }
        }
    });
    dlg.exec();
}

void DashboardPage::openPumpAutoConfig()
{
    PumpAutoConfigDialog dlg(m_deviceId, m_autoPumpMode, m_distanceStartCm, m_distanceStopCm, this);
    connect(&dlg, &PumpAutoConfigDialog::configSaved, this, [this](const QString &devId, const QJsonObject &config) {
        m_autoPumpMode = config.value(QStringLiteral("auto_mode")).toBool();
        m_distanceStartCm = config.value(QStringLiteral("distance_start_cm")).toDouble(35.0);
        m_distanceStopCm = config.value(QStringLiteral("distance_stop_cm")).toDouble(10.0);

        QSettings settings(QStringLiteral("ICTU"), QStringLiteral("SonApp"));
        settings.setValue(QStringLiteral("auto_pump_mode"), m_autoPumpMode);
        settings.setValue(QStringLiteral("distance_start_cm"), m_distanceStartCm);
        settings.setValue(QStringLiteral("distance_stop_cm"), m_distanceStopCm);

        emit deviceConfigRequested(devId, config);
    });
    dlg.exec();
}

void DashboardPage::setupDashboardLayout()
{
    auto *mainLayout = ui->verticalLayout;
    mainLayout->setContentsMargins(8, 6, 8, 6);
    mainLayout->setSpacing(6);

    // ==========================================
    // ROW 1: HERO CARD (Left) + PUMP RELAY CARD / ADD DEVICE STACK (Right)
    // ==========================================
    auto *topRow = new QHBoxLayout;
    topRow->setSpacing(6);

    // --- 1. HERO CLIMATE CARD (Left) ---
    auto *heroCard = new QFrame;
    heroCard->setObjectName(QStringLiteral("heroCard"));
    heroCard->setStyleSheet(
        "QFrame#heroCard { "
        "  background: #0e1938 url(:/images/hero_building_bg.png) no-repeat center center; "
        "  background-size: cover; "
        "  border: 1px solid #223565; "
        "  border-radius: 12px; "
        "}"
    );
    auto *heroLayout = new QVBoxLayout(heroCard);
    heroLayout->setContentsMargins(12, 6, 12, 6);
    heroLayout->setSpacing(4);

    // Hero Title
    auto *heroHead = new QHBoxLayout;
    auto *dropIcon = new QLabel(QStringLiteral("💧"));
    dropIcon->setStyleSheet("font-size: 11px;");
    auto *heroTitle = new QLabel(QStringLiteral("Son Environmental & Pump Monitor"));
    heroTitle->setStyleSheet("color: #cbd5e1; font-size: 11px; font-weight: 700;");
    heroHead->addWidget(dropIcon);
    heroHead->addWidget(heroTitle);
    heroHead->addStretch();
    heroLayout->addLayout(heroHead);

    // Hero Center: Left AC Display Box + Right Metrics
    auto *heroCenter = new QHBoxLayout;
    heroCenter->setSpacing(10);

    // Left AC Badge Box
    auto *acBox = new QFrame;
    acBox->setStyleSheet(
        "QFrame { "
        "  background-color: rgba(14, 25, 58, 0.75); "
        "  border: 1.5px solid rgba(52, 211, 153, 0.7); "
        "  border-radius: 12px; "
        "}"
    );
    auto *acLayout = new QVBoxLayout(acBox);
    acLayout->setContentsMargins(8, 4, 8, 4);
    acLayout->setSpacing(1);

    auto *onBadgeRow = new QHBoxLayout;
    auto *onBadge = new QLabel(QStringLiteral("ON"));
    onBadge->setStyleSheet("background-color: #10b981; color: #ffffff; font-size: 9px; font-weight: 900; border-radius: 8px; padding: 1px 6px;");
    onBadgeRow->addWidget(onBadge);
    onBadgeRow->addStretch();
    acLayout->addLayout(onBadgeRow);

    auto *tempTitle = new QLabel(QStringLiteral("🌡 TEMPERATURE"));
    tempTitle->setStyleSheet("color: #38bdf8; font-size: 9px; font-weight: 800; border: none; background: transparent;");
    acLayout->addWidget(tempTitle);

    m_heroTempValue = new QLabel(QStringLiteral("24.0 °C"));
    m_heroTempValue->setStyleSheet("color: #ffffff; font-size: 20px; font-weight: 900; font-family: monospace; border: none; background: transparent;");
    acLayout->addWidget(m_heroTempValue);

    auto *airWaves = new QLabel(QStringLiteral("SSS   | | |   SSS"));
    airWaves->setStyleSheet("color: #64748b; font-size: 9px; font-weight: 900; letter-spacing: 2px; border: none; background: transparent;");
    airWaves->setAlignment(Qt::AlignCenter);
    acLayout->addWidget(airWaves);

    heroCenter->addWidget(acBox, 3);

    // Right Metrics in Hero (Humidity + Pressure)
    auto *heroRight = new QVBoxLayout;
    heroRight->setSpacing(2);

    auto *humHeader = new QLabel(QStringLiteral("💧 HUMIDITY"));
    humHeader->setStyleSheet("color: #38bdf8; font-size: 9px; font-weight: 800; background: transparent;");
    heroRight->addWidget(humHeader);

    m_heroHumidityValue = new QLabel(QStringLiteral("60.0 %"));
    m_heroHumidityValue->setStyleSheet("color: #ffffff; font-size: 16px; font-weight: 900; font-family: monospace; background: transparent;");
    heroRight->addWidget(m_heroHumidityValue);

    auto *pressHeader = new QLabel(QStringLiteral("⏱ PRESSURE"));
    pressHeader->setStyleSheet("color: #38bdf8; font-size: 9px; font-weight: 800; background: transparent;");
    heroRight->addWidget(pressHeader);

    m_heroPressureValue = new QLabel(QStringLiteral("1002 mbar"));
    m_heroPressureValue->setStyleSheet("color: #ffffff; font-size: 16px; font-weight: 900; font-family: monospace; background: transparent;");
    heroRight->addWidget(m_heroPressureValue);

    heroCenter->addLayout(heroRight, 2);
    heroLayout->addLayout(heroCenter);

    // Bottom 3 mini pills inside Hero
    auto *pillRow = new QHBoxLayout;
    pillRow->setSpacing(6);

    auto makeGlassPill = [](const QString &t, const QString &val, const QString &valColor) {
        auto *p = new QFrame;
        p->setStyleSheet("background-color: rgba(14, 25, 58, 0.65); border: 1px solid rgba(56, 189, 248, 0.3); border-radius: 6px;");
        auto *l = new QVBoxLayout(p);
        l->setContentsMargins(4, 2, 4, 2);
        l->setSpacing(1);
        auto *tLbl = new QLabel(t);
        tLbl->setStyleSheet("color: #94a3b8; font-size: 8px; font-weight: 700; background: transparent; border: none;");
        auto *vLbl = new QLabel(val);
        vLbl->setStyleSheet(QStringLiteral("color: %1; font-size: 10px; font-weight: 900; background: transparent; border: none;").arg(valColor));
        l->addWidget(tLbl);
        l->addWidget(vLbl);
        return qMakePair(p, vLbl);
    };

    auto p1 = makeGlassPill("AC OPERATION", "☼ Heat", "#fbbf24");
    auto p2 = makeGlassPill("IONIZATION", "🫧 On", "#38bdf8");
    auto p3 = makeGlassPill("PUMP STATUS", "OFF", "#ef4444");

    m_heroAcOpValue = p1.second;
    m_heroIonValue = p2.second;
    m_heroFanValue = p3.second;

    pillRow->addWidget(p1.first);
    pillRow->addWidget(p2.first);
    pillRow->addWidget(p3.first);
    heroLayout->addLayout(pillRow);

    topRow->addWidget(heroCard, 11);

    // --- 2. PUMP CONTROL / ADD DEVICE STACK (Right Top) ---
    m_pumpCardStack = new QStackedWidget;

    // === PAGE 0: NO DEVICE (+ Big Add Button) ===
    auto *noDeviceCard = createCard(QStringLiteral("Thêm Thiết Bị"), QStringLiteral("➕"));
    auto *noDevLayout = static_cast<QVBoxLayout *>(noDeviceCard->layout());
    noDevLayout->setContentsMargins(12, 10, 12, 10);
    noDevLayout->setSpacing(6);

    auto *bigPlusBtn = new QPushButton(QStringLiteral("➕"));
    bigPlusBtn->setCursor(Qt::PointingHandCursor);
    bigPlusBtn->setFixedSize(54, 54);
    bigPlusBtn->setStyleSheet(
        "QPushButton { "
        "  background: #10b981; "
        "  color: #ffffff; "
        "  border: 2px solid #34d399; "
        "  border-radius: 27px; "
        "  font-size: 24px; "
        "  font-weight: 900; "
        "} "
        "QPushButton:hover { "
        "  background: #059669; "
        "  border-color: #6ee7b7; "
        "}"
    );
    connect(bigPlusBtn, &QPushButton::clicked, this, &DashboardPage::addDeviceRequested);

    auto *btnCenterRow = new QHBoxLayout;
    btnCenterRow->addStretch();
    btnCenterRow->addWidget(bigPlusBtn);
    btnCenterRow->addStretch();
    noDevLayout->addLayout(btnCenterRow);

    auto *addPromptLbl = new QLabel(QStringLiteral("Chưa có thiết bị kết nối"));
    addPromptLbl->setAlignment(Qt::AlignCenter);
    addPromptLbl->setStyleSheet("color: #ffffff; font-size: 12px; font-weight: 800;");
    noDevLayout->addWidget(addPromptLbl);

    auto *addHintLbl = new QLabel(QStringLiteral("Nhấn dấu + để chọn ESP32 Online"));
    addHintLbl->setAlignment(Qt::AlignCenter);
    addHintLbl->setStyleSheet("color: #94a3b8; font-size: 10px;");
    noDevLayout->addWidget(addHintLbl);

    m_pumpCardStack->addWidget(noDeviceCard);

    // === PAGE 1: ACTIVE PUMP CONTROL CARD ===
    auto *pumpCard = createCard(QStringLiteral("Điều khiển Bơm (Relay)"), QStringLiteral("⚡"));
    auto *pumpLayout = static_cast<QVBoxLayout *>(pumpCard->layout());
    pumpLayout->setContentsMargins(12, 8, 12, 8);
    pumpLayout->setSpacing(5);

    // Status Row with Device Name, Online Badge, Status Badge, Auto Config, Add and Unbind Button
    auto *statusRow = new QHBoxLayout;
    statusRow->setSpacing(4);
    m_pumpDeviceNameLbl = new QLabel(QStringLiteral("son-190782"));
    m_pumpDeviceNameLbl->setStyleSheet("color: #38bdf8; font-size: 11px; font-weight: 800;");

    m_pumpOnlineBadge = new QLabel(QStringLiteral("🟢 ONLINE"));
    m_pumpOnlineBadge->setStyleSheet("color: #10b981; font-size: 9px; font-weight: 900; background: rgba(16, 185, 129, 0.15); border-radius: 4px; padding: 2px 6px;");

    m_pumpStatusBadge = new QLabel(QStringLiteral("ĐANG TẮT [OFF]"));
    m_pumpStatusBadge->setStyleSheet("color: #ef4444; font-size: 9px; font-weight: 900; background: rgba(239, 68, 68, 0.15); border-radius: 4px; padding: 2px 6px;");

    auto *autoConfigBtn = new QPushButton(QStringLiteral("⚙ Ngưỡng"));
    autoConfigBtn->setCursor(Qt::PointingHandCursor);
    autoConfigBtn->setStyleSheet("background: #1e3a8a; color: #38bdf8; border: 1px solid #2563eb; border-radius: 4px; font-size: 9px; font-weight: 800; padding: 2px 6px;");
    connect(autoConfigBtn, &QPushButton::clicked, this, &DashboardPage::openPumpAutoConfig);

    auto *addDevBtn = new QPushButton(QStringLiteral("+ Thêm"));
    addDevBtn->setCursor(Qt::PointingHandCursor);
    addDevBtn->setStyleSheet("background: #065f46; color: #6ee7b7; border: 1px solid #059669; border-radius: 4px; font-size: 9px; font-weight: 800; padding: 2px 6px;");
    connect(addDevBtn, &QPushButton::clicked, this, &DashboardPage::addDeviceRequested);

    auto *unbindBtn = new QPushButton(QStringLiteral("✕ Gỡ"));
    unbindBtn->setCursor(Qt::PointingHandCursor);
    unbindBtn->setStyleSheet("background: #7f1d1d; color: #fca5a5; border: 1px solid #991b1b; border-radius: 4px; font-size: 9px; font-weight: 800; padding: 2px 6px;");
    connect(unbindBtn, &QPushButton::clicked, this, [this] {
        if (!m_deviceId.isEmpty()) {
            emit releaseDeviceRequested(m_deviceId);
        }
    });

    statusRow->addWidget(m_pumpDeviceNameLbl);
    statusRow->addWidget(m_pumpOnlineBadge);
    statusRow->addWidget(m_pumpStatusBadge);
    statusRow->addStretch();
    statusRow->addWidget(autoConfigBtn);
    statusRow->addWidget(addDevBtn);
    statusRow->addWidget(unbindBtn);
    pumpLayout->addLayout(statusRow);

    // Button Row: [Auto Toggle Switch] + [Manual Pump Toggle]
    auto *pumpBtnsRow = new QHBoxLayout;
    pumpBtnsRow->setSpacing(8);

    m_autoToggleButton = new QPushButton;
    m_autoToggleButton->setCursor(Qt::PointingHandCursor);
    m_autoToggleButton->setMinimumHeight(34);
    connect(m_autoToggleButton, &QPushButton::clicked, this, [this] {
        m_autoPumpMode = !m_autoPumpMode;

        QSettings settings(QStringLiteral("ICTU"), QStringLiteral("SonApp"));
        settings.setValue(QStringLiteral("auto_pump_mode"), m_autoPumpMode);

        const QJsonObject config{
            {"auto_mode", m_autoPumpMode},
            {"distance_start_cm", m_distanceStartCm},
            {"distance_stop_cm", m_distanceStopCm},
            {"sampling_interval_ms", 2000}
        };
        emit deviceConfigRequested(m_deviceId, config);
        updateDisplays();
    });

    m_pumpToggleButton = new QPushButton(QStringLiteral("BẬT MÁY BƠM"));
    m_pumpToggleButton->setCursor(Qt::PointingHandCursor);
    m_pumpToggleButton->setMinimumHeight(34);
    connect(m_pumpToggleButton, &QPushButton::clicked, this, [this] {
        const bool nextState = !m_pumpOn;
        emit relayControlRequested(m_deviceId, nextState);
    });

    pumpBtnsRow->addWidget(m_autoToggleButton, 1);
    pumpBtnsRow->addWidget(m_pumpToggleButton, 1);
    pumpLayout->addLayout(pumpBtnsRow);

    // Metrics summary inside pump card
    auto *metricsRow = new QHBoxLayout;
    metricsRow->setSpacing(6);

    auto *flowBox = new QFrame;
    flowBox->setStyleSheet("background: rgba(15, 23, 42, 0.6); border: 1px solid #1e293b; border-radius: 6px; padding: 2px;");
    auto *flowBoxL = new QVBoxLayout(flowBox);
    flowBoxL->setContentsMargins(4, 2, 4, 2);
    auto *fHead = new QLabel(QStringLiteral("LƯU LƯỢNG"));
    fHead->setStyleSheet("color: #64748b; font-size: 8px; font-weight: 700; border: none;");
    m_pumpFlowValue = new QLabel(QStringLiteral("0.00 L/min"));
    m_pumpFlowValue->setStyleSheet("color: #10b981; font-size: 11px; font-weight: 900; font-family: monospace; border: none;");
    flowBoxL->addWidget(fHead);
    flowBoxL->addWidget(m_pumpFlowValue);

    auto *totalBox = new QFrame;
    totalBox->setStyleSheet("background: rgba(15, 23, 42, 0.6); border: 1px solid #1e293b; border-radius: 6px; padding: 2px;");
    auto *totalBoxL = new QVBoxLayout(totalBox);
    totalBoxL->setContentsMargins(4, 2, 4, 2);
    auto *tHead = new QLabel(QStringLiteral("TỔNG NƯỚC"));
    tHead->setStyleSheet("color: #64748b; font-size: 8px; font-weight: 700; border: none;");
    m_pumpTotalValue = new QLabel(QStringLiteral("0.00 L"));
    m_pumpTotalValue->setStyleSheet("color: #38bdf8; font-size: 11px; font-weight: 900; font-family: monospace; border: none;");
    totalBoxL->addWidget(tHead);
    totalBoxL->addWidget(m_pumpTotalValue);

    metricsRow->addWidget(flowBox);
    metricsRow->addWidget(totalBox);
    pumpLayout->addLayout(metricsRow);

    m_pumpCardStack->addWidget(pumpCard);

    // Initial state: page 0 (No Device) if no device claimed yet
    m_pumpCardStack->setCurrentIndex(0);

    topRow->addWidget(m_pumpCardStack, 9);
    mainLayout->addLayout(topRow, 5);

    // ==========================================
    // ROW 2: 2 REALTIME CHARTS (Khoảng cách & Lưu lượng nước)
    // ==========================================
    auto *chartsRow = new QHBoxLayout;
    chartsRow->setSpacing(6);

    const QString detailBtnStyle =
        "QPushButton { background: #131f3f; color: #38bdf8; border: 1px solid #233565; border-radius: 4px; font-size: 10px; font-weight: 800; padding: 4px 8px; } "
        "QPushButton:hover { background: #1e3a8a; color: #ffffff; }";

    // --- 1. Distance Chart (HC-SR04) ---
    auto *distChartCard = createCard(QStringLiteral("Biểu đồ Khoảng cách (HC-SR04)"), QStringLiteral("📏"));
    auto *distChartLayout = static_cast<QVBoxLayout *>(distChartCard->layout());

    m_distanceSeries = new QLineSeries;
    m_distanceSeries->setPen(QPen(QColor(QStringLiteral("#38bdf8")), 2.4));
    auto *distChartView = buildChartView({m_distanceSeries}, &m_distanceAxisX, &m_distanceAxisY, 0.0, 50.0, QStringLiteral("cm"));
    distChartLayout->addWidget(distChartView, 1);

    auto *distDetailBtn = new QPushButton(QStringLiteral("⚙ Chi tiết & Chỉnh Ngưỡng (Bảng / Đồ thị)"));
    distDetailBtn->setStyleSheet(detailBtnStyle);
    distDetailBtn->setCursor(Qt::PointingHandCursor);
    connect(distDetailBtn, &QPushButton::clicked, this, [this] {
        openSensorDetail(QStringLiteral("Khoảng cách (HC-SR04)"), QStringLiteral("cm"), QStringLiteral("#38bdf8"), m_distanceHistory);
    });
    distChartLayout->addWidget(distDetailBtn);
    chartsRow->addWidget(distChartCard, 1);

    // --- 2. Water Flow Chart ---
    auto *flowChartCard = createCard(QStringLiteral("Biểu đồ Lưu lượng nước"), QStringLiteral("💧"));
    auto *flowChartLayout = static_cast<QVBoxLayout *>(flowChartCard->layout());

    m_flowSeries = new QLineSeries;
    m_flowSeries->setPen(QPen(QColor(QStringLiteral("#10b981")), 2.4));
    auto *flowChartView = buildChartView({m_flowSeries}, &m_flowAxisX, &m_flowAxisY, 0.0, 10.0, QStringLiteral("L/min"));
    flowChartLayout->addWidget(flowChartView, 1);

    auto *flowDetailBtn = new QPushButton(QStringLiteral("⚙ Chi tiết & Chỉnh Ngưỡng (Bảng / Đồ thị)"));
    flowDetailBtn->setStyleSheet(detailBtnStyle);
    flowDetailBtn->setCursor(Qt::PointingHandCursor);
    connect(flowDetailBtn, &QPushButton::clicked, this, [this] {
        openSensorDetail(QStringLiteral("Lưu lượng nước"), QStringLiteral("L/min"), QStringLiteral("#10b981"), m_flowHistory);
    });
    flowChartLayout->addWidget(flowDetailBtn);
    chartsRow->addWidget(flowChartCard, 1);

    mainLayout->addLayout(chartsRow, 5);
}

void DashboardPage::updateDisplays()
{
    if (m_heroTempValue)
        m_heroTempValue->setText(QStringLiteral("%1 °C").arg(m_currentTemp, 0, 'f', 1));
    if (m_heroHumidityValue)
        m_heroHumidityValue->setText(QStringLiteral("%1 %").arg(m_currentHumidity, 0, 'f', 1));
    if (m_heroPressureValue)
        m_heroPressureValue->setText(QStringLiteral("%1 mbar").arg(m_currentPressure, 0, 'f', 0));

    if (m_heroFanValue)
        m_heroFanValue->setText(m_pumpOn ? QStringLiteral("BƠM: ON") : QStringLiteral("BƠM: OFF"));

    // Update Pump Control Card
    if (m_pumpOnlineBadge) {
        m_pumpOnlineBadge->setText(m_isOnline ? QStringLiteral("🟢 ONLINE") : QStringLiteral("🔴 OFFLINE"));
        m_pumpOnlineBadge->setStyleSheet(m_isOnline
            ? "color: #10b981; font-size: 9px; font-weight: 900; background: rgba(16, 185, 129, 0.15); border-radius: 4px; padding: 2px 6px;"
            : "color: #ef4444; font-size: 9px; font-weight: 900; background: rgba(239, 68, 68, 0.15); border-radius: 4px; padding: 2px 6px;");
    }

    if (m_pumpStatusBadge) {
        m_pumpStatusBadge->setText(m_pumpOn ? QStringLiteral("ĐANG BẬT [ON]") : QStringLiteral("ĐANG TẮT [OFF]"));
        m_pumpStatusBadge->setStyleSheet(m_pumpOn
            ? "color: #10b981; font-size: 11px; font-weight: 900; background: rgba(16, 185, 129, 0.15); border-radius: 6px; padding: 2px 6px;"
            : "color: #ef4444; font-size: 11px; font-weight: 900; background: rgba(239, 68, 68, 0.15); border-radius: 6px; padding: 2px 6px;");
    }

    if (m_autoToggleButton) {
        m_autoToggleButton->setText(m_autoPumpMode ? QStringLiteral("🤖 AUTO: BẬT") : QStringLiteral("🤖 AUTO: TẮT"));
        m_autoToggleButton->setStyleSheet(m_autoPumpMode
            ? "QPushButton { background-color: #0284c7; color: #ffffff; border: 1.5px solid #38bdf8; border-radius: 17px; font-size: 11px; font-weight: 900; } QPushButton:hover { background-color: #0369a1; }"
            : "QPushButton { background-color: #1e293b; color: #94a3b8; border: 1px solid #334155; border-radius: 17px; font-size: 11px; font-weight: 900; } QPushButton:hover { background-color: #334155; color: #ffffff; }");
    }

    if (m_pumpToggleButton) {
        m_pumpToggleButton->setText(m_pumpOn ? QStringLiteral("TẮT MÁY BƠM") : QStringLiteral("BẬT MÁY BƠM"));
        m_pumpToggleButton->setStyleSheet(m_pumpOn
            ? "QPushButton { background-color: #ef4444; color: #ffffff; border: none; border-radius: 17px; font-size: 11px; font-weight: 900; } QPushButton:hover { background-color: #dc2626; }"
            : "QPushButton { background-color: #10b981; color: #ffffff; border: none; border-radius: 17px; font-size: 11px; font-weight: 900; } QPushButton:hover { background-color: #059669; }");
    }

    if (m_pumpFlowValue)
        m_pumpFlowValue->setText(QStringLiteral("%1 L/min").arg(m_currentFlow, 0, 'f', 2));

    if (m_pumpTotalValue)
        m_pumpTotalValue->setText(QStringLiteral("%1 L").arg(m_totalLiters, 0, 'f', 2));
}
