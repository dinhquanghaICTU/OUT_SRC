#include "DashboardPage.h"
#include "ui_DashboardPage.h"

#include <QChart>
#include <QChartView>
#include <QDate>
#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineSeries>
#include <QLocale>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScroller>
#include <QStyle>
#include <QTimer>
#include <QValueAxis>
#include <QVBoxLayout>

namespace {

QFrame *makeCard(const char *objectName = "stationCard")
{
    auto *card = new QFrame;
    card->setObjectName(QString::fromLatin1(objectName));
    card->setFrameShape(QFrame::NoFrame);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    return card;
}

QLabel *makeLabel(const QString &text, const char *objectName = nullptr, bool bold = false)
{
    auto *lbl = new QLabel(text);
    if (objectName)
        lbl->setObjectName(QString::fromLatin1(objectName));
    lbl->setWordWrap(true);
    if (bold) {
        QFont f = lbl->font();
        f.setBold(true);
        lbl->setFont(f);
    }
    return lbl;
}

} // namespace

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::DashboardPage)
{
    ui->setupUi(this);
    setupUiCustom();
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

void DashboardPage::setUsername(const QString &username)
{
    m_username = username;
}

void DashboardPage::setupUiCustom()
{
    setStyleSheet(QStringLiteral(
        "QWidget#DashboardPage { background-color: #060b18; color: #ecf2ff; font-family: sans-serif; } "
        "QFrame#stationCard { background-color: #0d1733; border: 1px solid #1c2b54; border-radius: 8px; } "
        "QLabel#stationTitle { color: #38bdf8; font-size: 14px; font-weight: 900; } "
        "QLabel#stationSubtitle { color: #94a3b8; font-size: 10px; font-weight: 600; } "
        "QLabel#badgeOnline { background-color: rgba(16, 185, 129, 0.2); color: #34d399; border: 1px solid #059669; border-radius: 4px; padding: 2px 8px; font-size: 10px; font-weight: 800; } "
        "QLabel#badgeOffline { background-color: rgba(100, 116, 139, 0.2); color: #94a3b8; border: 1px solid #475569; border-radius: 4px; padding: 2px 8px; font-size: 10px; font-weight: 800; } "
        "QLabel#cardHeader { color: #94a3b8; font-size: 10px; font-weight: 800; letter-spacing: 0.5px; text-transform: uppercase; } "
        "QLabel#bigUvValue { color: #f59e0b; font-size: 26px; font-weight: 900; font-family: monospace; } "
        "QLabel#bigPressureValue { color: #38bdf8; font-size: 26px; font-weight: 900; font-family: monospace; } "
        "QLabel#subMetricLabel { color: #94a3b8; font-size: 9px; font-weight: 600; } "
        "QLabel#subMetricValue { color: #ffffff; font-size: 11px; font-weight: 800; } "
        "QLabel#adviceText { color: #cbd5e1; font-size: 10px; line-height: 1.3; } "
        "QLabel#uvBadgeLow { background-color: rgba(16, 185, 129, 0.2); color: #34d399; border: 1px solid #059669; border-radius: 4px; padding: 2px 6px; font-size: 9px; font-weight: 800; } "
        "QLabel#uvBadgeMod { background-color: rgba(234, 179, 8, 0.2); color: #fde047; border: 1px solid #ca8a04; border-radius: 4px; padding: 2px 6px; font-size: 9px; font-weight: 800; } "
        "QLabel#uvBadgeHigh { background-color: rgba(249, 115, 22, 0.2); color: #fb923c; border: 1px solid #ea580c; border-radius: 4px; padding: 2px 6px; font-size: 9px; font-weight: 800; } "
        "QLabel#uvBadgeVeryHigh { background-color: rgba(239, 68, 68, 0.2); color: #f87171; border: 1px solid #dc2626; border-radius: 4px; padding: 2px 6px; font-size: 9px; font-weight: 800; } "
        "QLabel#uvBadgeExtreme { background-color: rgba(168, 85, 247, 0.25); color: #c084fc; border: 1px solid #9333ea; border-radius: 4px; padding: 2px 6px; font-size: 9px; font-weight: 800; } "
        "QProgressBar#uvRiskBar { background-color: #111d3d; border: 1px solid #1c2b54; border-radius: 3px; max-height: 6px; text-align: center; } "
        "QProgressBar#uvRiskBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #10b981, stop:0.25 #eab308, stop:0.55 #f97316, stop:0.8 #ef4444, stop:1.0 #a855f7); border-radius: 2px; } "
        "QPushButton#chartTab { background: #0e1938; color: #94a3b8; border: 1px solid #223565; border-radius: 4px; padding: 2px 8px; font-size: 9px; font-weight: 800; } "
        "QPushButton#chartTab:hover { background: #172554; color: #ffffff; } "
        "QPushButton#chartTabActive { background: #0284c7; color: #ffffff; border: 1px solid #38bdf8; border-radius: 4px; padding: 2px 8px; font-size: 9px; font-weight: 900; } "
        "QPushButton#actionButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0284c7, stop:1 #0369a1); color: #ffffff; border: 1px solid #38bdf8; border-radius: 6px; padding: 6px 12px; font-size: 10px; font-weight: 900; } "
        "QPushButton#actionButton:hover { background: #0284c7; } "
        "QPushButton#relayButtonOn { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #dc2626, stop:1 #b91c1c); color: #ffffff; border: 1px solid #f87171; border-radius: 6px; padding: 6px 12px; font-size: 10px; font-weight: 900; } "
        "QPushButton#relayButtonOff { background: #111d3d; color: #94a3b8; border: 1px solid #1c2b54; border-radius: 6px; padding: 6px 12px; font-size: 10px; font-weight: 800; } "
        "QPushButton#relayButtonOff:hover { background: #1e293b; color: #ffffff; }"
    ));

    ui->verticalLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(QStringLiteral(
        "QScrollArea { background: transparent; border: none; } "
        "QScrollBar:vertical { background: #060b18; width: 6px; border-radius: 3px; } "
        "QScrollBar::handle:vertical { background: #1e3a8a; border-radius: 3px; min-height: 20px; } "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    ));
    QScroller::grabGesture(scrollArea->viewport(), QScroller::LeftMouseButtonGesture);

    auto *container = new QWidget(scrollArea);
    container->setObjectName(QStringLiteral("dashboardContainer"));
    auto *contentLayout = new QVBoxLayout(container);
    contentLayout->setContentsMargins(8, 6, 8, 6);
    contentLayout->setSpacing(8);

    // 1. TOP HEADER BAR
    auto *headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);

    auto *titleCol = new QVBoxLayout;
    titleCol->setContentsMargins(0, 0, 0, 0);
    titleCol->setSpacing(2);
    m_stationNameLabel = makeLabel(tr("TRẠM ĐO UV & ÁP SUẤT KHÍ QUYỂN"), "stationTitle", true);
    m_stationNameLabel->setWordWrap(false);
    auto *authorSubtitle = makeLabel(tr("Hệ thống IoT giám sát môi trường · SVTH: Trung Kiên - Khoa ĐTVT (ICTU)"), "stationSubtitle");
    authorSubtitle->setWordWrap(false);
    titleCol->addWidget(m_stationNameLabel);
    titleCol->addWidget(authorSubtitle);
    headerLayout->addLayout(titleCol);
    headerLayout->addStretch();

    m_statusBadge = makeLabel(tr("○ Ngoại tuyến"), "badgeOffline", true);
    m_statusBadge->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(m_statusBadge, 0, Qt::AlignVCenter);

    m_lastUpdatedLabel = makeLabel(tr("Chưa có dữ liệu"), "subMetricLabel");
    m_lastUpdatedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    headerLayout->addWidget(m_lastUpdatedLabel, 0, Qt::AlignVCenter);

    contentLayout->addLayout(headerLayout);

    // 2. MAIN 2-COLUMN GRID (4 Cards)
    m_mainGrid = new QGridLayout;
    m_mainGrid->setContentsMargins(0, 0, 0, 0);
    m_mainGrid->setHorizontalSpacing(8);
    m_mainGrid->setVerticalSpacing(8);
    m_mainGrid->setColumnStretch(0, 1);
    m_mainGrid->setColumnStretch(1, 1);

    // --- CARD 1: CƯỜNG ĐỘ TIA UV ---
    m_cardUv = makeCard("stationCard");
    auto *uvLayout = new QVBoxLayout(m_cardUv);
    uvLayout->setContentsMargins(10, 8, 10, 8);
    uvLayout->setSpacing(6);

    auto *uvTopRow = new QHBoxLayout;
    uvTopRow->addWidget(makeLabel(tr("☀️ CƯỜNG ĐỘ TIA UV (UV INDEX)"), "cardHeader", true));
    uvTopRow->addStretch();
    m_uvRiskBadge = makeLabel(tr("Thấp (An toàn)"), "uvBadgeLow", true);
    uvTopRow->addWidget(m_uvRiskBadge);
    uvLayout->addLayout(uvTopRow);

    auto *uvValRow = new QHBoxLayout;
    m_uvValueLabel = makeLabel(QStringLiteral("--"), "bigUvValue", true);
    uvValRow->addWidget(m_uvValueLabel);
    uvValRow->addWidget(makeLabel(tr("UVI"), "subMetricLabel"), 0, Qt::AlignBottom);
    uvValRow->addStretch();

    auto *uvSubCol = new QVBoxLayout;
    uvSubCol->setSpacing(2);
    auto *uvVRow = new QHBoxLayout;
    uvVRow->addWidget(makeLabel(tr("Điện áp cảm biến:"), "subMetricLabel"));
    m_uvVoltageLabel = makeLabel(QStringLiteral("-- V"), "subMetricValue", true);
    uvVRow->addWidget(m_uvVoltageLabel);
    uvSubCol->addLayout(uvVRow);

    auto *uvTypeRow = new QHBoxLayout;
    uvTypeRow->addWidget(makeLabel(tr("Cảm biến:"), "subMetricLabel"));
    uvTypeRow->addWidget(makeLabel(QStringLiteral("GY-ML8511"), "subMetricValue", true));
    uvSubCol->addLayout(uvTypeRow);
    uvValRow->addLayout(uvSubCol);
    uvLayout->addLayout(uvValRow);

    // Risk bar
    m_uvRiskBar = new QProgressBar(m_cardUv);
    m_uvRiskBar->setObjectName(QStringLiteral("uvRiskBar"));
    m_uvRiskBar->setRange(0, 120); // 0 to 12.0
    m_uvRiskBar->setValue(0);
    m_uvRiskBar->setTextVisible(false);
    uvLayout->addWidget(m_uvRiskBar);

    m_uvAdviceLabel = makeLabel(tr("💡 Mức độ bức xạ an toàn cho các hoạt động ngoài trời."), "adviceText");
    uvLayout->addWidget(m_uvAdviceLabel);

    m_mainGrid->addWidget(m_cardUv, 0, 0);

    // --- CARD 2: ÁP SUẤT KHÍ QUYỂN ---
    m_cardPressure = makeCard("stationCard");
    auto *presLayout = new QVBoxLayout(m_cardPressure);
    presLayout->setContentsMargins(10, 8, 10, 8);
    presLayout->setSpacing(6);

    auto *presTopRow = new QHBoxLayout;
    presTopRow->addWidget(makeLabel(tr("🧭 ÁP SUẤT KHÍ QUYỂN (BMP180)"), "cardHeader", true));
    presTopRow->addStretch();
    m_weatherForecastBadge = makeLabel(tr("☀️ Ổn định / Khô ráo"), "uvBadgeLow", true);
    presTopRow->addWidget(m_weatherForecastBadge);
    presLayout->addLayout(presTopRow);

    auto *presValRow = new QHBoxLayout;
    m_pressureValueLabel = makeLabel(QStringLiteral("--"), "bigPressureValue", true);
    presValRow->addWidget(m_pressureValueLabel);
    presValRow->addWidget(makeLabel(tr("hPa"), "subMetricLabel"), 0, Qt::AlignBottom);
    presValRow->addStretch();

    auto *presSubCol = new QVBoxLayout;
    presSubCol->setSpacing(2);
    auto *mmHgRow = new QHBoxLayout;
    mmHgRow->addWidget(makeLabel(tr("Quy đổi:"), "subMetricLabel"));
    m_pressureMmHgLabel = makeLabel(QStringLiteral("-- mmHg"), "subMetricValue", true);
    mmHgRow->addWidget(m_pressureMmHgLabel);
    presSubCol->addLayout(mmHgRow);

    auto *altRow = new QHBoxLayout;
    altRow->addWidget(makeLabel(tr("Độ cao ước tính:"), "subMetricLabel"));
    m_altitudeLabel = makeLabel(QStringLiteral("-- m"), "subMetricValue");
    altRow->addWidget(m_altitudeLabel);
    presSubCol->addLayout(altRow);
    presValRow->addLayout(presSubCol);
    presLayout->addLayout(presValRow);

    m_weatherAdviceLabel = makeLabel(tr("Khí quyển duy trì ổn định, thời tiết thuận lợi cho các hoạt động ngoài trời."), "adviceText");
    presLayout->addWidget(m_weatherAdviceLabel);

    m_mainGrid->addWidget(m_cardPressure, 0, 1);

    // --- CARD 3: BIỂU ĐỒ DIỄN BIẾN THỜI GIAN THỰC ---
    m_cardChart = makeCard("stationCard");
    auto *chartCardLayout = new QVBoxLayout(m_cardChart);
    chartCardLayout->setContentsMargins(10, 8, 10, 8);
    chartCardLayout->setSpacing(4);

    auto *chartHeaderRow = new QHBoxLayout;
    chartHeaderRow->addWidget(makeLabel(tr("📈 DIỄN BIẾN THỜI GIAN THỰC"), "cardHeader", true));
    chartHeaderRow->addStretch();

    m_chartFilterAll = new QPushButton(tr("Tất cả"), m_cardChart);
    m_chartFilterAll->setObjectName(QStringLiteral("chartTabActive"));
    m_chartFilterUv = new QPushButton(tr("Tia UV"), m_cardChart);
    m_chartFilterUv->setObjectName(QStringLiteral("chartTab"));
    m_chartFilterPres = new QPushButton(tr("Áp suất"), m_cardChart);
    m_chartFilterPres->setObjectName(QStringLiteral("chartTab"));

    const auto setChartFilter = [this](int mode) {
        m_chartMode = mode;
        m_chartFilterAll->setObjectName(mode == 0 ? QStringLiteral("chartTabActive") : QStringLiteral("chartTab"));
        m_chartFilterUv->setObjectName(mode == 1 ? QStringLiteral("chartTabActive") : QStringLiteral("chartTab"));
        m_chartFilterPres->setObjectName(mode == 2 ? QStringLiteral("chartTabActive") : QStringLiteral("chartTab"));
        for (auto *b : {m_chartFilterAll, m_chartFilterUv, m_chartFilterPres}) {
            b->style()->unpolish(b);
            b->style()->polish(b);
        }
        if (m_uvSeries) m_uvSeries->setVisible(mode == 0 || mode == 1);
        if (m_pressureSeries) m_pressureSeries->setVisible(mode == 0 || mode == 2);
        if (m_axisY_Uv) m_axisY_Uv->setVisible(mode == 0 || mode == 1);
        if (m_axisY_Pres) m_axisY_Pres->setVisible(mode == 0 || mode == 2);
    };

    connect(m_chartFilterAll, &QPushButton::clicked, this, [=] { setChartFilter(0); });
    connect(m_chartFilterUv, &QPushButton::clicked, this, [=] { setChartFilter(1); });
    connect(m_chartFilterPres, &QPushButton::clicked, this, [=] { setChartFilter(2); });

    chartHeaderRow->addWidget(m_chartFilterAll);
    chartHeaderRow->addWidget(m_chartFilterUv);
    chartHeaderRow->addWidget(m_chartFilterPres);
    chartCardLayout->addLayout(chartHeaderRow);

    m_chart = new QChart;
    m_chart->legend()->hide();
    m_chart->setBackgroundVisible(false);
    m_chart->setMargins(QMargins(0, 0, 0, 0));

    m_uvSeries = new QLineSeries(this);
    m_uvSeries->setPen(QPen(QColor("#f59e0b"), 2.2));
    m_pressureSeries = new QLineSeries(this);
    m_pressureSeries->setPen(QPen(QColor("#38bdf8"), 2.2));

    m_chart->addSeries(m_uvSeries);
    m_chart->addSeries(m_pressureSeries);

    m_axisX = new QValueAxis(m_chart);
    m_axisX->setRange(0, 20);
    m_axisX->setTickCount(5);
    m_axisX->setLabelFormat(QStringLiteral("%d"));
    m_axisX->setGridLineColor(QColor("#1c2b54"));
    m_axisX->setLabelsColor(QColor("#64748b"));
    QFont axisF;
    axisF.setPixelSize(8);
    m_axisX->setLabelsFont(axisF);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_uvSeries->attachAxis(m_axisX);
    m_pressureSeries->attachAxis(m_axisX);

    m_axisY_Uv = new QValueAxis(m_chart);
    m_axisY_Uv->setRange(0, 12);
    m_axisY_Uv->setTickCount(4);
    m_axisY_Uv->setLabelFormat(QStringLiteral("%.1f"));
    m_axisY_Uv->setLabelsColor(QColor("#f59e0b"));
    m_axisY_Uv->setLabelsFont(axisF);
    m_axisY_Uv->setGridLineColor(QColor("#1c2b54"));
    m_chart->addAxis(m_axisY_Uv, Qt::AlignRight);
    m_uvSeries->attachAxis(m_axisY_Uv);

    m_axisY_Pres = new QValueAxis(m_chart);
    m_axisY_Pres->setRange(950, 1050);
    m_axisY_Pres->setTickCount(4);
    m_axisY_Pres->setLabelFormat(QStringLiteral("%.0f"));
    m_axisY_Pres->setLabelsColor(QColor("#38bdf8"));
    m_axisY_Pres->setLabelsFont(axisF);
    m_axisY_Pres->setGridLineColor(QColor("#1c2b54"));
    m_chart->addAxis(m_axisY_Pres, Qt::AlignLeft);
    m_pressureSeries->attachAxis(m_axisY_Pres);

    m_chartView = new QChartView(m_chart, m_cardChart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(80);
    m_chartView->setMaximumHeight(100);
    m_chartView->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    chartCardLayout->addWidget(m_chartView);

    m_mainGrid->addWidget(m_cardChart, 1, 0);

    // --- CARD 4: TRẠM ĐO & ĐIỀU KHIỂN CẢNH BÁO ---
    m_cardStation = makeCard("stationCard");
    auto *stLayout = new QVBoxLayout(m_cardStation);
    stLayout->setContentsMargins(10, 8, 10, 8);
    stLayout->setSpacing(6);

    auto *stHeaderRow = new QHBoxLayout;
    stHeaderRow->addWidget(makeLabel(tr("⚡ TRẠM THIẾT BỊ & ĐIỀU KHIỂN"), "cardHeader", true));
    stLayout->addLayout(stHeaderRow);

    auto *infoGrid = new QGridLayout;
    infoGrid->setContentsMargins(0, 0, 0, 0);
    infoGrid->setHorizontalSpacing(6);
    infoGrid->setVerticalSpacing(3);

    infoGrid->addWidget(makeLabel(tr("Mã trạm:"), "subMetricLabel"), 0, 0);
    m_stationIdLabel = makeLabel(QStringLiteral("Trungkien-150304"), "subMetricValue", true);
    infoGrid->addWidget(m_stationIdLabel, 0, 1);

    infoGrid->addWidget(makeLabel(tr("Cảm biến:"), "subMetricLabel"), 1, 0);
    m_sensorTypeLabel = makeLabel(QStringLiteral("GY-ML8511 & BMP180"), "subMetricValue");
    infoGrid->addWidget(m_sensorTypeLabel, 1, 1);

    infoGrid->addWidget(makeLabel(tr("Giao thức:"), "subMetricLabel"), 2, 0);
    infoGrid->addWidget(makeLabel(QStringLiteral("MQTT / WiFi · 2s/mẫu"), "subMetricValue"), 2, 1);
    stLayout->addLayout(infoGrid);

    stLayout->addStretch();

    auto *ctrlRow = new QHBoxLayout;
    ctrlRow->setSpacing(6);

    m_relayButton = new QPushButton(tr("🔔 CÒI CẢNH BÁO: TẮT"), m_cardStation);
    m_relayButton->setObjectName(QStringLiteral("relayButtonOff"));
    m_relayButton->setCursor(Qt::PointingHandCursor);
    connect(m_relayButton, &QPushButton::clicked, this, [this] {
        m_relayActive = !m_relayActive;
        m_relayButton->setObjectName(m_relayActive ? QStringLiteral("relayButtonOn") : QStringLiteral("relayButtonOff"));
        m_relayButton->setText(m_relayActive ? tr("🔔 CÒI CẢNH BÁO: BẬT") : tr("🔔 CÒI CẢNH BÁO: TẮT"));
        m_relayButton->style()->unpolish(m_relayButton);
        m_relayButton->style()->polish(m_relayButton);
        emit relayToggleRequested(m_activeDeviceId, m_relayActive);
    });
    ctrlRow->addWidget(m_relayButton, 1);

    m_viewHistoryButton = new QPushButton(tr("📊 LỊCH SỬ ĐO"), m_cardStation);
    m_viewHistoryButton->setObjectName(QStringLiteral("actionButton"));
    m_viewHistoryButton->setCursor(Qt::PointingHandCursor);
    connect(m_viewHistoryButton, &QPushButton::clicked, this, &DashboardPage::historyPageRequested);
    ctrlRow->addWidget(m_viewHistoryButton, 1);

    stLayout->addLayout(ctrlRow);

    m_mainGrid->addWidget(m_cardStation, 1, 1);
    m_mainGrid->setRowStretch(0, 1);
    m_mainGrid->setRowStretch(1, 1);

    contentLayout->addLayout(m_mainGrid, 1);

    scrollArea->setWidget(container);
    ui->verticalLayout->addWidget(scrollArea);
    applyResponsiveLayout();
}
void DashboardPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    applyResponsiveLayout();
}

