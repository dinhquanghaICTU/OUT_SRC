#include "DashboardPage.h"
#include "ui_DashboardPage.h"

#include "ui/widgets/CoolingSystemWidget.h"
#include "ui/dialogs/CoolingConfigDialog.h"

#include <QChart>
#include <QChartView>
#include <QDateTime>
#include <QFrame>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineSeries>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QValueAxis>
#include <QVBoxLayout>

namespace {
QFrame *createConsoleCard(const QString &objectName)
{
    auto *card = new QFrame;
    card->setObjectName(objectName);
    card->setStyleSheet(
        "QFrame#" + objectName + " { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0f192e, stop:1 #09101f); "
        "  border: 1.5px solid #1e293b; "
        "  border-radius: 12px; "
        "}"
    );
    return card;
}
}

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::DashboardPage)
{
    ui->setupUi(this);
    setObjectName(QStringLiteral("HoangMinhDashboardPage"));
    setStyleSheet("QWidget#HoangMinhDashboardPage { background-color: #060a14; }");

    setupUi();

    // Populate initial points for lively charts
    const QDateTime now = QDateTime::currentDateTime();
    for (int i = 20; i >= 0; --i) {
        const QDateTime pt = now.addSecs(-i * 2);
        const double t = 28.0 + (i % 3) * 0.4 + ((i % 2 == 0) ? 0.2 : -0.2);
        const double s = 0.12 + (i % 4) * 0.03;
        m_tempHistory.append({pt, t});
        m_soundHistory.append({pt, s});
        m_tempSeries->append(20 - i, t);
        m_soundSeries->append(20 - i, s);
    }
    m_tempAxisX->setRange(0, 20);
    m_soundAxisX->setRange(0, 20);

    updateDisplays();
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
    auto *rootLayout = ui->verticalLayout;
    rootLayout->setContentsMargins(8, 6, 8, 6);
    rootLayout->setSpacing(0);

    // Main 3-Column Command Cockpit Layout
    auto *cockpitLayout = new QHBoxLayout;
    cockpitLayout->setContentsMargins(0, 0, 0, 0);
    cockpitLayout->setSpacing(8);

    // =========================================================================
    // COLUMN 1: THÁP LÀM MÁT TUA-BIN (Cooling Turbine Tower)
    // =========================================================================
    auto *col1Card = createConsoleCard(QStringLiteral("col1TurbineCard"));
    auto *col1Layout = new QVBoxLayout(col1Card);
    col1Layout->setContentsMargins(10, 10, 10, 10);
    col1Layout->setSpacing(6);

    // Header Col 1
    auto *col1Header = new QHBoxLayout;
    auto *turbineIcon = new QLabel(QStringLiteral(""), col1Card);
    turbineIcon->setStyleSheet("font-size: 15px; color: #38bdf8;");
    col1Header->addWidget(turbineIcon);

    auto *col1Title = new QLabel(tr("THÁP TẢN NHIỆT"), col1Card);
    col1Title->setStyleSheet("font-size: 12px; font-weight: 900; color: #f8fafc; letter-spacing: 0.5px;");
    col1Header->addWidget(col1Title);
    col1Header->addStretch();

    m_fanStateBadge = new QLabel(tr("QUẠT: TẮT"), col1Card);
    m_fanStateBadge->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: 900; background: rgba(148, 163, 184, 0.15); border-radius: 4px; padding: 2px 6px;");
    col1Header->addWidget(m_fanStateBadge);
    col1Layout->addLayout(col1Header);

    // Dedicated Graphic Turbine Widget
    m_coolingWidget = new CoolingSystemWidget(col1Card);
    m_coolingWidget->setFixedHeight(175);
    col1Layout->addWidget(m_coolingWidget, 0, Qt::AlignCenter);

    // Digital Temperature Display Box
    auto *digitalBox = new QFrame(col1Card);
    digitalBox->setObjectName(QStringLiteral("col1DigitalBox"));
    digitalBox->setStyleSheet(QStringLiteral("QFrame#col1DigitalBox { background: rgba(15, 23, 42, 0.9); border: 1px solid #1e293b; border-radius: 8px; }"));
    auto *digitalLayout = new QVBoxLayout(digitalBox);
    digitalLayout->setContentsMargins(6, 4, 6, 4);
    digitalLayout->setSpacing(1);

    m_digitalTempLabel = new QLabel(QStringLiteral("28.5 °C"), digitalBox);
    m_digitalTempLabel->setAlignment(Qt::AlignCenter);
    m_digitalTempLabel->setStyleSheet("font-size: 22px; font-weight: 950; color: #38bdf8; font-family: 'DejaVu Sans', sans-serif; background: transparent; border: none;");
    digitalLayout->addWidget(m_digitalTempLabel);

    m_tempSubLabel = new QLabel(tr("CẢM BIẾN LM35 · ĐỘ CHÍNH XÁC CAO"), digitalBox);
    m_tempSubLabel->setAlignment(Qt::AlignCenter);
    m_tempSubLabel->setStyleSheet("font-size: 8px; color: #64748b; font-weight: 700; background: transparent; border: none;");
    digitalLayout->addWidget(m_tempSubLabel);
    col1Layout->addWidget(digitalBox);

    // Temperature Level Spectrum Chips
    auto *spectrumRow = new QHBoxLayout;
    spectrumRow->setSpacing(3);
    auto makeChip = [](const QString &txt, const QString &bg, const QString &fg) {
        auto *lbl = new QLabel(txt);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet(QStringLiteral("background: %1; color: %2; font-size: 8px; font-weight: 800; border-radius: 4px; padding: 3px 0px; border: none;").arg(bg, fg));
        return lbl;
    };
    spectrumRow->addWidget(makeChip(tr("< 28° Mát"), QStringLiteral("rgba(56, 189, 248, 0.15)"), QStringLiteral("#38bdf8")), 1);
    spectrumRow->addWidget(makeChip(tr("28-34° Ổn"), QStringLiteral("rgba(16, 185, 129, 0.15)"), QStringLiteral("#10b981")), 1);
    spectrumRow->addWidget(makeChip(tr("> 34° Nóng"), QStringLiteral("rgba(245, 158, 11, 0.15)"), QStringLiteral("#f59e0b")), 1);
    col1Layout->addLayout(spectrumRow);

    // Big Industrial Toggle Button
    m_fanActionBtn = new QPushButton(tr("BẬT QUẠT LÀM MÁT"), col1Card);
    m_fanActionBtn->setFixedHeight(38);
    m_fanActionBtn->setCursor(Qt::PointingHandCursor);
    connect(m_fanActionBtn, &QPushButton::clicked, this, [this] {
        const bool next = !m_fanOn;
        emit relayControlRequested(m_deviceId, next);
        m_fanOn = next;
        updateDisplays();
    });
    col1Layout->addWidget(m_fanActionBtn);

    cockpitLayout->addWidget(col1Card, 31);

    // =========================================================================
    // COLUMN 2: BẢNG ĐIỀU KHIỂN KHÍ HẬU (Climate Console & Automation Hub)
    // =========================================================================
    auto *col2Card = createConsoleCard(QStringLiteral("col2ControlCard"));
    auto *col2Layout = new QVBoxLayout(col2Card);
    col2Layout->setContentsMargins(10, 10, 10, 10);
    col2Layout->setSpacing(6);

    // Header Col 2
    auto *col2Header = new QHBoxLayout;
    auto *ctrlIcon = new QLabel(QStringLiteral(""), col2Card);
    ctrlIcon->setStyleSheet("font-size: 15px; color: #f59e0b; background: transparent; border: none;");
    col2Header->addWidget(ctrlIcon);

    m_stationTitle = new QLabel(tr("TRẠM LÀM MÁT"), col2Card);
    m_stationTitle->setStyleSheet("font-size: 12px; font-weight: 900; color: #f8fafc; background: transparent; border: none;");
    col2Header->addWidget(m_stationTitle);
    col2Header->addStretch();

    m_devIdPill = new QLabel(QStringLiteral("190782"), col2Card);
    m_devIdPill->setStyleSheet("color: #38bdf8; font-size: 10px; font-weight: 800; background: rgba(56, 189, 248, 0.15); border-radius: 4px; padding: 2px 5px; border: none;");
    col2Header->addWidget(m_devIdPill);

    m_onlinePill = new QLabel(tr("● Online"), col2Card);
    m_onlinePill->setStyleSheet("color: #10b981; font-size: 9px; font-weight: 800; background: rgba(16, 185, 129, 0.15); border-radius: 4px; padding: 2px 5px; border: none;");
    col2Header->addWidget(m_onlinePill);
    col2Layout->addLayout(col2Header);

    // Mode Selector Segmented Group
    auto *modeCard = new QFrame(col2Card);
    modeCard->setObjectName(QStringLiteral("col2ModeCard"));
    modeCard->setStyleSheet(QStringLiteral("QFrame#col2ModeCard { background: rgba(15, 23, 42, 0.7); border: 1px solid #1e293b; border-radius: 8px; }"));
    auto *modeLayout = new QVBoxLayout(modeCard);
    modeLayout->setContentsMargins(8, 6, 8, 6);
    modeLayout->setSpacing(4);

    auto *modeTitleRow = new QHBoxLayout;
    auto *modeHead = new QLabel(tr("CHẾ ĐỘ LÀM MÁT"), modeCard);
    modeHead->setStyleSheet("font-size: 9px; font-weight: 800; color: #64748b; background: transparent; border: none;");
    modeTitleRow->addWidget(modeHead);
    modeTitleRow->addStretch();
    modeLayout->addLayout(modeTitleRow);

    auto *modeBtnsRow = new QHBoxLayout;
    modeBtnsRow->setSpacing(6);
    m_modeAutoBtn = new QPushButton(tr("TỰ ĐỘNG"), modeCard);
    m_modeAutoBtn->setFixedHeight(30);
    m_modeAutoBtn->setCursor(Qt::PointingHandCursor);
    connect(m_modeAutoBtn, &QPushButton::clicked, this, [this] {
        m_autoCoolingMode = true;
        updateDisplays();
        checkAutoCoolingLogic();
    });
    modeBtnsRow->addWidget(m_modeAutoBtn, 1);

    m_modeManualBtn = new QPushButton(tr("THỦ CÔNG"), modeCard);
    m_modeManualBtn->setFixedHeight(30);
    m_modeManualBtn->setCursor(Qt::PointingHandCursor);
    connect(m_modeManualBtn, &QPushButton::clicked, this, [this] {
        m_autoCoolingMode = false;
        updateDisplays();
    });
    modeBtnsRow->addWidget(m_modeManualBtn, 1);
    modeLayout->addLayout(modeBtnsRow);
    col2Layout->addWidget(modeCard);

    // Auto Cooling Thresholds Panel
    auto *threshCard = new QFrame(col2Card);
    threshCard->setObjectName(QStringLiteral("col2ThreshCard"));
    threshCard->setStyleSheet(QStringLiteral("QFrame#col2ThreshCard { background: rgba(15, 23, 42, 0.7); border: 1px solid #1e293b; border-radius: 8px; }"));
    auto *threshLayout = new QVBoxLayout(threshCard);
    threshLayout->setContentsMargins(8, 6, 8, 6);
    threshLayout->setSpacing(4);

    auto *threshHead = new QLabel(tr("NGƯỠNG KÍCH HOẠT LÀM MÁT"), threshCard);
    threshHead->setStyleSheet("font-size: 9px; font-weight: 800; color: #64748b; background: transparent; border: none;");
    threshLayout->addWidget(threshHead);

    auto *threshRow1 = new QHBoxLayout;
    auto *startLbl = new QLabel(tr("Bật quạt khi nhiệt độ:"), threshCard);
    startLbl->setStyleSheet("font-size: 11px; color: #94a3b8; background: transparent; border: none;");
    m_threshStartText = new QLabel(QStringLiteral("≥ 34.0 °C"), threshCard);
    m_threshStartText->setStyleSheet("font-size: 12px; font-weight: 900; color: #f59e0b; background: transparent; border: none;");
    threshRow1->addWidget(startLbl);
    threshRow1->addStretch();
    threshRow1->addWidget(m_threshStartText);
    threshLayout->addLayout(threshRow1);

    auto *threshRow2 = new QHBoxLayout;
    auto *stopLbl = new QLabel(tr("Tắt quạt khi nhiệt độ:"), threshCard);
    stopLbl->setStyleSheet("font-size: 11px; color: #94a3b8; background: transparent; border: none;");
    m_threshStopText = new QLabel(QStringLiteral("≤ 28.0 °C"), threshCard);
    m_threshStopText->setStyleSheet("font-size: 12px; font-weight: 900; color: #10b981; background: transparent; border: none;");
    threshRow2->addWidget(stopLbl);
    threshRow2->addStretch();
    threshRow2->addWidget(m_threshStopText);
    threshLayout->addLayout(threshRow2);

    m_configBtn = new QPushButton(tr("Cài Đặt Ngưỡng Tự Động"), threshCard);
    m_configBtn->setFixedHeight(28);
    m_configBtn->setCursor(Qt::PointingHandCursor);
    m_configBtn->setStyleSheet("background: #1e293b; color: #38bdf8; border: 1px solid #334155; border-radius: 6px; font-size: 10px; font-weight: 700;");
    connect(m_configBtn, &QPushButton::clicked, this, &DashboardPage::openCoolingConfig);
    threshLayout->addWidget(m_configBtn);
    col2Layout->addWidget(threshCard);

    // Sound / Noise Level Monitor
    auto *soundCard = new QFrame(col2Card);
    soundCard->setObjectName(QStringLiteral("col2SoundCard"));
    soundCard->setStyleSheet(QStringLiteral("QFrame#col2SoundCard { background: rgba(15, 23, 42, 0.7); border: 1px solid #1e293b; border-radius: 8px; }"));
    auto *soundLayout = new QVBoxLayout(soundCard);
    soundLayout->setContentsMargins(8, 6, 8, 6);
    soundLayout->setSpacing(3);

    auto *soundHeadRow = new QHBoxLayout;
    auto *sndTitle = new QLabel(tr("ĐỘ ỒN (MAX9814)"), soundCard);
    sndTitle->setStyleSheet("font-size: 9px; font-weight: 800; color: #64748b; background: transparent; border: none;");
    m_soundValText = new QLabel(QStringLiteral("0.12 Vpp (Êm)"), soundCard);
    m_soundValText->setStyleSheet("font-size: 11px; font-weight: 900; color: #10b981; background: transparent; border: none;");
    soundHeadRow->addWidget(sndTitle);
    soundHeadRow->addStretch();
    soundHeadRow->addWidget(m_soundValText);
    soundLayout->addLayout(soundHeadRow);

    m_soundBar = new QProgressBar(soundCard);
    m_soundBar->setFixedHeight(8);
    m_soundBar->setRange(0, 100);
    m_soundBar->setValue(12);
    m_soundBar->setTextVisible(false);
    m_soundBar->setStyleSheet(
        "QProgressBar { background-color: #1e293b; border-radius: 4px; border: none; } "
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #10b981, stop:0.7 #f59e0b, stop:1 #ef4444); border-radius: 4px; }"
    );
    soundLayout->addWidget(m_soundBar);
    col2Layout->addWidget(soundCard);

    // Operating Summary Mini Row
    auto *statsRow = new QHBoxLayout;
    statsRow->setSpacing(6);

    auto *peakBox = new QFrame(col2Card);
    peakBox->setObjectName(QStringLiteral("col2PeakBox"));
    peakBox->setStyleSheet(QStringLiteral("QFrame#col2PeakBox { background: rgba(15, 23, 42, 0.7); border: 1px solid #1e293b; border-radius: 6px; }"));
    auto *peakLayout = new QVBoxLayout(peakBox);
    peakLayout->setContentsMargins(6, 4, 6, 4);
    auto *peakTitle = new QLabel(tr("NHIỆT ĐỘ ĐỈNH"), peakBox);
    peakTitle->setStyleSheet("font-size: 8px; color: #64748b; font-weight: 700; background: transparent; border: none;");
    m_peakValText = new QLabel(QStringLiteral("28.5 °C"), peakBox);
    m_peakValText->setStyleSheet("font-size: 11px; color: #f59e0b; font-weight: 900; background: transparent; border: none;");
    peakLayout->addWidget(peakTitle);
    peakLayout->addWidget(m_peakValText);
    statsRow->addWidget(peakBox, 1);

    auto *relayBox = new QFrame(col2Card);
    relayBox->setObjectName(QStringLiteral("col2RelayBox"));
    relayBox->setStyleSheet(QStringLiteral("QFrame#col2RelayBox { background: rgba(15, 23, 42, 0.7); border: 1px solid #1e293b; border-radius: 6px; }"));
    auto *relayLayout = new QVBoxLayout(relayBox);
    relayLayout->setContentsMargins(6, 4, 6, 4);
    auto *relayTitle = new QLabel(tr("TRẠNG THÁI RELAY"), relayBox);
    relayTitle->setStyleSheet("font-size: 8px; color: #64748b; font-weight: 700; background: transparent; border: none;");
    m_relayStateText = new QLabel(tr("NGẮT (OFF)"), relayBox);
    m_relayStateText->setStyleSheet("font-size: 11px; color: #94a3b8; font-weight: 900; background: transparent; border: none;");
    relayLayout->addWidget(relayTitle);
    relayLayout->addWidget(m_relayStateText);
    statsRow->addWidget(relayBox, 1);

    col2Layout->addLayout(statsRow);

    cockpitLayout->addWidget(col2Card, 34);

    // =========================================================================
    // COLUMN 3: TRỤC GIÁM SÁT REALTIME KÉP (Dual Telemetry Stream)
    // =========================================================================
    auto *col3Card = createConsoleCard(QStringLiteral("col3TelemetryCard"));
    auto *col3Layout = new QVBoxLayout(col3Card);
    col3Layout->setContentsMargins(10, 8, 10, 8);
    col3Layout->setSpacing(4);

    // Header Col 3
    auto *col3Header = new QHBoxLayout;
    auto *chartIcon = new QLabel(QStringLiteral(""), col3Card);
    chartIcon->setStyleSheet("font-size: 13px; background: transparent; border: none;");
    col3Header->addWidget(chartIcon);
    auto *col3Title = new QLabel(tr("TELEMETRY THỜI GIAN THỰC"), col3Card);
    col3Title->setStyleSheet("font-size: 11px; font-weight: 900; color: #f8fafc; background: transparent; border: none;");
    col3Header->addWidget(col3Title);
    col3Header->addStretch();
    col3Layout->addLayout(col3Header);

    // Chart 1: Temperature Line Chart (Upper half)
    auto *tempChartTitle = new QLabel(tr("NHIỆT ĐỘ LM35 (°C)"), col3Card);
    tempChartTitle->setStyleSheet("font-size: 10px; font-weight: 800; color: #38bdf8; margin-top: 2px; background: transparent; border: none;");
    col3Layout->addWidget(tempChartTitle);

    m_tempSeries = new QLineSeries(this);
    auto *tempChartView = buildChartView(m_tempSeries, &m_tempAxisX, &m_tempAxisY, 15.0, 50.0, QStringLiteral("°C"), QColor("#38bdf8"));
    col3Layout->addWidget(tempChartView, 1);

    // Chart 2: Sound Level Line Chart (Lower half)
    auto *soundChartTitle = new QLabel(tr("ĐỘ ỒN MAX9814 (Vpp)"), col3Card);
    soundChartTitle->setStyleSheet("font-size: 10px; font-weight: 800; color: #10b981; margin-top: 2px; background: transparent; border: none;");
    col3Layout->addWidget(soundChartTitle);

    m_soundSeries = new QLineSeries(this);
    auto *soundChartView = buildChartView(m_soundSeries, &m_soundAxisX, &m_soundAxisY, 0.0, 2.0, QStringLiteral("Vpp"), QColor("#10b981"));
    col3Layout->addWidget(soundChartView, 1);

    // Bottom Navigation Button
    auto *historyNavBtn = new QPushButton(tr("Mở Bảng Lịch Sử && Thống Kê"), col3Card);
    historyNavBtn->setFixedHeight(30);
    historyNavBtn->setCursor(Qt::PointingHandCursor);
    historyNavBtn->setStyleSheet(
        "QPushButton { background: #1e293b; color: #f8fafc; border: 1px solid #334155; border-radius: 6px; font-size: 11px; font-weight: 800; } "
        "QPushButton:hover { background: #334155; color: #38bdf8; }"
    );
    connect(historyNavBtn, &QPushButton::clicked, this, [this] {
        emit navigateToPageRequested(3);
    });
    col3Layout->addWidget(historyNavBtn);

    cockpitLayout->addWidget(col3Card, 35);

    rootLayout->addLayout(cockpitLayout);
}

