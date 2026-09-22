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
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScroller>
#include <QStyle>
#include <QValueAxis>
#include <QVBoxLayout>

namespace {

QFrame *makeFrame(const char *objectName = "scadaCard")
{
    auto *card = new QFrame;
    card->setObjectName(QString::fromLatin1(objectName));
    card->setFrameShape(QFrame::NoFrame);
    return card;
}

QLabel *makeLabel(const QString &text, const char *objectName = nullptr, bool bold = false)
{
    auto *lbl = new QLabel(text);
    if (objectName)
        lbl->setObjectName(QString::fromLatin1(objectName));
    lbl->setWordWrap(false);
    if (bold) {
        QFont f = lbl->font();
        f.setBold(true);
        lbl->setFont(f);
    }
    return lbl;
}

struct ClickFilter : public QObject {
    std::function<void()> onClick;
    ClickFilter(QObject *parent, std::function<void()> cb) : QObject(parent), onClick(cb) {}
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonRelease) {
            if (onClick) onClick();
            return true;
        }
        return QObject::eventFilter(watched, event);
    }
};

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
        "QFrame#scadaCard { background-color: #0b1329; border: 1.5px solid #1c2b54; border-radius: 8px; } "
        "QFrame#sensorBlock { background-color: #0f1c3d; border: 1px solid #23386b; border-radius: 6px; } "
        "QFrame#sensorBlock:hover { border-color: #38bdf8; background-color: #13244e; } "
        "QFrame#busGridFrame { background-color: #09132c; border: 1px dashed #0284c7; border-radius: 5px; } "
        "QLabel#panelHeader { color: #38bdf8; font-size: 11px; font-weight: 900; letter-spacing: 0.5px; text-transform: uppercase; } "
        "QLabel#blockHeader { color: #cbd5e1; font-size: 10px; font-weight: 800; } "
        "QLabel#busLabel { color: #38bdf8; font-size: 10px; font-weight: 900; letter-spacing: 0.5px; } "
        "QLabel#bigValVoltage { color: #38bdf8; font-size: 24px; font-weight: 900; font-family: monospace; } "
        "QLabel#bigValCurrent { color: #f59e0b; font-size: 24px; font-weight: 900; font-family: monospace; } "
        "QLabel#bigValPower { color: #10b981; font-size: 22px; font-weight: 900; font-family: monospace; } "
        "QLabel#metaKey { color: #94a3b8; font-size: 9px; font-weight: 600; } "
        "QLabel#metaVal { color: #ffffff; font-size: 10px; font-weight: 800; } "
        "QLabel#badgeNormal { background-color: rgba(16, 185, 129, 0.2); color: #34d399; border: 1px solid #059669; border-radius: 4px; padding: 2px 6px; font-size: 9px; font-weight: 800; } "
        "QLabel#badgeWarning { background-color: rgba(234, 179, 8, 0.2); color: #fde047; border: 1px solid #ca8a04; border-radius: 4px; padding: 2px 6px; font-size: 9px; font-weight: 800; } "
        "QLabel#badgeDanger { background-color: rgba(239, 68, 68, 0.2); color: #f87171; border: 1px solid #dc2626; border-radius: 4px; padding: 2px 6px; font-size: 9px; font-weight: 800; } "
        "QLabel#sensorPhoto { border: 1.5px solid #23386b; border-radius: 6px; background-color: #060b18; } "
        "QProgressBar#voltageBar { background-color: #111d3d; border: 1px solid #1c2b54; border-radius: 3px; max-height: 7px; text-align: center; } "
        "QProgressBar#voltageBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #eab308, stop:0.65 #10b981, stop:0.85 #10b981, stop:1.0 #ef4444); border-radius: 2px; } "
        "QProgressBar#currentBar { background-color: #111d3d; border: 1px solid #1c2b54; border-radius: 3px; max-height: 7px; text-align: center; } "
        "QProgressBar#currentBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #10b981, stop:0.5 #f59e0b, stop:0.85 #ef4444); border-radius: 2px; } "
        "QPushButton#chartTab { background: #0e1938; color: #94a3b8; border: 1px solid #223565; border-radius: 4px; padding: 3px 8px; font-size: 9px; font-weight: 800; } "
        "QPushButton#chartTab:hover { background: #172554; color: #ffffff; } "
        "QPushButton#chartTabActive { background: #0284c7; color: #ffffff; border: 1px solid #38bdf8; border-radius: 4px; padding: 3px 8px; font-size: 9px; font-weight: 900; } "
        "QPushButton#actionButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0284c7, stop:1 #0369a1); color: #ffffff; border: 1px solid #38bdf8; border-radius: 5px; padding: 5px 10px; font-size: 9px; font-weight: 900; } "
        "QPushButton#actionButton:hover { background: #0284c7; } "
        "QPushButton#relayOnBtn { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #10b981, stop:1 #059669); color: #ffffff; border: 1px solid #34d399; border-radius: 5px; padding: 5px 10px; font-size: 9px; font-weight: 900; } "
        "QPushButton#relayOffBtn { background: #1f1422; color: #f87171; border: 1px solid #7f1d1d; border-radius: 5px; padding: 5px 10px; font-size: 9px; font-weight: 900; } "
        "QLabel#adviceBanner { color: #cbd5e1; font-size: 9px; font-weight: 600; padding: 4px; border-radius: 4px; background: rgba(30, 41, 59, 0.4); border: 1px solid #1e293b; }"
    ));

    ui->verticalLayout->setContentsMargins(6, 6, 6, 6);
    ui->verticalLayout->setSpacing(0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(QStringLiteral("QScrollArea { background: transparent; border: none; }"));
    QScroller::grabGesture(scrollArea->viewport(), QScroller::LeftMouseButtonGesture);

    auto *container = new QWidget(scrollArea);
    container->setObjectName(QStringLiteral("dashboardContainer"));
    container->setStyleSheet(QStringLiteral("QWidget#dashboardContainer { background: #060b18; }"));

    m_masterHLayout = new QHBoxLayout(container);
    m_masterHLayout->setContentsMargins(0, 0, 0, 0);
    m_masterHLayout->setSpacing(8);

    // =========================================================================
    // KHỐI TRÁI: SƠ ĐỒ NGUYÊN LÝ BIỂU TRƯNG & CẢM BIẾN (Width ~345px)
    // =========================================================================
    m_leftPanel = makeFrame("scadaCard");
    m_leftPanel->setFixedWidth(345);
    auto *leftLayout = new QVBoxLayout(m_leftPanel);
    leftLayout->setContentsMargins(8, 8, 8, 8);
    leftLayout->setSpacing(6);

    auto *leftHeadRow = new QHBoxLayout;
    leftHeadRow->addWidget(makeLabel(tr("⚡ SƠ ĐỒ MẠCH NGUYÊN LÝ & CẢM BIẾN"), "panelHeader", true));
    leftHeadRow->addStretch();
    leftLayout->addLayout(leftHeadRow);

    // 1. AC Grid Bus Bar (Biểu trưng nguồn lưới)
    auto *busFrame = makeFrame("busGridFrame");
    auto *busLayout = new QHBoxLayout(busFrame);
    busLayout->setContentsMargins(8, 4, 8, 4);
    busLayout->addWidget(makeLabel(tr("∿ NGUỒN LƯỚI AC: 220V - 50Hz (TCVN)"), "busLabel", true));
    busLayout->addStretch();
    busLayout->addWidget(makeLabel(tr("ĐẦU VÀO"), "metaKey"));
    leftLayout->addWidget(busFrame);

    // 2. Sensor 1: ZMPT101B (Biến áp cảm ứng đo áp)
    m_blockZmpt = makeFrame("sensorBlock");
    m_blockZmpt->setCursor(Qt::PointingHandCursor);
    m_blockZmpt->installEventFilter(new ClickFilter(m_blockZmpt, [this] { openVoltageDetail(); }));
    auto *zmptLayout = new QVBoxLayout(m_blockZmpt);
    zmptLayout->setContentsMargins(8, 6, 8, 6);
    zmptLayout->setSpacing(3);

    auto *zTop = new QHBoxLayout;
    zTop->addWidget(makeLabel(tr("⚡ ĐIỆN ÁP (ZMPT101B)"), "blockHeader", true));
    zTop->addStretch();
    m_voltageStatusBadge = makeLabel(tr("220V Ổn định"), "badgeNormal", true);
    zTop->addWidget(m_voltageStatusBadge);
    zmptLayout->addLayout(zTop);

    auto *zMid = new QHBoxLayout;
    m_voltageValLabel = makeLabel(QStringLiteral("221.8"), "bigValVoltage", true);
    zMid->addWidget(m_voltageValLabel);
    zMid->addWidget(makeLabel(tr("V"), "metaKey"), 0, Qt::AlignBottom);
    zMid->addSpacing(10);

    auto *zMeta = new QVBoxLayout;
    zMeta->setSpacing(1);
    auto *zFRow = new QHBoxLayout;
    zFRow->addWidget(makeLabel(tr("Tần số:"), "metaKey"));
    m_voltageFreqLabel = makeLabel(QStringLiteral("50.0 Hz"), "metaVal", true);
    zFRow->addWidget(m_voltageFreqLabel);
    zMeta->addLayout(zFRow);

    auto *zPkRow = new QHBoxLayout;
    zPkRow->addWidget(makeLabel(tr("V_đỉnh:"), "metaKey"));
    m_voltagePeakLabel = makeLabel(QStringLiteral("313.6 V"), "metaVal");
    zPkRow->addWidget(m_voltagePeakLabel);
    zMeta->addLayout(zPkRow);
    zMid->addLayout(zMeta);
    zMid->addStretch();

    m_voltageImageLabel = new QLabel(m_blockZmpt);
    m_voltageImageLabel->setObjectName(QStringLiteral("sensorPhoto"));
    m_voltageImageLabel->setFixedSize(44, 44);
    m_voltageImageLabel->setScaledContents(true);
    QPixmap p1(QStringLiteral(":/images/zmpt101b.png"));
    if (!p1.isNull()) {
        m_voltageImageLabel->setPixmap(p1.scaled(44, 44, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    m_voltageImageLabel->setToolTip(tr("Biến áp đo điện áp ZMPT101B - Nhấn xem chi tiết"));
    zMid->addWidget(m_voltageImageLabel, 0, Qt::AlignVCenter);
    zmptLayout->addLayout(zMid);
    leftLayout->addWidget(m_blockZmpt);

    // 3. Sensor 2: ACS712 (Cảm biến dòng Hall)
    m_blockAcs = makeFrame("sensorBlock");
    m_blockAcs->setCursor(Qt::PointingHandCursor);
    m_blockAcs->installEventFilter(new ClickFilter(m_blockAcs, [this] { openCurrentDetail(); }));
    auto *acsLayout = new QVBoxLayout(m_blockAcs);
    acsLayout->setContentsMargins(8, 6, 8, 6);
    acsLayout->setSpacing(3);

    auto *aTop = new QHBoxLayout;
    aTop->addWidget(makeLabel(tr("🔌 DÒNG TẢI (ACS712)"), "blockHeader", true));
    aTop->addStretch();
    m_currentStatusBadge = makeLabel(tr("An toàn (<16A)"), "badgeNormal", true);
    aTop->addWidget(m_currentStatusBadge);
    acsLayout->addLayout(aTop);

    auto *aMid = new QHBoxLayout;
    m_currentValLabel = makeLabel(QStringLiteral("2.35"), "bigValCurrent", true);
    aMid->addWidget(m_currentValLabel);
    aMid->addWidget(makeLabel(tr("A"), "metaKey"), 0, Qt::AlignBottom);
    aMid->addSpacing(10);

    auto *aMeta = new QVBoxLayout;
    aMeta->setSpacing(1);
    auto *aMxRow = new QHBoxLayout;
    aMxRow->addWidget(makeLabel(tr("Dòng max:"), "metaKey"));
    m_currentMaxLabel = makeLabel(QStringLiteral("30.0 A"), "metaVal", true);
    aMxRow->addWidget(m_currentMaxLabel);
    aMeta->addLayout(aMxRow);

    auto *aSnRow = new QHBoxLayout;
    aSnRow->addWidget(makeLabel(tr("Độ nhạy:"), "metaKey"));
    m_currentSensLabel = makeLabel(QStringLiteral("66 mV/A"), "metaVal");
    aSnRow->addWidget(m_currentSensLabel);
    aMeta->addLayout(aSnRow);
    aMid->addLayout(aMeta);
    aMid->addStretch();

    m_currentImageLabel = new QLabel(m_blockAcs);
    m_currentImageLabel->setObjectName(QStringLiteral("sensorPhoto"));
    m_currentImageLabel->setFixedSize(44, 44);
    m_currentImageLabel->setScaledContents(true);
    QPixmap p2(QStringLiteral(":/images/acs712.png"));
    if (!p2.isNull()) {
        m_currentImageLabel->setPixmap(p2.scaled(44, 44, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    m_currentImageLabel->setToolTip(tr("Cảm biến dòng điện ACS712 - Nhấn xem chi tiết"));
    aMid->addWidget(m_currentImageLabel, 0, Qt::AlignVCenter);
    acsLayout->addLayout(aMid);
    leftLayout->addWidget(m_blockAcs);

    // 4. Phụ tải & Rơ-le bảo vệ
    m_blockLoadRelay = makeFrame("sensorBlock");
    auto *loadLayout = new QVBoxLayout(m_blockLoadRelay);
    loadLayout->setContentsMargins(8, 6, 8, 6);
    loadLayout->setSpacing(3);

    auto *lTop = new QHBoxLayout;
    lTop->addWidget(makeLabel(tr("💡 PHỤ TẢI & CÔNG SUẤT"), "blockHeader", true));
    lTop->addStretch();
    m_powerValLabel = makeLabel(QStringLiteral("518"), "bigValPower", true);
    lTop->addWidget(m_powerValLabel);
    lTop->addWidget(makeLabel(tr("W"), "metaKey"), 0, Qt::AlignBottom);
    loadLayout->addLayout(lTop);

    auto *lMetaRow = new QHBoxLayout;
    m_powerFactorLabel = makeLabel(tr("cosφ ≈ 0.98"), "metaVal", true);
    m_energyKwhLabel = makeLabel(tr("0.12 kWh"), "metaVal", true);
    lMetaRow->addWidget(makeLabel(tr("Hệ số:"), "metaKey"));
    lMetaRow->addWidget(m_powerFactorLabel);
    lMetaRow->addSpacing(10);
    lMetaRow->addWidget(makeLabel(tr("Điện năng:"), "metaKey"));
    lMetaRow->addWidget(m_energyKwhLabel);
    lMetaRow->addStretch();
    loadLayout->addLayout(lMetaRow);

    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(4);

    m_relayButton = new QPushButton(tr("⚡ RƠ LE: ĐÓNG TẢI"), m_blockLoadRelay);
    m_relayButton->setObjectName(QStringLiteral("relayOnBtn"));
    m_relayButton->setCursor(Qt::PointingHandCursor);
    connect(m_relayButton, &QPushButton::clicked, this, [this] {
        m_relayActive = !m_relayActive;
        m_relayButton->setObjectName(m_relayActive ? QStringLiteral("relayOnBtn") : QStringLiteral("relayOffBtn"));
        m_relayButton->setText(m_relayActive ? tr("⚡ RƠ LE: ĐÓNG TẢI") : tr("🚨 RƠ LE: NGẮT TẢI"));
        m_relayButton->style()->unpolish(m_relayButton);
        m_relayButton->style()->polish(m_relayButton);
        emit relayControlRequested(m_deviceId, m_relayActive);
    });
    btnRow->addWidget(m_relayButton, 1);

    m_viewHistoryButton = new QPushButton(tr("📊 LỊCH SỬ"), m_blockLoadRelay);
    m_viewHistoryButton->setObjectName(QStringLiteral("actionButton"));
    m_viewHistoryButton->setCursor(Qt::PointingHandCursor);
    connect(m_viewHistoryButton, &QPushButton::clicked, this, &DashboardPage::historyPageRequested);
    btnRow->addWidget(m_viewHistoryButton, 1);

    m_devicesButton = new QPushButton(tr("⚙️ THIẾT BỊ"), m_blockLoadRelay);
    m_devicesButton->setObjectName(QStringLiteral("chartTab"));
    m_devicesButton->setCursor(Qt::PointingHandCursor);
    connect(m_devicesButton, &QPushButton::clicked, this, [this] {
        emit devicesPageRequested();
    });
    btnRow->addWidget(m_devicesButton, 0);

    loadLayout->addLayout(btnRow);
    leftLayout->addWidget(m_blockLoadRelay);

    leftLayout->addStretch();
    m_masterHLayout->addWidget(m_leftPanel, 0);

    // =========================================================================
    // KHỐI PHẢI: TRUNG TÂM BIỂU ĐỒ DIỄN BIẾN THỜI GIAN THỰC (Không co rít)
    // =========================================================================
    m_rightPanel = makeFrame("scadaCard");
    auto *rightLayout = new QVBoxLayout(m_rightPanel);
    rightLayout->setContentsMargins(10, 8, 10, 8);
    rightLayout->setSpacing(6);

    // Chart Header row with filter tabs
    auto *chartHeader = new QHBoxLayout;
    chartHeader->setSpacing(3);
    chartHeader->addWidget(makeLabel(tr("📈 BIỂU ĐỒ DIỄN BIẾN"), "panelHeader", true));
    chartHeader->addStretch();

    m_chartFilterAll = new QPushButton(tr("Tất cả"), m_rightPanel);
    m_chartFilterAll->setObjectName(QStringLiteral("chartTabActive"));
    m_chartFilterVoltage = new QPushButton(tr("U (V)"), m_rightPanel);
    m_chartFilterVoltage->setObjectName(QStringLiteral("chartTab"));
    m_chartFilterCurrent = new QPushButton(tr("I (A)"), m_rightPanel);
    m_chartFilterCurrent->setObjectName(QStringLiteral("chartTab"));
    m_chartFilterPower = new QPushButton(tr("P (W)"), m_rightPanel);
    m_chartFilterPower->setObjectName(QStringLiteral("chartTab"));

    const auto setChartFilter = [this](int mode) {
        m_chartMode = mode;
        m_chartFilterAll->setObjectName(mode == 0 ? QStringLiteral("chartTabActive") : QStringLiteral("chartTab"));
        m_chartFilterVoltage->setObjectName(mode == 1 ? QStringLiteral("chartTabActive") : QStringLiteral("chartTab"));
        m_chartFilterCurrent->setObjectName(mode == 2 ? QStringLiteral("chartTabActive") : QStringLiteral("chartTab"));
        m_chartFilterPower->setObjectName(mode == 3 ? QStringLiteral("chartTabActive") : QStringLiteral("chartTab"));

        for (auto *b : {m_chartFilterAll, m_chartFilterVoltage, m_chartFilterCurrent, m_chartFilterPower}) {
            b->style()->unpolish(b);
            b->style()->polish(b);
        }

        if (m_voltageSeries) m_voltageSeries->setVisible(mode == 0 || mode == 1);
        if (m_currentSeries) m_currentSeries->setVisible(mode == 0 || mode == 2);
        if (m_powerSeries) m_powerSeries->setVisible(mode == 0 || mode == 3);

        if (m_axisY_Voltage) m_axisY_Voltage->setVisible(mode == 0 || mode == 1 || mode == 3);
        if (m_axisY_Current) m_axisY_Current->setVisible(mode == 0 || mode == 2);
    };

    connect(m_chartFilterAll, &QPushButton::clicked, this, [=] { setChartFilter(0); });
    connect(m_chartFilterVoltage, &QPushButton::clicked, this, [=] { setChartFilter(1); });
    connect(m_chartFilterCurrent, &QPushButton::clicked, this, [=] { setChartFilter(2); });
    connect(m_chartFilterPower, &QPushButton::clicked, this, [=] { setChartFilter(3); });

    chartHeader->addWidget(m_chartFilterAll);
    chartHeader->addWidget(m_chartFilterVoltage);
    chartHeader->addWidget(m_chartFilterCurrent);
    chartHeader->addWidget(m_chartFilterPower);
    rightLayout->addLayout(chartHeader);

    // QChart and QChartView
    m_chart = new QChart;
    m_chart->legend()->hide();
    m_chart->setBackgroundVisible(false);
    m_chart->setMargins(QMargins(0, 0, 0, 0));

    m_voltageSeries = new QLineSeries(this);
    m_voltageSeries->setPen(QPen(QColor("#38bdf8"), 2.2)); // Voltage Cyan

    m_currentSeries = new QLineSeries(this);
    m_currentSeries->setPen(QPen(QColor("#f59e0b"), 2.2)); // Current Amber

    m_powerSeries = new QLineSeries(this);
    m_powerSeries->setPen(QPen(QColor("#10b981"), 2.2)); // Power Emerald

    m_chart->addSeries(m_voltageSeries);
    m_chart->addSeries(m_currentSeries);
    m_chart->addSeries(m_powerSeries);

    QFont axisF;
    axisF.setPixelSize(8);

    m_axisX = new QValueAxis(m_chart);
    m_axisX->setRange(0, 24);
    m_axisX->setTickCount(5);
    m_axisX->setLabelFormat(QStringLiteral("%d"));
    m_axisX->setGridLineColor(QColor("#1c2b54"));
    m_axisX->setLabelsColor(QColor("#64748b"));
    m_axisX->setLabelsFont(axisF);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_voltageSeries->attachAxis(m_axisX);
    m_currentSeries->attachAxis(m_axisX);
    m_powerSeries->attachAxis(m_axisX);

    // Left Y Axis: Voltage & Power
    m_axisY_Voltage = new QValueAxis(m_chart);
    m_axisY_Voltage->setRange(0, 260);
    m_axisY_Voltage->setTickCount(4);
    m_axisY_Voltage->setLabelFormat(QStringLiteral("%.0f"));
    m_axisY_Voltage->setLabelsColor(QColor("#38bdf8"));
    m_axisY_Voltage->setLabelsFont(axisF);
    m_axisY_Voltage->setGridLineColor(QColor("#1c2b54"));
    m_chart->addAxis(m_axisY_Voltage, Qt::AlignLeft);
    m_voltageSeries->attachAxis(m_axisY_Voltage);
    m_powerSeries->attachAxis(m_axisY_Voltage);

    // Right Y Axis: Current
    m_axisY_Current = new QValueAxis(m_chart);
    m_axisY_Current->setRange(0, 10);
    m_axisY_Current->setTickCount(4);
    m_axisY_Current->setLabelFormat(QStringLiteral("%.1f"));
    m_axisY_Current->setLabelsColor(QColor("#f59e0b"));
    m_axisY_Current->setLabelsFont(axisF);
    m_axisY_Current->setGridLineColor(QColor("#1c2b54"));
    m_chart->addAxis(m_axisY_Current, Qt::AlignRight);
    m_currentSeries->attachAxis(m_axisY_Current);

    m_chartView = new QChartView(m_chart, m_rightPanel);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(180);
    m_chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_chartView->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    rightLayout->addWidget(m_chartView, 1);

    // Safety & Range Bars
    auto *meterGrid = new QGridLayout;
    meterGrid->setContentsMargins(0, 0, 0, 0);
    meterGrid->setHorizontalSpacing(8);
    meterGrid->setVerticalSpacing(3);

    meterGrid->addWidget(makeLabel(tr("⚡ Điện áp AC (0 - 300V):"), "metaKey"), 0, 0);
    m_voltageBar = new QProgressBar(m_rightPanel);
    m_voltageBar->setObjectName(QStringLiteral("voltageBar"));
    m_voltageBar->setRange(0, 300);
    m_voltageBar->setValue(222);
    m_voltageBar->setTextVisible(false);
    meterGrid->addWidget(m_voltageBar, 0, 1);

    meterGrid->addWidget(makeLabel(tr("🔌 Dòng tải AC (0 - 30A):"), "metaKey"), 1, 0);
    m_currentBar = new QProgressBar(m_rightPanel);
    m_currentBar->setObjectName(QStringLiteral("currentBar"));
    m_currentBar->setRange(0, 300);
    m_currentBar->setValue(24);
    m_currentBar->setTextVisible(false);
    meterGrid->addWidget(m_currentBar, 1, 1);

    rightLayout->addLayout(meterGrid);

    // Diagnostics & Advice banner
    auto *botRow = new QHBoxLayout;
    botRow->setSpacing(6);
    m_statusAdviceLabel = makeLabel(tr("💡 Lưới điện 220V và phụ tải ổn định trong tiêu chuẩn an toàn TCVN."), "adviceBanner");
    m_statusAdviceLabel->setWordWrap(true);
    botRow->addWidget(m_statusAdviceLabel, 1);

    m_lastUpdatedLabel = makeLabel(tr("🕒 --:--:--"), "metaKey");
    m_lastUpdatedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    botRow->addWidget(m_lastUpdatedLabel, 0, Qt::AlignVCenter);

    rightLayout->addLayout(botRow);

    m_masterHLayout->addWidget(m_rightPanel, 1);

    scrollArea->setWidget(container);
    ui->verticalLayout->addWidget(scrollArea);
}

void DashboardPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    applyResponsiveLayout();
}

void DashboardPage::applyResponsiveLayout()
{
    if (!m_masterHLayout)
        return;

    const bool compact = width() < 640;
    if (compact) {
        m_masterHLayout->setDirection(QBoxLayout::TopToBottom);
        if (m_leftPanel) m_leftPanel->setFixedWidth(QWIDGETSIZE_MAX);
    } else {
        m_masterHLayout->setDirection(QBoxLayout::LeftToRight);
        if (m_leftPanel) m_leftPanel->setFixedWidth(345);
    }
}

void DashboardPage::updateVoltageDisplay(double voltageV)
{
    m_curVoltage = voltageV;
    m_voltageValLabel->setText(voltageV > 0.0 ? QString::number(voltageV, 'f', 1) : QStringLiteral("--"));
    m_voltageBar->setValue(qBound(0, static_cast<int>(voltageV), 300));
    m_voltagePeakLabel->setText(QStringLiteral("%1 V").arg(QString::number(voltageV * 1.4142, 'f', 1)));

    if (voltageV < 10.0) {
        m_voltageStatusBadge->setText(tr("⚠️ Sụt áp lưới!"));
        m_voltageStatusBadge->setObjectName(QStringLiteral("badgeWarning"));
    } else if (voltageV > 245.0) {
        m_voltageStatusBadge->setText(tr("🚨 Quá áp lưới!"));
        m_voltageStatusBadge->setObjectName(QStringLiteral("badgeDanger"));
    } else {
        m_voltageStatusBadge->setText(tr("Điện áp ổn định"));
        m_voltageStatusBadge->setObjectName(QStringLiteral("badgeNormal"));
    }
    m_voltageStatusBadge->style()->unpolish(m_voltageStatusBadge);
    m_voltageStatusBadge->style()->polish(m_voltageStatusBadge);
}

void DashboardPage::updateCurrentDisplay(double currentA)
{
    m_curCurrent = currentA;
    m_currentValLabel->setText(currentA >= 0.0 ? QString::number(currentA, 'f', 2) : QStringLiteral("--"));
    m_currentBar->setValue(qBound(0, static_cast<int>(currentA * 10.0), 300));

    if (currentA > 20.0) {
        m_currentStatusBadge->setText(tr("🚨 Quá dòng (>20A)!"));
        m_currentStatusBadge->setObjectName(QStringLiteral("badgeDanger"));
    } else if (currentA > 15.0) {
        m_currentStatusBadge->setText(tr("⚠️ Tải cao (>15A)"));
        m_currentStatusBadge->setObjectName(QStringLiteral("badgeWarning"));
    } else {
        m_currentStatusBadge->setText(tr("Tải an toàn (<16A)"));
        m_currentStatusBadge->setObjectName(QStringLiteral("badgeNormal"));
    }
    m_currentStatusBadge->style()->unpolish(m_currentStatusBadge);
    m_currentStatusBadge->style()->polish(m_currentStatusBadge);
}

void DashboardPage::updatePowerDisplay(double powerW)
{
    m_curPower = powerW;
    m_powerValLabel->setText(powerW >= 0.0 ? QString::number(static_cast<int>(powerW)) : QStringLiteral("--"));
    double kwh = (powerW * 0.5) / 1000.0;
    m_energyKwhLabel->setText(QStringLiteral("%1 kWh").arg(QString::number(kwh, 'f', 2)));
}

void DashboardPage::updateRealtimeChart(double voltageV, double currentA, double powerW)
{
    m_voltageSeries->append(m_sampleCount, voltageV);
    m_currentSeries->append(m_sampleCount, currentA);
    m_powerSeries->append(m_sampleCount, powerW);

    while (m_voltageSeries->count() > 25) {
        m_voltageSeries->remove(0);
        m_currentSeries->remove(0);
        m_powerSeries->remove(0);
    }

    if (m_axisY_Voltage && !m_voltageSeries->points().isEmpty()) {
        if (m_chartMode == 3) {
            // Power only
            double minP = m_powerSeries->points().first().y();
            double maxP = minP;
            for (const QPointF &pt : m_powerSeries->points()) {
                minP = qMin(minP, pt.y());
                maxP = qMax(maxP, pt.y());
            }
            const double diff = maxP - minP;
            const double pad = qMax(20.0, (diff == 0.0 ? 50.0 : diff * 0.2));
            m_axisY_Voltage->setRange(qMax(0.0, minP - pad), maxP + pad);
        } else {
            // Voltage or All
            double minV = m_voltageSeries->points().first().y();
            double maxV = minV;
            for (const QPointF &pt : m_voltageSeries->points()) {
                minV = qMin(minV, pt.y());
                maxV = qMax(maxV, pt.y());
            }
            const double diff = maxV - minV;
            const double pad = qMax(5.0, (diff == 0.0 ? 10.0 : diff * 0.2));
            m_axisY_Voltage->setRange(qMax(0.0, minV - pad), maxV + pad);
        }
    }

    if (m_axisY_Current && !m_currentSeries->points().isEmpty()) {
        double maxC = 5.0;
        for (const QPointF &pt : m_currentSeries->points()) {
            maxC = qMax(maxC, pt.y());
        }
        m_axisY_Current->setRange(0.0, qMax(6.0, maxC * 1.3));
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

    double v = reading.pressureHpa > 0 ? reading.pressureHpa : 220.0;
    double a = reading.distanceCm >= 0 ? reading.distanceCm : 2.35;
    double p = reading.temperatureC > 0 ? reading.temperatureC : (v * a * 0.98);

    m_voltageHistory.append({measured, v});
    m_currentHistory.append({measured, a});
    m_powerHistory.append({measured, p});

    if (m_voltageHistory.size() > 100) m_voltageHistory.removeFirst();
    if (m_currentHistory.size() > 100) m_currentHistory.removeFirst();
    if (m_powerHistory.size() > 100) m_powerHistory.removeFirst();

    updateVoltageDisplay(v);
    updateCurrentDisplay(a);
    updatePowerDisplay(p);
    updateRealtimeChart(v, a, p);
}

void DashboardPage::updateDeviceMetrics(const QJsonObject &metrics)
{
    double v = m_curVoltage;
    double a = m_curCurrent;
    double p = m_curPower;

    if (metrics.contains(QStringLiteral("voltage_v"))) {
        v = metrics.value(QStringLiteral("voltage_v")).toDouble();
    } else if (metrics.contains(QStringLiteral("pressure_hpa"))) {
        v = metrics.value(QStringLiteral("pressure_hpa")).toDouble();
    }

    if (metrics.contains(QStringLiteral("current_a"))) {
        a = metrics.value(QStringLiteral("current_a")).toDouble();
    } else if (metrics.contains(QStringLiteral("distance_cm"))) {
        a = metrics.value(QStringLiteral("distance_cm")).toDouble();
    }

    if (metrics.contains(QStringLiteral("power_w"))) {
        p = metrics.value(QStringLiteral("power_w")).toDouble();
    } else {
        p = v * a * 0.98;
    }

    if (metrics.contains(QStringLiteral("relay_on"))) {
        m_relayActive = metrics.value(QStringLiteral("relay_on")).toBool();
        m_relayButton->setObjectName(m_relayActive ? QStringLiteral("relayOnBtn") : QStringLiteral("relayOffBtn"));
        m_relayButton->setText(m_relayActive ? tr("⚡ RƠ LE: ĐÓNG TẢI") : tr("🚨 RƠ LE: NGẮT TẢI"));
        m_relayButton->style()->unpolish(m_relayButton);
        m_relayButton->style()->polish(m_relayButton);
    }

    const QDateTime now = QDateTime::currentDateTime();
    m_voltageHistory.append({now, v});
    m_currentHistory.append({now, a});
    m_powerHistory.append({now, p});

    if (m_voltageHistory.size() > 100) m_voltageHistory.removeFirst();
    if (m_currentHistory.size() > 100) m_currentHistory.removeFirst();
    if (m_powerHistory.size() > 100) m_powerHistory.removeFirst();

    updateVoltageDisplay(v);
    updateCurrentDisplay(a);
    updatePowerDisplay(p);
    updateRealtimeChart(v, a, p);
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
    if (devices.isEmpty()) {
        m_hasDevice = false;
        m_isOnline = false;
        return;
    }

    m_hasDevice = true;
    for (const QJsonValue &v : devices) {
        const QJsonObject dev = v.toObject();
        const QString devId = dev.value(QStringLiteral("device_id")).toString();
        if (devId.contains(QStringLiteral("Theanh"), Qt::CaseInsensitive) || devId == m_deviceId || m_deviceId.isEmpty()) {
            m_deviceId = devId;
            const QString devName = dev.value(QStringLiteral("name")).toString();
            if (!devName.isEmpty()) {
                m_deviceName = devName;
            }
            m_isOnline = dev.value(QStringLiteral("is_online")).toBool(true);

            if (dev.contains(QStringLiteral("relay_on"))) {
                m_relayActive = dev.value(QStringLiteral("relay_on")).toBool();
                m_relayButton->setObjectName(m_relayActive ? QStringLiteral("relayOnBtn") : QStringLiteral("relayOffBtn"));
                m_relayButton->setText(m_relayActive ? tr("⚡ RƠ LE: ĐÓNG TẢI") : tr("🚨 RƠ LE: NGẮT TẢI"));
                m_relayButton->style()->unpolish(m_relayButton);
                m_relayButton->style()->polish(m_relayButton);
            }
            break;
        }
    }
}

void DashboardPage::setDeviceId(const QString &deviceId)
{
    m_deviceId = deviceId;
}

void DashboardPage::openVoltageDetail()
{
    SensorDetailDialog dlg(
        tr("Điện Áp AC Lưới Điện (ZMPT101B)"),
        QStringLiteral("V"),
        QStringLiteral("#38bdf8"),
        m_voltageHistory,
        this,
        10.0,
        250.0
    );
    dlg.exec();
}

void DashboardPage::openCurrentDetail()
{
    SensorDetailDialog dlg(
        tr("Dòng Điện Phụ Tải (ACS712)"),
        QStringLiteral("A"),
        QStringLiteral("#f59e0b"),
        m_currentHistory,
        this,
        0.0,
        20.0
    );
    dlg.exec();
}

void DashboardPage::openPowerDetail()
{
    SensorDetailDialog dlg(
        tr("Công Suất Tiêu Thụ Tức Thời (W)"),
        QStringLiteral("W"),
        QStringLiteral("#10b981"),
        m_powerHistory,
        this,
        0.0,
        2200.0
    );
    dlg.exec();
}

void DashboardPage::openAddDeviceDialog()
{
    auto *dlg = new SelectOnlineDeviceDialog(m_availableDevices, this);
    m_currentSelectDialog = dlg;

    connect(dlg, &SelectOnlineDeviceDialog::refreshRequested, this, &DashboardPage::refreshDevicesRequested);
    connect(dlg, &SelectOnlineDeviceDialog::deviceSelected, this, [this](const QString &devId, const QString &devName) {
        emit claimDeviceRequested(devId, devName);
    });
    dlg->exec();
    delete dlg;
}