void DashboardPage::applyResponsiveLayout()
{
    if (!m_mainGrid)
        return;

    while (QLayoutItem *item = m_mainGrid->takeAt(0))
        delete item;

    const bool compact = width() < 500;
    if (compact) {
        m_mainGrid->addWidget(m_cardUv, 0, 0);
        m_mainGrid->addWidget(m_cardPressure, 1, 0);
        m_mainGrid->addWidget(m_cardChart, 2, 0);
        m_mainGrid->addWidget(m_cardStation, 3, 0);
        m_mainGrid->setColumnStretch(0, 1);
        m_mainGrid->setColumnStretch(1, 0);
        m_mainGrid->setRowStretch(0, 1);
        m_mainGrid->setRowStretch(1, 1);
        m_mainGrid->setRowStretch(2, 1);
        m_mainGrid->setRowStretch(3, 1);
    } else {
        m_mainGrid->addWidget(m_cardUv, 0, 0);
        m_mainGrid->addWidget(m_cardPressure, 0, 1);
        m_mainGrid->addWidget(m_cardChart, 1, 0);
        m_mainGrid->addWidget(m_cardStation, 1, 1);
        m_mainGrid->setColumnStretch(0, 1);
        m_mainGrid->setColumnStretch(1, 1);
        m_mainGrid->setRowStretch(0, 1);
        m_mainGrid->setRowStretch(1, 1);
    }
}

