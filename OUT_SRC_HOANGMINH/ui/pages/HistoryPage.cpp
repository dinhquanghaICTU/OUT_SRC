#include "HistoryPage.h"

#include "ui_HistoryPage.h"

#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QButtonGroup>
#include <QChart>
#include <QChartView>
#include <QDate>
#include <QDateTimeAxis>
#include <QDialog>
#include <QEvent>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLegend>
#include <QLegendMarker>
#include <QLineSeries>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QTableWidgetItem>
#include <QValueAxis>
#include <QVBoxLayout>
#include <QTimer>

#include <limits>

namespace {
static QColor getColorForKey(const QString &k, int fallbackIdx = 0) {
    if (k == QStringLiteral("temperature_c")) return QColor("#38bdf8"); // Cyan Nhiệt độ LM35
    if (k == QStringLiteral("sound_vpp")) return QColor("#10b981"); // Xanh ngọc Độ ồn MAX9814
    static const QList<QColor> fallback{QColor("#38bdf8"), QColor("#10b981"), QColor("#f59e0b"), QColor("#a855f7")};
    return fallback.at(fallbackIdx % fallback.size());
}

static QString metricShortName(const QString &key) {
    static const QHash<QString, QString> names{
        {QStringLiteral("temperature_c"), QObject::tr("Nhiệt độ LM35")},
        {QStringLiteral("sound_vpp"), QObject::tr("Độ ồn MAX9814")}};
    return names.value(key, key);
}
} // namespace