QChartView *DashboardPage::buildChartView(QLineSeries *series, QValueAxis **axisX, QValueAxis **axisY,
                                          double minY, double maxY, const QString &unit, const QColor &color)
{
    auto *chart = new QChart;
    chart->setBackgroundVisible(false);
    chart->legend()->hide();
    chart->setMargins(QMargins(0, 0, 0, 0));

    QPen pen(color, 2.0);
    series->setPen(pen);
    chart->addSeries(series);

    *axisX = new QValueAxis(chart);
    (*axisX)->setRange(0, 20);
    (*axisX)->setTickCount(5);
    (*axisX)->setGridLineColor(QColor(30, 41, 59, 130));
    (*axisX)->setLabelsColor(QColor(148, 163, 184));
    (*axisX)->setLabelFormat(QStringLiteral("%d"));
    QFont axisFont;
    axisFont.setPixelSize(8);
    (*axisX)->setLabelsFont(axisFont);
    chart->addAxis(*axisX, Qt::AlignBottom);
    series->attachAxis(*axisX);

    *axisY = new QValueAxis(chart);
    (*axisY)->setRange(minY, maxY);
    (*axisY)->setTickCount(3);
    (*axisY)->setGridLineColor(QColor(30, 41, 59, 130));
    (*axisY)->setLabelsColor(QColor(148, 163, 184));
    (*axisY)->setTitleText(unit);
    (*axisY)->setTitleFont(axisFont);
    (*axisY)->setTitleBrush(QColor(148, 163, 184));
    (*axisY)->setLabelsFont(axisFont);
    (*axisY)->setLabelFormat(QStringLiteral("%.1f"));
    chart->addAxis(*axisY, Qt::AlignLeft);
    series->attachAxis(*axisY);

    auto *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    return view;
}