void DashboardPage::updateUvCard(double uvIndex, double uvVoltage)
{
    m_uvValueLabel->setText(QString::number(uvIndex, 'f', 2));
    m_uvVoltageLabel->setText(QStringLiteral("%1 V").arg(uvVoltage, 0, 'f', 3));
    m_uvRiskBar->setValue(qBound(0, static_cast<int>(uvIndex * 10.0), 120));

    if (uvIndex < 3.0) {
        m_uvRiskBadge->setText(tr("Thấp (An toàn)"));
        m_uvRiskBadge->setObjectName(QStringLiteral("uvBadgeLow"));
        m_uvAdviceLabel->setText(tr("💡 Mức độ bức xạ an toàn cho các hoạt động ngoài trời."));
    } else if (uvIndex < 6.0) {
        m_uvRiskBadge->setText(tr("Trung bình"));
        m_uvRiskBadge->setObjectName(QStringLiteral("uvBadgeMod"));
        m_uvAdviceLabel->setText(tr("⚠️ Nên đeo kính râm và che chắn khi ra ngoài nắng gắt."));
    } else if (uvIndex < 8.0) {
        m_uvRiskBadge->setText(tr("Cao (Nguy hại)"));
        m_uvRiskBadge->setObjectName(QStringLiteral("uvBadgeHigh"));
        m_uvAdviceLabel->setText(tr("🚨 Cần bôi kem chống nắng, mặc áo dài tay và đeo kính bảo hộ."));
    } else if (uvIndex < 11.0) {
        m_uvRiskBadge->setText(tr("Rất cao (Nguy hiểm)"));
        m_uvRiskBadge->setObjectName(QStringLiteral("uvBadgeVeryHigh"));
        m_uvAdviceLabel->setText(tr("⛔ Hạn chế ra ngoài từ 10h-16h. Da và mắt có nguy cơ tổn thương nhanh!"));
    } else {
        m_uvRiskBadge->setText(tr("Cực độ (Báo động)"));
        m_uvRiskBadge->setObjectName(QStringLiteral("uvBadgeExtreme"));
        m_uvAdviceLabel->setText(tr("☠️ Nguy cơ bỏng da trong vài phút! Khuyến cáo ở trong nhà."));
    }
    m_uvRiskBadge->style()->unpolish(m_uvRiskBadge);
    m_uvRiskBadge->style()->polish(m_uvRiskBadge);
}