HistoryPage::HistoryPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::HistoryPage),
      m_chart(new QChart),
      m_chartView(new QChartView(m_chart, this)),
      m_primaryStat(new QLabel(this)),
      m_secondaryStat(new QLabel(this)),
      m_thirdStat(new QLabel(this)),
      m_chartTitle(new QLabel(this)),
      m_chartHint(new QLabel(this)),
      m_headerSubtitle(new QLabel(this)),
      m_analyticsGrid(new QGridLayout),
      m_chartCard(new QFrame(this)),
      m_primaryStatCard(nullptr),
      m_secondaryStatCard(nullptr),
      m_summaryStatCard(nullptr)
{
    ui->setupUi(this);
    m_headerSubtitle->hide();
    setStyleSheet(
        "QWidget#HistoryPage { background-color: #070d1e; color: #ecf2ff; font-family: sans-serif; } "
        "QLabel#historyRecordBadge { background: #0e1938; color: #38bdf8; border: 1px solid #1e293b; border-radius: 4px; padding: 2px 6px; font-size: 10px; font-weight: 700; } "
        "QPushButton#deviceViewTabButton { background: #0e1938; color: #94a3b8; border: 1px solid #223565; border-radius: 6px; padding: 3px 8px; font-size: 10px; font-weight: 800; } "
        "QPushButton#deviceViewTabButton:checked { background: #0284c7; color: #ffffff; border-color: #38bdf8; font-weight: 900; } "
        "QPushButton#historySearchButton { background: #10b981; color: #ffffff; border: none; border-radius: 6px; font-size: 10px; font-weight: 900; padding: 3px 10px; } "
        "QComboBox { background-color: #0f1c3f; color: #ffffff; border: 1px solid #233870; border-radius: 6px; padding: 2px 6px; font-size: 10px; font-weight: 700; } "
        "QFrame#historyChartCard { background-color: #0d1733; border: 1px solid #1c2b54; border-radius: 8px; } "
        "QFrame#historyStatCard { background-color: #0e1938; border: 1px solid #223565; border-radius: 6px; } "
        "QLabel#historyStatTitle { color: #94a3b8; font-size: 9px; font-weight: 700; } "
        "QLabel#historyStatValue { color: #38bdf8; font-size: 12px; font-weight: 900; } "
        "QTableWidget#historyTableSmart { background-color: #0c1630; color: #ffffff; gridline-color: #1c2b54; border: 1px solid #1c2b54; border-radius: 8px; font-size: 10px; } "
        "QTableWidget#historyTableSmart QHeaderView::section { background-color: #111d3d; color: #94a3b8; font-weight: 800; font-size: 10px; padding: 4px; border: none; }"
    );

    ui->recordCountLabel->setObjectName(QStringLiteral("historyRecordBadge"));
    ui->chartTabButton->setObjectName(QStringLiteral("deviceViewTabButton"));
    ui->tableTabButton->setObjectName(QStringLiteral("deviceViewTabButton"));
    ui->deviceCombo->setObjectName(QStringLiteral("historyDeviceCombo"));
    ui->periodCombo->setObjectName(QStringLiteral("historyPeriodCombo"));
    ui->dateCombo->setObjectName(QStringLiteral("historyDateCombo"));
    ui->searchButton->setObjectName(QStringLiteral("historySearchButton"));
    ui->navBarLayout->setSpacing(8);
    ui->filterLayout->setSpacing(8);

    auto *tabGroup = new QButtonGroup(this);
    tabGroup->addButton(ui->chartTabButton);
    tabGroup->addButton(ui->tableTabButton);
    tabGroup->setExclusive(true);

    m_metricCombo = new QComboBox(this);
    m_metricCombo->setObjectName(QStringLiteral("historyMetricCombo"));
    m_metricCombo->setMinimumWidth(130);
    m_metricCombo->hide();

    m_zoomBtn = new QPushButton(tr("Phóng to"), this);
    m_zoomBtn->setObjectName(QStringLiteral("historyZoomButton"));
    m_zoomBtn->setCursor(Qt::PointingHandCursor);
    m_zoomBtn->setStyleSheet("QPushButton { background: #0e1938; color: #38bdf8; border: 1px solid #223565; border-radius: 6px; padding: 3px 8px; font-size: 10px; font-weight: 800; } QPushButton:hover { background: #1e293b; }");
    m_zoomBtn->setToolTip(tr("Phóng to biểu đồ"));
    connect(m_zoomBtn, &QPushButton::clicked, this, [this] {
        openChartZoomDialog(m_selectedMetricKey);
    });

    ui->filterLayout->addWidget(m_metricCombo);
    ui->filterLayout->addWidget(m_zoomBtn);

    connect(ui->chartTabButton, &QPushButton::clicked, this, [this] {
        ui->viewStack->setCurrentIndex(0);
        if (m_metricCombo && m_metricCombo->count() > 0) m_metricCombo->show();
        if (m_zoomBtn) m_zoomBtn->show();
    });
    connect(ui->tableTabButton, &QPushButton::clicked, this, [this] {
        ui->viewStack->setCurrentIndex(1);
        if (m_metricCombo) m_metricCombo->hide();
        if (m_zoomBtn) m_zoomBtn->hide();
    });

    ui->historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->historyTable->verticalHeader()->hide();
    ui->historyTable->setObjectName(QStringLiteral("historyTableSmart"));

    auto makeStatCard = [this](QLabel *&titleOut, const QString &defaultTitle, QLabel *value) {
        auto *card = new QFrame(this);
        card->setObjectName(QStringLiteral("historyStatCard"));
        card->setCursor(Qt::PointingHandCursor);
        card->setFixedHeight(48);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(10, 4, 10, 4);
        layout->setSpacing(2);

        titleOut = new QLabel(defaultTitle, card);
        titleOut->setObjectName(QStringLiteral("historyStatTitle"));
        value->setObjectName(QStringLiteral("historyStatValue"));
        value->setText(QStringLiteral("--"));
        value->setWordWrap(false);
        layout->addWidget(titleOut);
        layout->addWidget(value);
        card->installEventFilter(this);
        return card;
    };

    m_chartView->setObjectName(QStringLiteral("historyChart"));
    m_chartView->setMinimumHeight(190);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setCursor(Qt::PointingHandCursor);
    m_chartView->setToolTip(tr("Chạm vào biểu đồ để phóng to"));
    m_chartView->setStyleSheet(QStringLiteral("background: #070d1e; border: none;"));
    m_chartView->viewport()->installEventFilter(this);

    m_chart->setBackgroundBrush(QBrush(QColor("#070d1e")));
    m_chart->setPlotAreaBackgroundBrush(QBrush(QColor("#0c1630")));
    m_chart->setPlotAreaBackgroundVisible(true);
    m_chart->setTitle(QString());
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->legend()->setLabelColor(QColor("#ffffff"));
    m_chart->setMargins(QMargins(2, 2, 2, 2));
    m_chart->setBackgroundRoundness(0);

    QFont legFont;
    legFont.setPixelSize(9);
    m_chart->legend()->setFont(legFont);

    m_chartCard->setObjectName(QStringLiteral("historyChartCard"));
    auto *chartLayout = new QVBoxLayout(m_chartCard);
    chartLayout->setContentsMargins(10, 6, 10, 6);
    chartLayout->setSpacing(4);

    m_chartHeaderLayout = new QHBoxLayout;
    m_chartHeaderLayout->setContentsMargins(2, 0, 2, 0);
    m_chartHeaderLayout->setSpacing(4);

    m_chartTitle = new QLabel(tr("Dữ liệu cảm biến trạm làm mát"), m_chartCard);
    m_chartTitle->setStyleSheet("font-size: 11px; font-weight: 900; color: #f8fafc;");
    m_chartHint = new QLabel(m_chartCard);
    m_chartHint->hide();
    m_chartHeaderLayout->addWidget(m_chartTitle, 1);
    m_chartHeaderLayout->addWidget(m_chartHint);

    chartLayout->addLayout(m_chartHeaderLayout);
    chartLayout->addWidget(m_chartView, 1);

    connect(m_metricCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        if (m_metricCombo->currentIndex() >= 0) {
            m_selectedMetricKey = m_metricCombo->currentData().toString();
            updateChart();
        }
    });

    m_analyticsGrid->setContentsMargins(0, 0, 0, 0);
    m_analyticsGrid->setHorizontalSpacing(8);
    m_analyticsGrid->setVerticalSpacing(0);
    m_primaryStatCard = makeStatCard(m_primaryStatTitle, tr("NHIỆT ĐỘ LM35 (TB)"), m_primaryStat);
    m_secondaryStatCard = makeStatCard(m_secondaryStatTitle, tr("ĐỘ ỒN MAX9814 (TB)"), m_secondaryStat);
    m_summaryStatCard = makeStatCard(m_summaryStatTitle, tr("TỔNG SỐ BẢN GHI"), m_thirdStat);
    m_analyticsGrid->addWidget(m_primaryStatCard, 0, 0);
    m_analyticsGrid->addWidget(m_secondaryStatCard, 0, 1);
    m_analyticsGrid->addWidget(m_summaryStatCard, 0, 2);

    auto *chartPageLayout = new QVBoxLayout(ui->chartPage);
    chartPageLayout->setContentsMargins(0, 0, 0, 0);
    chartPageLayout->setSpacing(6);
    chartPageLayout->addWidget(m_chartCard, 1);
    chartPageLayout->addLayout(m_analyticsGrid);

    ui->viewStack->setCurrentIndex(0);
    applyResponsiveLayout();

    m_selectedDate = QDate::currentDate();
    ui->dateCombo->setMinimumWidth(130);
    ui->periodCombo->setMinimumWidth(75);
    ui->periodCombo->setItemData(0, QStringLiteral("day"));
    ui->periodCombo->setItemData(1, QStringLiteral("month"));
    ui->periodCombo->setItemData(2, QStringLiteral("year"));

    rebuildDateOptions();

    connect(ui->searchButton, &QPushButton::clicked,
            this, &HistoryPage::requestCurrentHistory);
    connect(ui->deviceCombo, &QComboBox::currentIndexChanged,
            this, [this](int) { requestCurrentHistory(); });
    connect(ui->periodCombo, &QComboBox::currentIndexChanged,
            this, [this](int) {
        rebuildDateOptions();
        requestCurrentHistory();
    });
    connect(ui->dateCombo, &QComboBox::currentIndexChanged,
            this, [this](int idx) {
        if (idx >= 0) {
            const QVariant dVal = ui->dateCombo->itemData(idx);
            if (dVal.isValid() && dVal.canConvert<QDate>()) {
                m_selectedDate = dVal.toDate();
            }
            requestCurrentHistory();
        }
    });
}

bool HistoryPage::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
        if (m_chartView && watched == m_chartView->viewport()) {
            openChartZoomDialog(m_selectedMetricKey);
            return true;
        }
        if (watched == m_primaryStatCard) {
            openChartZoomDialog(QStringLiteral("temperature_c"));
            return true;
        }
        if (watched == m_secondaryStatCard) {
            openChartZoomDialog(QStringLiteral("sound_vpp"));
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void HistoryPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    applyResponsiveLayout();
}