void DashboardPage::appendPoint(QLineSeries *series, QVector<HoangMinhDataPoint> &history, double val, QValueAxis *axisX, QValueAxis *axisY)
{
    const QDateTime now = QDateTime::currentDateTime();
    history.append({now, val});
    if (history.size() > 100)
        history.removeFirst();

    series->append(history.size() - 1, val);
    if (series->count() > 30)
        series->remove(0);

    QList<QPointF> pts;
    for (int i = 0; i < series->count(); ++i) {
        pts.append(QPointF(i, series->at(i).y()));
    }
    series->replace(pts);
    axisX->setRange(0, qMax(20, series->count() - 1));

    double minVal = val;
    double maxVal = val;
    for (const auto &p : pts) {
        minVal = qMin(minVal, p.y());
        maxVal = qMax(maxVal, p.y());
    }
    const double span = maxVal - minVal;
    const double pad = qMax(1.0, span * 0.2);
    axisY->setRange(qMax(0.0, minVal - pad), maxVal + pad);
}

void DashboardPage::updateReading(const SensorReading &reading)
{
    m_currentTemp = reading.temperatureC;
    m_currentSound = reading.soundVpp;
    m_fanOn = reading.fanOn;
    m_peakTemp = qMax(m_peakTemp, m_currentTemp);

    appendPoint(m_tempSeries, m_tempHistory, m_currentTemp, m_tempAxisX, m_tempAxisY);
    appendPoint(m_soundSeries, m_soundHistory, m_currentSound, m_soundAxisX, m_soundAxisY);

    updateDisplays();
    checkAutoCoolingLogic();
}