void DashboardPage::updatePressureCard(double pressureHpa, double tempC)
{
    Q_UNUSED(tempC);
    m_pressureValueLabel->setText(QString::number(pressureHpa, 'f', 1));
    const double mmHg = pressureHpa * 0.750062;
    m_pressureMmHgLabel->setText(QStringLiteral("%1 mmHg").arg(mmHg, 0, 'f', 1));

    double alt = 0.0;
    if (pressureHpa > 0.0) {
        alt = 44330.0 * (1.0 - std::pow(pressureHpa / 1013.25, 0.190295));
        if (alt < -200.0) alt = 0.0;
    }
    m_altitudeLabel->setText(QStringLiteral("~%1 m").arg(qRound(alt)));

    if (pressureHpa >= 1016.0) {
        m_weatherForecastBadge->setText(tr("☀️ Nắng ráo / Áp cao"));
        m_weatherAdviceLabel->setText(tr("Trời quang đãng, áp cao khô ráo, tầm nhìn tốt."));
    } else if (pressureHpa >= 1008.0) {
        m_weatherForecastBadge->setText(tr("🌤️ Ổn định / Thuận lợi"));
        m_weatherAdviceLabel->setText(tr("Khí quyển duy trì ổn định, thời tiết bình thường."));
    } else if (pressureHpa >= 1000.0) {
        m_weatherForecastBadge->setText(tr("⛅ Nhiều mây / Khả năng mưa"));
        m_weatherAdviceLabel->setText(tr("Áp suất giảm nhẹ, độ ẩm tăng, có thể có mưa rào."));
    } else {
        m_weatherForecastBadge->setText(tr("⛈️ Áp thấp / Nguy cơ bão"));
        m_weatherAdviceLabel->setText(tr("Cảnh báo khí áp giảm mạnh! Nguy cơ dông lốc, mưa to bão lớn!"));
    }
    m_weatherForecastBadge->style()->unpolish(m_weatherForecastBadge);
    m_weatherForecastBadge->style()->polish(m_weatherForecastBadge);
}