void HistoryPage::applyResponsiveLayout()
{
    const int pageWidth = contentsRect().width();
    const bool compact = pageWidth <= 800;

    ui->deviceCombo->setMinimumWidth(compact ? 160 : 220);
    ui->deviceCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->periodCombo->setMinimumWidth(compact ? 65 : 85);
    ui->dateCombo->setMinimumWidth(compact ? 130 : 160);
    ui->searchButton->setMinimumWidth(compact ? 48 : 70);
    if (m_metricCombo)
        m_metricCombo->setMinimumWidth(compact ? 120 : 150);
    if (m_zoomBtn)
        m_zoomBtn->setMinimumWidth(compact ? 75 : 90);
    m_chartView->setMinimumHeight(compact ? 190 : 250);
    ui->historyTable->setMinimumHeight(compact ? 110 : 170);
    ui->verticalLayout->setContentsMargins(compact ? 8 : 14, compact ? 6 : 10,
                                           compact ? 8 : 14, compact ? 6 : 10);
    ui->verticalLayout->setSpacing(compact ? 6 : 8);
}

HistoryPage::~HistoryPage()
{
    delete ui;
}

void HistoryPage::setDevices(const QJsonArray &devices)
{
    const QString selected = ui->deviceCombo->currentData().toString();
    ui->deviceCombo->blockSignals(true);
    ui->deviceCombo->clear();
    m_deviceOnline.clear();
    for (const QJsonValue &value : devices) {
        const QJsonObject device = value.toObject();
        const QString id = device.value(QStringLiteral("device_id")).toString();
        const QString name = device.value(QStringLiteral("name")).toString();
        const QString type = device.value(QStringLiteral("device_type")).toString();
        const QString addedBy = device.value(QStringLiteral("added_by")).toString();
        const bool online = device.value(QStringLiteral("online")).toBool(true);
        m_deviceOnline.insert(id, online);

        QString itemText = name;
        if (itemText.isEmpty()) {
            itemText = id;
        }
        ui->deviceCombo->addItem(itemText, id);
        ui->deviceCombo->setItemData(ui->deviceCombo->count() - 1, type, Qt::UserRole + 1);
        ui->deviceCombo->setItemData(ui->deviceCombo->count() - 1,
            QStringLiteral("%1 (%2)%3 · %4").arg(name, id, addedBy.isEmpty() ? QString() : tr(" · Thêm bởi: %1").arg(addedBy),
                                                 online ? tr("Trực tuyến") : tr("Ngoại tuyến")),
            Qt::ToolTipRole);
    }
    const int previous = ui->deviceCombo->findData(selected);
    if (previous >= 0)
        ui->deviceCombo->setCurrentIndex(previous);
    ui->deviceCombo->blockSignals(false);
    requestCurrentHistory();
}

QDate HistoryPage::selectedDate() const
{
    if (ui && ui->dateCombo) {
        const QVariant val = ui->dateCombo->currentData();
        if (val.isValid() && val.canConvert<QDate>()) {
            return val.toDate();
        }
    }
    return m_selectedDate.isValid() ? m_selectedDate : QDate::currentDate();
}

void HistoryPage::rebuildDateOptions()
{
    if (!ui || !ui->dateCombo) return;

    ui->dateCombo->blockSignals(true);
    ui->dateCombo->clear();

    const QString period = ui->periodCombo->currentData().toString();
    const QDate today = QDate::currentDate();
    if (!m_selectedDate.isValid()) {
        m_selectedDate = today;
    }

    if (period == QStringLiteral("year")) {
        const int startYear = today.year();
        for (int y = startYear; y >= startYear - 5; --y) {
            const QDate d(y, 1, 1);
            const QString label = (y == today.year())
                ? tr("Năm %1 (Hiện tại)").arg(y)
                : tr("Năm %1").arg(y);
            ui->dateCombo->addItem(label, d);
        }
    } else if (period == QStringLiteral("month")) {
        const QDate curMonth(today.year(), today.month(), 1);
        for (int i = 0; i < 24; ++i) {
            const QDate d = curMonth.addMonths(-i);
            const QString label = (i == 0)
                ? tr("Tháng %1 (Hiện tại)").arg(d.toString(QStringLiteral("MM/yyyy")))
                : tr("Tháng %1").arg(d.toString(QStringLiteral("MM/yyyy")));
            ui->dateCombo->addItem(label, d);
        }
    } else {
        for (int i = 0; i < 31; ++i) {
            const QDate d = today.addDays(-i);
            QString label;
            if (i == 0) {
                label = tr("Hôm nay (%1)").arg(d.toString(QStringLiteral("dd/MM")));
            } else if (i == 1) {
                label = tr("Hôm qua (%1)").arg(d.toString(QStringLiteral("dd/MM")));
            } else {
                label = d.toString(QStringLiteral("dd/MM/yyyy"));
            }
            ui->dateCombo->addItem(label, d);
        }
    }

    int matchIdx = -1;
    for (int i = 0; i < ui->dateCombo->count(); ++i) {
        const QDate d = ui->dateCombo->itemData(i).toDate();
        if (period == QStringLiteral("year")) {
            if (d.year() == m_selectedDate.year()) { matchIdx = i; break; }
        } else if (period == QStringLiteral("month")) {
            if (d.year() == m_selectedDate.year() && d.month() == m_selectedDate.month()) { matchIdx = i; break; }
        } else {
            if (d == m_selectedDate) { matchIdx = i; break; }
        }
    }

    if (matchIdx >= 0) {
        ui->dateCombo->setCurrentIndex(matchIdx);
    } else {
        QString customLabel;
        if (period == QStringLiteral("year")) {
            customLabel = tr("Năm %1").arg(m_selectedDate.year());
        } else if (period == QStringLiteral("month")) {
            customLabel = tr("Tháng %1").arg(m_selectedDate.toString(QStringLiteral("MM/yyyy")));
        } else {
            customLabel = m_selectedDate.toString(QStringLiteral("dd/MM/yyyy"));
        }
        ui->dateCombo->insertItem(0, customLabel, m_selectedDate);
        ui->dateCombo->setCurrentIndex(0);
    }

    ui->dateCombo->blockSignals(false);
}