void DashboardPage::setDevices(const QJsonArray &devices)
{
    if (devices.isEmpty())
        return;

    const QJsonObject dev = devices.first().toObject();
    m_deviceId   = dev.value(QStringLiteral("device_id")).toString(QStringLiteral("190782"));
    m_deviceName = dev.value(QStringLiteral("name")).toString(tr("Trạm Làm Mát Tự Động"));
    m_isOnline   = dev.value(QStringLiteral("online")).toBool(true);

    // Load cooling thresholds từ config của thiết bị (nếu server trả về)
    const QJsonObject cfg = dev.value(QStringLiteral("config")).toObject();
    if (cfg.contains(QStringLiteral("fan_start_temp")))
        m_fanStartTemp = cfg.value(QStringLiteral("fan_start_temp")).toDouble(m_fanStartTemp);
    if (cfg.contains(QStringLiteral("fan_stop_temp")))
        m_fanStopTemp = cfg.value(QStringLiteral("fan_stop_temp")).toDouble(m_fanStopTemp);
    if (cfg.contains(QStringLiteral("max_sound_vpp")))
        m_maxSoundVpp = cfg.value(QStringLiteral("max_sound_vpp")).toDouble(m_maxSoundVpp);
    if (cfg.contains(QStringLiteral("sampling_interval_seconds")))
        m_sampleIntervalSec = cfg.value(QStringLiteral("sampling_interval_seconds")).toInt(m_sampleIntervalSec);

    const QJsonObject metrics = dev.value(QStringLiteral("metrics")).toObject();
    if (metrics.contains(QStringLiteral("temperature_c")))
        m_currentTemp = metrics.value(QStringLiteral("temperature_c")).toDouble();
    if (metrics.contains(QStringLiteral("sound_vpp")))
        m_currentSound = metrics.value(QStringLiteral("sound_vpp")).toDouble();
    if (metrics.contains(QStringLiteral("relay_state")))
        m_fanOn = metrics.value(QStringLiteral("relay_state")).toBool();

    m_peakTemp = qMax(m_peakTemp, m_currentTemp);

    appendPoint(m_tempSeries,  m_tempHistory,  m_currentTemp,  m_tempAxisX,  m_tempAxisY);
    appendPoint(m_soundSeries, m_soundHistory, m_currentSound, m_soundAxisX, m_soundAxisY);

    updateDisplays();
    checkAutoCoolingLogic();
}