void DashboardPage::updateRealtimeChart(double uvIndex, double pressureHpa)
{
    m_uvSeries->append(m_sampleCount, uvIndex);
    m_pressureSeries->append(m_sampleCount, pressureHpa);

    while (m_uvSeries->count() > 25) {
        m_uvSeries->remove(0);
        m_pressureSeries->remove(0);
    }

    if (m_axisY_Pres && !m_pressureSeries->points().isEmpty()) {
        double minP = m_pressureSeries->points().first().y();
        double maxP = minP;
        for (const QPointF &pt : m_pressureSeries->points()) {
            minP = qMin(minP, pt.y());
            maxP = qMax(maxP, pt.y());
        }
        const double diff = maxP - minP;
        const double pad = qMax(5.0, (diff == 0.0 ? 10.0 : diff * 0.2));
        m_axisY_Pres->setRange(qMax(0.0, minP - pad), maxP + pad);
    }

    m_axisX->setRange(qMax(0, m_sampleCount - 24), qMax(24, m_sampleCount));
    ++m_sampleCount;
}

void DashboardPage::updateReading(const SensorReading &reading)
{
    const QDateTime measured = reading.measuredAt.isValid()
        ? reading.measuredAt.toLocalTime()
        : QDateTime::currentDateTime();

    m_lastUpdatedLabel->setText(tr("Cập nhật: %1").arg(measured.toString(QStringLiteral("HH:mm:ss"))));
    m_statusBadge->setText(tr("● Trực tuyến"));
    m_statusBadge->setObjectName(QStringLiteral("badgeOnline"));
    m_statusBadge->style()->unpolish(m_statusBadge);
    m_statusBadge->style()->polish(m_statusBadge);

    updateUvCard(reading.uvIndex, reading.uvVoltage);
    updatePressureCard(reading.pressureHpa, reading.temperatureC);
    updateRealtimeChart(reading.uvIndex, reading.pressureHpa);
}