void HistoryPage::requestCurrentHistory()
{
    const QString deviceId = ui->deviceCombo->currentData().toString();
    const QString period = ui->periodCombo->currentData().toString();
    const QDate curDate = selectedDate();

    if (deviceId.isEmpty()) {
        ui->historyTable->setRowCount(0);
        ui->recordCountLabel->setText(tr("0 bản ghi"));
        m_primaryStat->setText(QStringLiteral("--"));
        m_secondaryStat->setText(QStringLiteral("--"));
        m_thirdStat->setText(tr("Chưa có thiết bị"));
        m_cachedKeys = {};
        m_cachedRows = {};
        updateMetricSelector();
        updateChart();
        return;
    }
    emit historyRequested(deviceId, period, curDate.toString(Qt::ISODate));
}

void HistoryPage::setPeriod(const QString &period)
{
    const int idx = ui->periodCombo->findData(period);
    if (idx >= 0) {
        ui->periodCombo->setCurrentIndex(idx);
    }
}

void HistoryPage::setDate(const QDate &date)
{
    if (!date.isValid()) return;
    m_selectedDate = date;
    rebuildDateOptions();
    requestCurrentHistory();
}

void HistoryPage::setViewTab(int tabIndex)
{
    if (tabIndex == 1) {
        ui->tableTabButton->setChecked(true);
        ui->viewStack->setCurrentIndex(1);
    } else {
        ui->chartTabButton->setChecked(true);
        ui->viewStack->setCurrentIndex(0);
    }
}

void HistoryPage::setMetric(const QString &key)
{
    m_selectedMetricKey = key;
    const int idx = m_metricCombo->findData(key);
    if (idx >= 0) {
        m_metricCombo->setCurrentIndex(idx);
    } else {
        updateChart();
    }
}

void HistoryPage::updateMetricSelector()
{
    m_metricCombo->blockSignals(true);
    m_metricCombo->clear();

    if (m_cachedKeys.isEmpty()) {
        m_metricCombo->hide();
        m_metricCombo->blockSignals(false);
        return;
    }

    QStringList plotableKeys;
    for (const QJsonValue &k : m_cachedKeys) {
        const QString keyStr = k.toString();
        if (keyStr != QStringLiteral("ir_detected") && keyStr != QStringLiteral("relay_state"))
            plotableKeys.append(keyStr);
    }

    if (!plotableKeys.isEmpty()) {
        for (const QString &key : plotableKeys) {
            QString cleanName = metricTitle(key);
            if (key == QStringLiteral("temperature_c")) cleanName = tr("Nhiệt độ LM35 (°C)");
            else if (key == QStringLiteral("sound_vpp")) cleanName = tr("Độ ồn MAX9814 (Vpp)");
            m_metricCombo->addItem(tr("%1").arg(cleanName), key);
        }
        int idx = m_metricCombo->findData(m_selectedMetricKey);
        if (idx >= 0 && m_selectedMetricKey != QStringLiteral("all")) {
            m_metricCombo->setCurrentIndex(idx);
        } else {
            int defaultIdx = m_metricCombo->findData(QStringLiteral("temperature_c"));
            if (defaultIdx < 0)
                defaultIdx = 0;
            m_metricCombo->setCurrentIndex(defaultIdx);
            m_selectedMetricKey = m_metricCombo->itemData(defaultIdx).toString();
        }
        const bool isChartTab = (ui->viewStack->currentIndex() == 0);
        m_metricCombo->setVisible(isChartTab);
        if (m_zoomBtn) m_zoomBtn->setVisible(isChartTab);
    } else {
        m_metricCombo->hide();
        if (m_zoomBtn) m_zoomBtn->hide();
        m_selectedMetricKey.clear();
    }
    m_metricCombo->blockSignals(false);
}