void DashboardPage::checkAutoCoolingLogic()
{
    if (!m_autoCoolingMode)
        return;

    if (m_currentTemp >= m_fanStartTemp && !m_fanOn) {
        m_fanOn = true;
        emit relayControlRequested(m_deviceId, true);
        updateDisplays();
    } else if (m_currentTemp <= m_fanStopTemp && m_fanOn) {
        m_fanOn = false;
        emit relayControlRequested(m_deviceId, false);
        updateDisplays();
    }
}

void DashboardPage::updateDisplays()
{
    // 1. Column 1 (Cooling Turbine Tower)
    if (m_coolingWidget) {
        m_coolingWidget->setThreshold(m_fanStartTemp);
        m_coolingWidget->setTemperature(m_currentTemp);
        m_coolingWidget->setSoundVpp(m_currentSound);
        m_coolingWidget->setFanRunning(m_fanOn);
    }

    if (m_fanStateBadge) {
        m_fanStateBadge->setText(m_fanOn ? tr("QUẠT: ON") : tr("QUẠT: OFF"));
        m_fanStateBadge->setStyleSheet(m_fanOn
            ? "color: #06b6d4; font-size: 9px; font-weight: 900; background: rgba(6, 182, 212, 0.15); border: 1px solid rgba(6, 182, 212, 0.4); border-radius: 4px; padding: 2px 6px;"
            : "color: #94a3b8; font-size: 9px; font-weight: 900; background: rgba(148, 163, 184, 0.15); border: 1px solid rgba(148, 163, 184, 0.3); border-radius: 4px; padding: 2px 6px;");
    }

    if (m_digitalTempLabel) {
        m_digitalTempLabel->setText(QStringLiteral("%1 °C").arg(m_currentTemp, 0, 'f', 1));
        m_digitalTempLabel->setStyleSheet(m_currentTemp >= m_fanStartTemp
            ? "font-size: 22px; font-weight: 950; color: #f59e0b;"
            : "font-size: 22px; font-weight: 950; color: #38bdf8;");
    }

    if (m_fanActionBtn) {
        m_fanActionBtn->setText(m_fanOn ? tr("TẮT QUẠT LÀM MÁT") : tr("BẬT QUẠT LÀM MÁT"));
        m_fanActionBtn->setStyleSheet(m_fanOn
            ? "QPushButton { background-color: #ef4444; color: #ffffff; border: none; border-radius: 19px; font-size: 11px; font-weight: 900; } QPushButton:hover { background-color: #dc2626; }"
            : "QPushButton { background-color: #06b6d4; color: #0b1329; border: none; border-radius: 19px; font-size: 11px; font-weight: 900; } QPushButton:hover { background-color: #22d3ee; }");
    }

    // 2. Column 2 (Climate Console)
    if (m_devIdPill)
        m_devIdPill->setText(m_deviceId);

    if (m_onlinePill) {
        m_onlinePill->setText(m_isOnline ? tr("● Online") : tr("Offline"));
        m_onlinePill->setStyleSheet(m_isOnline
            ? "color: #10b981; font-size: 9px; font-weight: 800; background: rgba(16, 185, 129, 0.15); border-radius: 4px; padding: 2px 5px;"
            : "color: #ef4444; font-size: 9px; font-weight: 800; background: rgba(239, 68, 68, 0.15); border-radius: 4px; padding: 2px 5px;");
    }

    if (m_modeAutoBtn && m_modeManualBtn) {
        m_modeAutoBtn->setStyleSheet(m_autoCoolingMode
            ? "background-color: #0284c7; color: #ffffff; border: 1.5px solid #38bdf8; border-radius: 6px; font-size: 10px; font-weight: 900;"
            : "background-color: #1e293b; color: #94a3b8; border: 1px solid #334155; border-radius: 6px; font-size: 10px; font-weight: 700;");
        m_modeManualBtn->setStyleSheet(!m_autoCoolingMode
            ? "background-color: #0284c7; color: #ffffff; border: 1.5px solid #38bdf8; border-radius: 6px; font-size: 10px; font-weight: 900;"
            : "background-color: #1e293b; color: #94a3b8; border: 1px solid #334155; border-radius: 6px; font-size: 10px; font-weight: 700;");
    }

    if (m_threshStartText)
        m_threshStartText->setText(QStringLiteral("≥ %1 °C").arg(m_fanStartTemp, 0, 'f', 1));

    if (m_threshStopText)
        m_threshStopText->setText(QStringLiteral("≤ %1 °C").arg(m_fanStopTemp, 0, 'f', 1));

    if (m_soundValText) {
        QString soundDesc = tr("Êm");
        if (m_currentSound > 1.2) soundDesc = tr("ỒN!");
        else if (m_currentSound > 0.6) soundDesc = tr("Vừa");
        m_soundValText->setText(QStringLiteral("%1 Vpp (%2)").arg(m_currentSound, 0, 'f', 2).arg(soundDesc));
    }

    if (m_soundBar) {
        const int pct = qBound(0, int((m_currentSound / 2.0) * 100.0), 100);
        m_soundBar->setValue(pct);
    }

    if (m_peakValText)
        m_peakValText->setText(QStringLiteral("%1 °C").arg(m_peakTemp, 0, 'f', 1));

    if (m_relayStateText) {
        m_relayStateText->setText(m_fanOn ? tr("ĐÓNG (ON)") : tr("NGẮT (OFF)"));
        m_relayStateText->setStyleSheet(m_fanOn
            ? "font-size: 11px; color: #06b6d4; font-weight: 900;"
            : "font-size: 11px; color: #94a3b8; font-weight: 900;");
    }
}

void DashboardPage::openCoolingConfig()
{
    QJsonObject cfg{
        {QStringLiteral("fan_start_temp"), m_fanStartTemp},
        {QStringLiteral("fan_stop_temp"), m_fanStopTemp},
        {QStringLiteral("max_sound_vpp"), m_maxSoundVpp},
        {QStringLiteral("sampling_interval_seconds"), m_sampleIntervalSec}
    };

    CoolingConfigDialog dlg(cfg, this);
    if (dlg.exec() == QDialog::Accepted) {
        const QJsonObject newCfg = dlg.configData();
        m_fanStartTemp = newCfg.value(QStringLiteral("fan_start_temp")).toDouble(34.0);
        m_fanStopTemp = newCfg.value(QStringLiteral("fan_stop_temp")).toDouble(28.0);
        m_maxSoundVpp = newCfg.value(QStringLiteral("max_sound_vpp")).toDouble(1.5);
        m_sampleIntervalSec = newCfg.value(QStringLiteral("sampling_interval_seconds")).toInt(2);

        updateDisplays();
        checkAutoCoolingLogic();

        emit deviceConfigRequested(m_deviceId, newCfg);
    }
}