void DashboardPage::setDevices(const QJsonArray &devices)
{
    if (devices.isEmpty())
        return;

    for (const QJsonValue &v : devices) {
        const QJsonObject dev = v.toObject();
        const QString devId = dev.value(QStringLiteral("device_id")).toString();
        if (devId.contains(QStringLiteral("Trungkien"), Qt::CaseInsensitive) || devId == m_activeDeviceId) {
            m_activeDeviceId = devId;
            m_stationIdLabel->setText(devId);
            const QString devName = dev.value(QStringLiteral("name")).toString();
            if (!devName.isEmpty()) {
                if (devName.contains(QStringLiteral("Trung Kiên"), Qt::CaseInsensitive)
                    || devName.contains(QStringLiteral("Trạm Đo UV"), Qt::CaseInsensitive)) {
                    m_stationNameLabel->setText(tr("TRẠM ĐO UV & ÁP SUẤT KHÍ QUYỂN"));
                } else {
                    m_stationNameLabel->setText(devName);
                }
            }

            const bool online = dev.value(QStringLiteral("online")).toBool(false);
            if (online) {
                m_statusBadge->setText(tr("● Trực tuyến"));
                m_statusBadge->setObjectName(QStringLiteral("badgeOnline"));
            } else {
                m_statusBadge->setText(tr("○ Ngoại tuyến"));
                m_statusBadge->setObjectName(QStringLiteral("badgeOffline"));
            }
            m_statusBadge->style()->unpolish(m_statusBadge);
            m_statusBadge->style()->polish(m_statusBadge);

            const QJsonObject metrics = dev.value(QStringLiteral("metrics")).toObject();
            if (!metrics.isEmpty()) {
                const double uv = metrics.value(QStringLiteral("uv_index")).toDouble(0.0);
                const double uvV = metrics.value(QStringLiteral("uv_voltage")).toDouble(0.0);
                const double pres = metrics.value(QStringLiteral("pressure_hpa")).toDouble(1013.25);
                updateUvCard(uv, uvV);
                updatePressureCard(pres, 28.0);
                if (online)
                    updateRealtimeChart(uv, pres);

                const QString lastSeen = dev.value(QStringLiteral("last_seen_at")).toString();
                if (!lastSeen.isEmpty()) {
                    QDateTime dt = QDateTime::fromString(lastSeen, Qt::ISODateWithMs);
                    if (!dt.isValid()) dt = QDateTime::fromString(lastSeen, Qt::ISODate);
                    if (dt.isValid()) {
                        m_lastUpdatedLabel->setText(tr("Cập nhật: %1").arg(dt.toLocalTime().toString(QStringLiteral("HH:mm:ss"))));
                    }
                }
            }

            const QJsonObject stateObj = dev.value(QStringLiteral("state")).toObject();
            if (stateObj.contains(QStringLiteral("relay"))) {
                const bool relayState = stateObj.value(QStringLiteral("relay")).toBool(false);
                if (m_relayActive != relayState) {
                    m_relayActive = relayState;
                    m_relayButton->setObjectName(m_relayActive ? QStringLiteral("relayButtonOn") : QStringLiteral("relayButtonOff"));
                    m_relayButton->setText(m_relayActive ? tr("🔔 CÒI CẢNH BÁO: BẬT") : tr("🔔 CÒI CẢNH BÁO: TẮT"));
                    m_relayButton->style()->unpolish(m_relayButton);
                    m_relayButton->style()->polish(m_relayButton);
                }
            }
            break;
        }
    }
}