void HistoryPage::setHistory(const QJsonObject &history)
{
    m_cachedPeriod = history.value(QStringLiteral("period")).toString();
    m_cachedSelectedDate = history.value(QStringLiteral("selected_date")).toString();
    m_cachedKeys = history.value(QStringLiteral("metric_keys")).toArray();
    m_cachedRows = history.value(QStringLiteral("data")).toArray();
    const QJsonArray keys = m_cachedKeys;
    const QJsonArray rows = m_cachedRows;

    ui->historyTable->clear();
    ui->historyTable->setRowCount(rows.size());
    ui->historyTable->setColumnCount(keys.size() + 1);
    ui->historyTable->setAlternatingRowColors(false);

    QStringList headers;
    if (m_cachedPeriod == QStringLiteral("day")) {
        headers.append(tr("Thời gian (Giây)"));
        for (const QJsonValue &key : keys)
            headers.append(metricTitle(key.toString()));
    } else if (m_cachedPeriod == QStringLiteral("month")) {
        headers.append(tr("Ngày (Tổng hợp)"));
        for (const QJsonValue &key : keys)
            headers.append(tr("%1 (TB)").arg(metricTitle(key.toString())));
    } else if (m_cachedPeriod == QStringLiteral("year")) {
        headers.append(tr("Tháng (Tổng hợp)"));
        for (const QJsonValue &key : keys)
            headers.append(tr("%1 (TB)").arg(metricTitle(key.toString())));
    } else {
        headers.append(tr("Thời gian"));
        for (const QJsonValue &key : keys)
            headers.append(metricTitle(key.toString()));
    }
    ui->historyTable->setHorizontalHeaderLabels(headers);

    for (int row = 0; row < rows.size(); ++row) {
        const QJsonObject entry = rows.at(row).toObject();
        const QString recordedAtStr = entry.value(QStringLiteral("recorded_at")).toString();

        QString cellTimeText;
        if (m_cachedPeriod == QStringLiteral("day")) {
            QDateTime time = QDateTime::fromString(recordedAtStr, Qt::ISODateWithMs);
            if (!time.isValid())
                time = QDateTime::fromString(recordedAtStr, Qt::ISODate);
            if (!time.isValid())
                time = QDateTime::fromString(recordedAtStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            time = time.toLocalTime();
            cellTimeText = time.isValid() ? time.toString(QStringLiteral("HH:mm:ss")) : recordedAtStr;
        } else if (m_cachedPeriod == QStringLiteral("month")) {
            const QString label = entry.value(QStringLiteral("label")).toString();
            const QDate d = QDate::fromString(recordedAtStr, QStringLiteral("yyyy-MM-dd"));
            cellTimeText = !label.isEmpty() ? label : (d.isValid() ? tr("Ngày %1").arg(d.toString(QStringLiteral("dd/MM/yyyy"))) : recordedAtStr);
        } else if (m_cachedPeriod == QStringLiteral("year")) {
            const QString label = entry.value(QStringLiteral("label")).toString();
            const QDate m = QDate::fromString(recordedAtStr + QStringLiteral("-01"), QStringLiteral("yyyy-MM-dd"));
            cellTimeText = !label.isEmpty() ? label : (m.isValid() ? tr("Tháng %1").arg(m.toString(QStringLiteral("MM/yyyy"))) : recordedAtStr);
        } else {
            cellTimeText = recordedAtStr;
        }

        const QColor rowBg(row % 2 == 0 ? "#0c1630" : "#111d3d");
        auto *timeItem = new QTableWidgetItem(cellTimeText);
        timeItem->setForeground(QBrush(QColor("#ffffff")));
        timeItem->setBackground(QBrush(rowBg));
        ui->historyTable->setItem(row, 0, timeItem);

        const QJsonObject metrics = entry.value(QStringLiteral("metrics")).toObject();
        for (int column = 0; column < keys.size(); ++column) {
            const QJsonValue value = metrics.value(keys.at(column).toString());
            auto *valItem = new QTableWidgetItem(
                value.isDouble() ? QString::number(value.toDouble(), 'f', 2) : QStringLiteral("—"));
            valItem->setForeground(QBrush(QColor("#38bdf8")));
            valItem->setBackground(QBrush(rowBg));
            ui->historyTable->setItem(row, column + 1, valItem);
        }
    }

    const int total = history.value(QStringLiteral("total")).toInt();
    if (m_cachedPeriod == QStringLiteral("day")) {
        ui->recordCountLabel->setText(tr("● %1 bản ghi").arg(total));
    } else if (m_cachedPeriod == QStringLiteral("month")) {
        ui->recordCountLabel->setText(tr("%1 ngày").arg(total));
    } else if (m_cachedPeriod == QStringLiteral("year")) {
        ui->recordCountLabel->setText(tr("%1 tháng").arg(total));
    } else {
        ui->recordCountLabel->setText(tr("%1 bản ghi").arg(total));
    }

    const QJsonObject averages = history.value(QStringLiteral("averages")).toObject();
    if (averages.value(QStringLiteral("temperature_c")).isDouble()) {
        if (m_primaryStatTitle) m_primaryStatTitle->setText(tr("NHIỆT ĐỘ LM35 (TB)"));
        m_primaryStat->setText(QStringLiteral("%1 °C").arg(QString::number(averages.value(QStringLiteral("temperature_c")).toDouble(), 'f', 1)));
        m_primaryStat->setStyleSheet(QStringLiteral("color: #38bdf8; font-size: 14px; font-weight: 900;"));
    } else {
        if (m_primaryStatTitle) m_primaryStatTitle->setText(tr("NHIỆT ĐỘ LM35"));
        m_primaryStat->setText(QStringLiteral("-- °C"));
        m_primaryStat->setStyleSheet(QStringLiteral("color: #94a3b8; font-size: 14px; font-weight: 900;"));
    }

    if (averages.value(QStringLiteral("sound_vpp")).isDouble()) {
        if (m_secondaryStatTitle) m_secondaryStatTitle->setText(tr("ĐỘ ỒN MAX9814 (TB)"));
        m_secondaryStat->setText(QStringLiteral("%1 Vpp").arg(QString::number(averages.value(QStringLiteral("sound_vpp")).toDouble(), 'f', 2)));
        m_secondaryStat->setStyleSheet(QStringLiteral("color: #10b981; font-size: 14px; font-weight: 900;"));
    } else {
        if (m_secondaryStatTitle) m_secondaryStatTitle->setText(tr("ĐỘ ỒN MAX9814"));
        m_secondaryStat->setText(QStringLiteral("-- Vpp"));
        m_secondaryStat->setStyleSheet(QStringLiteral("color: #94a3b8; font-size: 14px; font-weight: 900;"));
    }

    if (m_summaryStatTitle) {
        if (m_cachedPeriod == QStringLiteral("month")) {
            m_summaryStatTitle->setText(tr("SỐ NGÀY CÓ DỮ LIỆU"));
            m_thirdStat->setText(tr("%1 ngày").arg(total));
        } else if (m_cachedPeriod == QStringLiteral("year")) {
            m_summaryStatTitle->setText(tr("SỐ THÁNG CÓ DỮ LIỆU"));
            m_thirdStat->setText(tr("%1 tháng").arg(total));
        } else {
            m_summaryStatTitle->setText(tr("TỔNG SỐ BẢN GHI"));
            m_thirdStat->setText(tr("%1 mẫu").arg(total));
        }
    }
    m_thirdStat->setStyleSheet(QStringLiteral("color: #ecf2ff; font-size: 14px; font-weight: 900;"));

    updateMetricSelector();
    updateChart();
}

void HistoryPage::updateChart()
{
    m_chart->removeAllSeries();
    const QList<QAbstractAxis *> oldAxes = m_chart->axes();
    for (QAbstractAxis *axis : oldAxes) {
        m_chart->removeAxis(axis);
        axis->deleteLater();
    }
    const QJsonArray &keys = m_cachedKeys;
    const QJsonArray &rows = m_cachedRows;

    if (keys.isEmpty() || rows.isEmpty()) {
        m_chartTitle->setText(tr("Không có dữ liệu"));
        m_chartHint->setText(tr("Không có dữ liệu trong khoảng thời gian đã chọn."));
        return;
    }

    QString activeKey = m_selectedMetricKey;
    if (activeKey.isEmpty() || activeKey == QStringLiteral("all")) {
        for (const QJsonValue &k : keys) {
            const QString keyStr = k.toString();
            if (keyStr != QStringLiteral("ir_detected") && keyStr != QStringLiteral("relay_state")) {
                activeKey = keyStr;
                break;
            }
        }
        m_selectedMetricKey = activeKey;
    }

    if (activeKey.isEmpty()) {
        m_chartTitle->setText(tr("Không có dữ liệu biểu đồ"));
        return;
    }

    const QString currentPeriod = m_cachedPeriod.isEmpty()
        ? ui->periodCombo->currentData().toString()
        : m_cachedPeriod;

    auto *barSeries = new QBarSeries(m_chart);

    QStringList categories;
    QList<QJsonObject> chronologicalRows;
    for (int r = rows.size() - 1; r >= 0; --r) {
        chronologicalRows.append(rows.at(r).toObject());
    }

    double overallMin = std::numeric_limits<double>::max();
    double overallMax = std::numeric_limits<double>::lowest();

    auto *barSet = new QBarSet(metricTitle(activeKey));
    const QColor col = getColorForKey(activeKey, 0);
    barSet->setColor(col);
    barSet->setBorderColor(col.lighter(120));

    if (currentPeriod == QStringLiteral("month")) {
        for (const QJsonObject &entry : chronologicalRows) {
            const QString label = entry.value(QStringLiteral("label")).toString();
            const QString recStr = entry.value(QStringLiteral("recorded_at")).toString();
            const QDate d = QDate::fromString(recStr, QStringLiteral("yyyy-MM-dd"));
            const QString cat = !label.isEmpty() ? label : (d.isValid() ? QStringLiteral("Ngày %1").arg(d.toString(QStringLiteral("dd/MM"))) : recStr);
            categories << cat;
            const QJsonObject m = entry.value(QStringLiteral("metrics")).toObject();
            const double val = m.value(activeKey).toDouble(0.0);
            *barSet << val;
            overallMin = qMin(overallMin, val);
            overallMax = qMax(overallMax, val);
        }
        m_chartTitle->setText(tr("Biểu đồ cột %1 theo ngày · Tháng %2").arg(metricShortName(activeKey), m_cachedSelectedDate.left(7)));
        m_chartHint->setText(tr("Mỗi cột đại diện cho giá trị %1 trung bình của một ngày trong tháng.").arg(metricTitle(activeKey)));
    } else if (currentPeriod == QStringLiteral("year")) {
        for (const QJsonObject &entry : chronologicalRows) {
            const QString label = entry.value(QStringLiteral("label")).toString();
            const QString recStr = entry.value(QStringLiteral("recorded_at")).toString();
            const int mNum = recStr.mid(5, 2).toInt();
            const QString cat = !label.isEmpty() ? label : QStringLiteral("Tháng %1").arg(mNum, 2, 10, QChar('0'));
            categories << cat;
            const QJsonObject m = entry.value(QStringLiteral("metrics")).toObject();
            const double val = m.value(activeKey).toDouble(0.0);
            *barSet << val;
            overallMin = qMin(overallMin, val);
            overallMax = qMax(overallMax, val);
        }
        m_chartTitle->setText(tr("Biểu đồ cột %1 theo tháng · Năm %2").arg(metricShortName(activeKey), m_cachedSelectedDate.left(4)));
        m_chartHint->setText(tr("Mỗi cột đại diện cho giá trị %1 trung bình của một tháng trong năm.").arg(metricTitle(activeKey)));
    } else {
        // currentPeriod == "day" -> Thông minh nhóm mẫu theo khoảng thời gian để biểu đồ luôn đẹp và không bao giờ dồn cột
        struct RawSample {
            QDateTime dt;
            double val;
        };
        QList<RawSample> rawSamples;
        for (const QJsonObject &entry : chronologicalRows) {
            const QJsonValue v = entry.value(QStringLiteral("metrics")).toObject().value(activeKey);
            if (!v.isDouble()) continue;
            QString recStr = entry.value(QStringLiteral("recorded_at")).toString();
            QDateTime dt = QDateTime::fromString(recStr, Qt::ISODateWithMs);
            if (!dt.isValid()) dt = QDateTime::fromString(recStr, Qt::ISODate);
            if (!dt.isValid()) dt = QDateTime::fromString(recStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            dt = dt.toLocalTime();
            if (dt.isValid()) {
                rawSamples.append({dt, v.toDouble()});
            }
        }

        if (rawSamples.isEmpty()) {
            m_chartTitle->setText(tr("Không có dữ liệu biểu đồ"));
            return;
        }

        if (rawSamples.size() <= 14) {
            for (const auto &s : rawSamples) {
                categories.append(s.dt.toString(QStringLiteral("HH:mm")));
                *barSet << s.val;
                overallMin = qMin(overallMin, s.val);
                overallMax = qMax(overallMax, s.val);
            }
        } else {
            int binMinutes = 5;
            QMap<QString, QPair<double, int>> bins;
            QStringList binOrder;

            for (int bm : {5, 10, 15, 30, 60, 120}) {
                binMinutes = bm;
                bins.clear();
                binOrder.clear();
                for (const auto &s : rawSamples) {
                    int m = s.dt.time().minute();
                    int b = (m / binMinutes) * binMinutes;
                    QTime binTime(s.dt.time().hour(), b, 0);
                    QString binKey = binTime.toString(QStringLiteral("HH:mm"));
                    if (!bins.contains(binKey)) {
                        binOrder.append(binKey);
                    }
                    bins[binKey].first += s.val;
                    bins[binKey].second += 1;
                }
                if (binOrder.size() <= 16) {
                    break;
                }
            }

            for (const QString &binKey : binOrder) {
                categories.append(binKey);
                double avg = bins[binKey].first / bins[binKey].second;
                *barSet << avg;
                overallMin = qMin(overallMin, avg);
                overallMax = qMax(overallMax, avg);
            }
        }

        m_chartTitle->setText(tr("Biểu đồ cột %1 · Ngày %2").arg(metricShortName(activeKey), m_cachedSelectedDate.left(10)));
        m_chartHint->setText(tr("Mỗi cột đại diện cho giá trị %1 trung bình theo mốc thời gian trong ngày.").arg(metricTitle(activeKey)));
    }

    const int numBars = categories.size();
    double barWidth = 0.60;
    if (numBars <= 1) {
        barWidth = 0.10;
    } else if (numBars <= 3) {
        barWidth = 0.22;
    } else if (numBars <= 6) {
        barWidth = 0.38;
    } else if (numBars <= 12) {
        barWidth = 0.52;
    } else {
        barWidth = 0.65;
    }
    barSeries->setBarWidth(barWidth);

    barSeries->append(barSet);
    m_chart->addSeries(barSeries);

    auto *axisX = new QBarCategoryAxis(m_chart);
    QFont axisFont;
    axisFont.setPixelSize(9);
    axisX->setLabelsFont(axisFont);
    axisX->setLabelsColor(QColor("#94a3b8"));
    axisX->setGridLineColor(QColor("#1c2b54"));
    if (categories.size() > 8) {
        axisX->setLabelsAngle(-40);
    }
    axisX->append(categories);
    m_chart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    auto *axisY = new QValueAxis(m_chart);
    axisY->setLabelsFont(axisFont);
    axisY->setLabelsColor(col);
    axisY->setTitleBrush(col);
    axisY->setTitleFont(axisFont);
    axisY->setGridLineColor(QColor("#1c2b54"));
    axisY->setTitleText(compactMetricTitle(activeKey));

    if (overallMin > overallMax) {
        overallMin = 0.0;
        overallMax = 10.0;
    }

    if (activeKey == QStringLiteral("temperature_c")) {
        axisY->setRange(qMax(0.0, overallMin - 2.0), qMin(60.0, qMax(40.0, overallMax + 3.0)));
        axisY->setLabelFormat("%.1f");
    } else if (activeKey == QStringLiteral("sound_vpp")) {
        axisY->setRange(0.0, qMax(1.0, overallMax + 0.2));
        axisY->setLabelFormat("%.2f");
    } else {
        const double diff = overallMax - overallMin;
        const double padding = qMax(0.5, (diff == 0.0 ? 1.0 : diff * 0.15));
        axisY->setRange(qMax(0.0, overallMin - padding), overallMax + padding);
        axisY->setLabelFormat("%.1f");
    }
    m_chart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);
}

void HistoryPage::openChartZoomDialog(const QString &initialMetricKey)
{
    if (m_cachedKeys.isEmpty() || m_cachedRows.isEmpty()) return;

    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("chartZoomDialog"));
    dialog.setWindowTitle(tr("Phóng to biểu đồ lịch sử"));
    dialog.setModal(true);
    dialog.setStyleSheet(
        "QDialog#chartZoomDialog { background-color: #070d1e; color: #ecf2ff; } "
        "QLabel#chartZoomTitle { color: #f8fafc; font-size: 16px; font-weight: 900; } "
        "QLabel#chartZoomSubtitle { color: #38bdf8; font-size: 11px; font-weight: 700; } "
        "QComboBox#historyMetricCombo { background-color: #0f1c3f; color: #ffffff; border: 1px solid #233870; border-radius: 6px; padding: 4px 8px; font-size: 11px; font-weight: 700; } "
        "QLabel#chartZoomStatBadge { background: #0e1938; color: #38bdf8; border: 1px solid #1c2b54; border-radius: 6px; padding: 4px 8px; font-size: 11px; font-weight: 800; } "
        "QPushButton#chartZoomCloseBtn { background: #1e293b; color: #94a3b8; border: 1px solid #334155; border-radius: 18px; font-size: 14px; font-weight: 900; } "
        "QPushButton#chartZoomBottomClose { background: #0284c7; color: #ffffff; border: none; border-radius: 6px; font-size: 12px; font-weight: 800; padding: 6px 14px; }"
    );

    const int availableWidth = parentWidget() ? parentWidget()->width() - 16 : 760;
    const int availableHeight = parentWidget() ? parentWidget()->height() - 16 : 460;
    dialog.resize(qMax(360, availableWidth), qMax(280, availableHeight));

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(10);

    auto *headerLayout = new QHBoxLayout;
    headerLayout->setSpacing(10);

    auto *titleBlock = new QVBoxLayout;
    titleBlock->setSpacing(2);
    auto *dialogTitle = new QLabel(tr("Biểu đồ chi tiết"), &dialog);
    dialogTitle->setObjectName(QStringLiteral("chartZoomTitle"));
    auto *dialogSubtitle = new QLabel(ui->deviceCombo->currentText(), &dialog);
    dialogSubtitle->setObjectName(QStringLiteral("chartZoomSubtitle"));
    titleBlock->addWidget(dialogTitle);
    titleBlock->addWidget(dialogSubtitle);
    headerLayout->addLayout(titleBlock, 1);

    auto *metricCombo = new QComboBox(&dialog);
    metricCombo->setObjectName(QStringLiteral("historyMetricCombo"));
    metricCombo->setMinimumWidth(180);

    QStringList plotableKeys;
    for (const QJsonValue &k : m_cachedKeys) {
        const QString keyStr = k.toString();
        if (keyStr != QStringLiteral("ir_detected") && keyStr != QStringLiteral("relay_state"))
            plotableKeys.append(keyStr);
    }

    for (const QString &key : plotableKeys) {
        metricCombo->addItem(tr("%1").arg(metricTitle(key)), key);
    }

    QString selectedKey = initialMetricKey.isEmpty() ? m_selectedMetricKey : initialMetricKey;
    if (selectedKey.isEmpty() || selectedKey == QStringLiteral("all") || metricCombo->findData(selectedKey) < 0)
        selectedKey = plotableKeys.isEmpty() ? QString() : plotableKeys.first();

    int foundIdx = metricCombo->findData(selectedKey);
    if (foundIdx >= 0)
        metricCombo->setCurrentIndex(foundIdx);

    headerLayout->addWidget(metricCombo, 0, Qt::AlignVCenter);

    auto *closeBtn = new QPushButton(QStringLiteral("Đóng"), &dialog);
    closeBtn->setObjectName(QStringLiteral("chartZoomCloseBtn"));
    closeBtn->setFixedSize(36, 36);
    closeBtn->setCursor(Qt::PointingHandCursor);
    headerLayout->addWidget(closeBtn, 0, Qt::AlignVCenter);
    root->addLayout(headerLayout);

    auto *zoomChart = new QChart;
    zoomChart->setAnimationOptions(QChart::SeriesAnimations);
    zoomChart->legend()->setAlignment(Qt::AlignBottom);
    zoomChart->legend()->setVisible(true);

    auto *zoomChartView = new QChartView(zoomChart, &dialog);
    zoomChartView->setRenderHint(QPainter::Antialiasing);
    zoomChartView->setStyleSheet("background: #070d1e; border: none;");
    root->addWidget(zoomChartView, 1);

    auto *statsLayout = new QHBoxLayout;
    statsLayout->setSpacing(10);

    auto *countBadge = new QLabel(tr("Tổng: %1 bản ghi").arg(m_cachedRows.size()), &dialog);
    auto *minBadge = new QLabel(&dialog);
    auto *maxBadge = new QLabel(&dialog);
    auto *avgBadge = new QLabel(&dialog);
    for (QLabel *badge : {countBadge, minBadge, maxBadge, avgBadge}) {
        badge->setObjectName(QStringLiteral("chartZoomStatBadge"));
    }
    statsLayout->addWidget(countBadge);
    statsLayout->addWidget(minBadge);
    statsLayout->addWidget(maxBadge);
    statsLayout->addWidget(avgBadge);
    statsLayout->addStretch();

    auto *bottomClose = new QPushButton(tr("Đóng"), &dialog);
    bottomClose->setObjectName(QStringLiteral("chartZoomBottomClose"));
    bottomClose->setCursor(Qt::PointingHandCursor);
    statsLayout->addWidget(bottomClose);
    root->addLayout(statsLayout);

    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(bottomClose, &QPushButton::clicked, &dialog, &QDialog::accept);

    const auto renderZoomChart = [this, zoomChart, metricCombo, dialogTitle, minBadge, maxBadge, avgBadge]() {
        zoomChart->removeAllSeries();
        for (QAbstractAxis *axis : zoomChart->axes()) {
            zoomChart->removeAxis(axis);
            axis->deleteLater();
        }

        const QString activeMetric = metricCombo->currentData().toString();
        const QJsonArray &rows = m_cachedRows;

        dialogTitle->setText(tr("Biểu đồ chi tiết: %1").arg(metricTitle(activeMetric)));
        const QColor seriesColor = getColorForKey(activeMetric, 0);

        double overallMin = std::numeric_limits<double>::max();
        double overallMax = std::numeric_limits<double>::lowest();
        double overallSum = 0;
        int overallCount = 0;

        auto *barSet = new QBarSet(metricTitle(activeMetric));
        barSet->setColor(seriesColor);
        barSet->setBorderColor(seriesColor.lighter(120));

        QStringList categories;
        for (int r = rows.size() - 1; r >= 0; --r) {
            const QJsonObject entry = rows.at(r).toObject();
            const QJsonValue v = entry.value(QStringLiteral("metrics")).toObject().value(activeMetric);
            if (!v.isDouble()) continue;
            const double val = v.toDouble();
            QString recordedAtStr = entry.value(QStringLiteral("recorded_at")).toString();
            QString cat;
            if (m_cachedPeriod == QStringLiteral("month")) {
                const QString label = entry.value(QStringLiteral("label")).toString();
                const QDate d = QDate::fromString(recordedAtStr, QStringLiteral("yyyy-MM-dd"));
                cat = !label.isEmpty() ? label : (d.isValid() ? QStringLiteral("Ngày %1").arg(d.toString(QStringLiteral("dd/MM"))) : recordedAtStr);
            } else if (m_cachedPeriod == QStringLiteral("year")) {
                const QString label = entry.value(QStringLiteral("label")).toString();
                const int mNum = recordedAtStr.mid(5, 2).toInt();
                cat = !label.isEmpty() ? label : QStringLiteral("Tháng %1").arg(mNum, 2, 10, QChar('0'));
            } else {
                QDateTime dt = QDateTime::fromString(recordedAtStr, Qt::ISODateWithMs);
                if (!dt.isValid()) dt = QDateTime::fromString(recordedAtStr, Qt::ISODate);
                dt = dt.toLocalTime();
                cat = dt.isValid() ? dt.toString(QStringLiteral("HH:mm")) : recordedAtStr;
            }
            categories.append(cat);
            *barSet << val;
            overallMin = qMin(overallMin, val);
            overallMax = qMax(overallMax, val);
            overallSum += val;
            ++overallCount;
        }

        if (overallCount > 0) {
            minBadge->setText(tr("Min: %1 %2").arg(QString::number(overallMin, 'f', 1), compactMetricTitle(activeMetric)));
            maxBadge->setText(tr("Max: %1 %2").arg(QString::number(overallMax, 'f', 1), compactMetricTitle(activeMetric)));
            avgBadge->setText(tr("TB: %1 %2").arg(QString::number(overallSum / overallCount, 'f', 1), compactMetricTitle(activeMetric)));
        }

        auto *barSeries = new QBarSeries(zoomChart);
        barSeries->append(barSet);
        zoomChart->addSeries(barSeries);

        auto *axisX = new QBarCategoryAxis(zoomChart);
        QFont axisFont;
        axisFont.setPixelSize(9);
        axisX->setLabelsFont(axisFont);
        axisX->setLabelsColor(QColor("#94a3b8"));
        if (categories.size() > 8) {
            axisX->setLabelsAngle(-40);
        }
        axisX->append(categories);
        zoomChart->addAxis(axisX, Qt::AlignBottom);
        barSeries->attachAxis(axisX);

        auto *axisY = new QValueAxis(zoomChart);
        axisY->setLabelsFont(axisFont);
        axisY->setLabelsColor(seriesColor);
        axisY->setTitleText(compactMetricTitle(activeMetric));
        if (overallMin > overallMax) { overallMin = 0; overallMax = 10; }
        const double diff = overallMax - overallMin;
        const double padding = qMax(0.5, (diff == 0.0 ? 1.0 : diff * 0.15));
        axisY->setRange(qMax(0.0, overallMin - padding), overallMax + padding);
        axisY->setLabelFormat("%.1f");
        zoomChart->addAxis(axisY, Qt::AlignLeft);
        barSeries->attachAxis(axisY);
    };

    connect(metricCombo, &QComboBox::currentIndexChanged, &dialog, renderZoomChart);
    renderZoomChart();

    dialog.exec();
}

QString HistoryPage::currentDeviceType() const
{
    return ui->deviceCombo->currentData(Qt::UserRole + 1).toString();
}

QString HistoryPage::metricTitle(const QString &key)
{
    static const QHash<QString, QString> names{
        {"temperature_c", tr("Nhiệt độ LM35 (°C)")},
        {"sound_vpp", tr("Độ ồn MAX9814 (Vpp)")}};
    return names.value(key, key);
}

QString HistoryPage::compactMetricTitle(const QString &key)
{
    static const QHash<QString, QString> names{
        {"temperature_c", tr("°C")},
        {"sound_vpp", tr("Vpp")}};
    return names.value(key, key);
}

QString HistoryPage::metricUnit(const QString &key)
{
    return compactMetricTitle(key);
}
