#include "HistoryPage.h"

#include "ui_HistoryPage.h"

#include <QAreaSeries>
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
#include <QLinearGradient>
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

static QString canonicalMetricKey(const QString &rawKey)
{
    if (rawKey == QStringLiteral("lux") || rawKey == QStringLiteral("light_lux"))
        return QStringLiteral("light_lux");
    if (rawKey == QStringLiteral("motion") || rawKey == QStringLiteral("motion_detected") || rawKey == QStringLiteral("detech"))
        return QStringLiteral("motion_detected");
    if (rawKey == QStringLiteral("relay") || rawKey == QStringLiteral("relay_on"))
        return QStringLiteral("relay");
    if (rawKey == QStringLiteral("temp") || rawKey == QStringLiteral("temperature_c"))
        return QStringLiteral("temperature_c");
    if (rawKey == QStringLiteral("humidity") || rawKey == QStringLiteral("humidity_percent"))
        return QStringLiteral("humidity_percent");
    return rawKey;
}

static QStringList getCanonicalMetricKeys(const QJsonArray &rawKeys)
{
    QStringList canonicalList;
    for (const QJsonValue &k : rawKeys) {
        const QString canon = canonicalMetricKey(k.toString());
        if (!canonicalList.contains(canon)) {
            canonicalList.append(canon);
        }
    }
    return canonicalList;
}

static bool extractMetricValue(const QJsonObject &metrics, const QString &canonicalKey, double &outVal)
{
    auto tryKey = [&](const QString &k) -> bool {
        if (metrics.contains(k)) {
            const QJsonValue v = metrics.value(k);
            if (v.isDouble()) {
                outVal = v.toDouble();
                return true;
            }
            if (v.isBool()) {
                outVal = v.toBool() ? 1.0 : 0.0;
                return true;
            }
            if (v.isString()) {
                const QString s = v.toString().trimmed().toLower();
                if (s == QStringLiteral("true") || s == QStringLiteral("on") || s == QStringLiteral("1")) {
                    outVal = 1.0;
                    return true;
                }
                if (s == QStringLiteral("false") || s == QStringLiteral("off") || s == QStringLiteral("0")) {
                    outVal = 0.0;
                    return true;
                }
                bool ok = false;
                double d = s.toDouble(&ok);
                if (ok) {
                    outVal = d;
                    return true;
                }
            }
        }
        return false;
    };

    if (canonicalKey == QStringLiteral("light_lux")) {
        return tryKey(QStringLiteral("light_lux")) || tryKey(QStringLiteral("lux"));
    }
    if (canonicalKey == QStringLiteral("motion_detected")) {
        return tryKey(QStringLiteral("motion_detected")) || tryKey(QStringLiteral("motion")) || tryKey(QStringLiteral("detech"));
    }
    if (canonicalKey == QStringLiteral("relay")) {
        return tryKey(QStringLiteral("relay")) || tryKey(QStringLiteral("relay_on"));
    }
    if (canonicalKey == QStringLiteral("temperature_c")) {
        return tryKey(QStringLiteral("temperature_c")) || tryKey(QStringLiteral("temp"));
    }
    if (canonicalKey == QStringLiteral("humidity_percent")) {
        return tryKey(QStringLiteral("humidity_percent")) || tryKey(QStringLiteral("humidity"));
    }
    return tryKey(canonicalKey);
}

HistoryPage::HistoryPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::HistoryPage),
      m_chart(new QChart),
      m_chartView(new QChartView(m_chart, this)),
      m_primaryStat(new QLabel(this)),
      m_secondaryStat(new QLabel(this)),
      m_thirdStat(new QLabel(this)),
      m_chartHint(new QLabel(this)),
      m_headerSubtitle(new QLabel(this)),
      m_analyticsGrid(new QGridLayout),
      m_chartCard(new QFrame(this)),
      m_primaryStatCard(nullptr),
      m_secondaryStatCard(nullptr),
      m_summaryStatCard(nullptr)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral(
        "HistoryPage { background-color: #060b17; }"
        "QLabel { color: #e2e8f0; font-family: 'Segoe UI', sans-serif; }"
        "QFrame#historyChartCard { background-color: #0d1527; border: 1px solid #1e293b; border-radius: 8px; }"
        "QFrame#historyStatCard { background-color: #0f172a; border: 1px solid #1e293b; border-radius: 8px; }"
        "QFrame#historyStatCard:hover { border-color: #f59e0b; background-color: #111c33; }"
        "QLabel#historyCardTitle { color: #f8fafc; font-size: 11px; font-weight: 800; }"
        "QLabel#historyStatTitle { color: #94a3b8; font-size: 9px; font-weight: 700; }"
        "QLabel#historyStatValue { color: #38bdf8; font-size: 12px; font-weight: 900; }"
        "QLabel#historyStatIcon { font-size: 14px; color: #f59e0b; }"
        "QPushButton#deviceViewTabButton { background: #0f172a; color: #64748b; border: 1px solid #1e293b; border-radius: 5px; font-size: 10px; font-weight: 700; padding: 4px 10px; }"
        "QPushButton#deviceViewTabButton:hover { background: #1e293b; color: #ffffff; }"
        "QPushButton#deviceViewTabButton:checked { background: #0284c7; color: #ffffff; border-color: #38bdf8; }"
        "QPushButton#historySearchButton { background: #f59e0b; color: #020617; border: none; border-radius: 5px; font-size: 10px; font-weight: 800; padding: 4px 12px; }"
        "QPushButton#historySearchButton:hover { background: #fbbf24; }"
        "QPushButton#historyZoomButton { background: #1e293b; color: #38bdf8; border: 1px solid #0284c7; border-radius: 5px; font-size: 9px; font-weight: 700; padding: 3px 8px; }"
        "QComboBox, QDateEdit { background: #0f172a; color: #f8fafc; border: 1px solid #1e293b; border-radius: 5px; padding: 2px 6px; font-size: 10px; }"
        "QTableWidget#historyTableSmart { background-color: #0b1329; alternate-background-color: #111c38; border: 1px solid #1e293b; border-radius: 6px; gridline-color: #1e293b; color: #f8fafc; font-size: 11px; }"
        "QTableWidget#historyTableSmart::item { padding: 6px 8px; color: #f8fafc; }"
        "QTableWidget#historyTableSmart::item:alternate { background-color: #111c38; color: #f8fafc; }"
        "QTableWidget#historyTableSmart::item:selected { background-color: #0284c7; color: #ffffff; }"
        "QHeaderView::section { background-color: #080d1a; color: #94a3b8; font-weight: 800; padding: 6px 8px; border: none; border-bottom: 1.5px solid #1e293b; font-size: 10px; }"
    ));
    m_chart->setBackgroundBrush(QColor("#0d1527"));
    m_chart->setPlotAreaBackgroundBrush(QColor("#080e1e"));
    m_chart->setPlotAreaBackgroundVisible(true);
    m_chart->legend()->setLabelColor(QColor("#94a3b8"));

    ui->recordCountLabel->setObjectName(QStringLiteral("historyRecordBadge"));
    ui->chartTabButton->setObjectName(QStringLiteral("deviceViewTabButton"));
    ui->tableTabButton->setObjectName(QStringLiteral("deviceViewTabButton"));
    ui->deviceCombo->setObjectName(QStringLiteral("historyDeviceCombo"));
    ui->periodCombo->setObjectName(QStringLiteral("historyPeriodCombo"));
    ui->dateEdit->setObjectName(QStringLiteral("historyDateEdit"));
    ui->searchButton->setObjectName(QStringLiteral("historySearchButton"));
    ui->filterLayout->setSpacing(6);

    auto *tabGroup = new QButtonGroup(this);
    tabGroup->addButton(ui->chartTabButton);
    tabGroup->addButton(ui->tableTabButton);
    tabGroup->setExclusive(true);

    connect(ui->chartTabButton, &QPushButton::clicked, this, [this] {
        ui->viewStack->setCurrentIndex(0);
    });
    connect(ui->tableTabButton, &QPushButton::clicked, this, [this] {
        ui->viewStack->setCurrentIndex(1);
    });

    ui->historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->historyTable->verticalHeader()->hide();
    ui->historyTable->setObjectName(QStringLiteral("historyTableSmart"));

    auto makeStatCard = [this](const QString &title, QLabel *value, const QString &icon) {
        auto *card = new QFrame(this);
        card->setObjectName(QStringLiteral("historyStatCard"));
        card->setCursor(Qt::PointingHandCursor);
        card->setToolTip(tr("Bấm để xem phóng to biểu đồ chỉ số này"));
        auto *layout = new QHBoxLayout(card);
        layout->setContentsMargins(6, 2, 6, 2);
        layout->setSpacing(4);
        auto *iconLabel = new QLabel(icon, card);
        iconLabel->setObjectName(QStringLiteral("historyStatIcon"));
        iconLabel->setAlignment(Qt::AlignCenter);

        auto *textLayout = new QVBoxLayout;
        textLayout->setContentsMargins(0, 0, 0, 0);
        textLayout->setSpacing(0);
        auto *titleLabel = new QLabel(title, card);
        titleLabel->setObjectName(QStringLiteral("historyStatTitle"));
        value->setObjectName(QStringLiteral("historyStatValue"));
        value->setText(QStringLiteral("--"));
        value->setWordWrap(false);
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(value);

        layout->addWidget(iconLabel, 0, Qt::AlignVCenter);
        layout->addLayout(textLayout, 1);
        card->installEventFilter(this);
        return card;
    };

    m_chartView->setObjectName(QStringLiteral("historyChart"));
    m_chartView->setMinimumHeight(190);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setCursor(Qt::PointingHandCursor);
    m_chartView->setToolTip(tr("Chạm vào biểu đồ để phóng to"));
    m_chartView->viewport()->installEventFilter(this);
    m_chart->setTitle(QString());
    m_chart->setAnimationOptions(QChart::NoAnimation);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->setMargins(QMargins(0, 0, 0, 0));
    m_chart->setBackgroundRoundness(0);

    QFont legFont;
    legFont.setPixelSize(9);
    m_chart->legend()->setFont(legFont);

    m_chartCard->setObjectName(QStringLiteral("historyChartCard"));
    auto *chartLayout = new QVBoxLayout(m_chartCard);
    chartLayout->setContentsMargins(8, 4, 8, 4);
    chartLayout->setSpacing(2);

    m_chartHeaderLayout = new QHBoxLayout;
    m_chartHeaderLayout->setContentsMargins(0, 0, 0, 0);
    m_chartHeaderLayout->setSpacing(4);

    m_chartTitle = new QLabel(tr("Dữ liệu cảm biến"), m_chartCard);
    m_chartTitle->setObjectName(QStringLiteral("historyCardTitle"));
    m_chartHint = new QLabel(m_chartCard);
    m_chartHint->setObjectName(QStringLiteral("historyChartHint"));
    m_chartHint->hide();
    m_chartHeaderLayout->addWidget(m_chartTitle, 1);

    m_metricCombo = new QComboBox(m_chartCard);
    m_metricCombo->setObjectName(QStringLiteral("historyMetricCombo"));
    m_metricCombo->setMinimumWidth(110);
    m_metricCombo->hide();
    m_chartHeaderLayout->addWidget(m_metricCombo, 0, Qt::AlignVCenter | Qt::AlignRight);

    auto *zoomBtn = new QPushButton(tr("Phóng to"), m_chartCard);
    zoomBtn->setObjectName(QStringLiteral("historyZoomButton"));
    zoomBtn->setCursor(Qt::PointingHandCursor);
    zoomBtn->setToolTip(tr("Phóng to biểu đồ"));
    connect(zoomBtn, &QPushButton::clicked, this, [this] {
        openChartZoomDialog(m_selectedMetricKey);
    });
    m_chartHeaderLayout->addWidget(zoomBtn, 0, Qt::AlignVCenter | Qt::AlignRight);

    chartLayout->addLayout(m_chartHeaderLayout);
    chartLayout->addWidget(m_chartView, 1);

    connect(m_metricCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        if (m_metricCombo->currentIndex() >= 0) {
            m_selectedMetricKey = m_metricCombo->currentData().toString();
            updateChart();
        }
    });

    m_analyticsGrid->setContentsMargins(0, 0, 0, 0);
    m_analyticsGrid->setHorizontalSpacing(6);
    m_analyticsGrid->setVerticalSpacing(0);
    m_primaryStatCard = makeStatCard(tr("Chỉ số chính"), m_primaryStat, QStringLiteral("↯"));
    m_secondaryStatCard = makeStatCard(tr("Chỉ số phụ"), m_secondaryStat, QStringLiteral(""));
    m_summaryStatCard = makeStatCard(tr("Tóm tắt"), m_thirdStat, QStringLiteral(""));
    m_analyticsGrid->addWidget(m_primaryStatCard, 0, 0);
    m_analyticsGrid->addWidget(m_secondaryStatCard, 0, 1);
    m_analyticsGrid->addWidget(m_summaryStatCard, 0, 2);

    // Build Chart View Page into ui->chartPage
    auto *chartPageLayout = new QVBoxLayout(ui->chartPage);
    chartPageLayout->setContentsMargins(0, 0, 0, 0);
    chartPageLayout->setSpacing(4);
    chartPageLayout->addWidget(m_chartCard, 1);
    chartPageLayout->addLayout(m_analyticsGrid);

    ui->viewStack->setCurrentIndex(0);
    applyResponsiveLayout();

    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit->setMinimumWidth(105);
    ui->dateEdit->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    ui->periodCombo->setMinimumWidth(80);
    ui->periodCombo->setItemData(0, QStringLiteral("day"));
    ui->periodCombo->setItemData(1, QStringLiteral("month"));
    ui->periodCombo->setItemData(2, QStringLiteral("year"));
    connect(ui->searchButton, &QPushButton::clicked,
            this, &HistoryPage::requestCurrentHistory);
    connect(ui->deviceCombo, &QComboBox::currentIndexChanged,
            this, [this](int) { requestCurrentHistory(); });
    connect(ui->periodCombo, &QComboBox::currentIndexChanged,
            this, [this](int) { requestCurrentHistory(); });
    connect(ui->dateEdit, &QDateEdit::dateChanged,
            this, [this](const QDate &) { requestCurrentHistory(); });

    auto *refreshTimer = new QTimer(this);
    refreshTimer->setInterval(30000);
    connect(refreshTimer, &QTimer::timeout, this, [this]() {
        if (isVisible() && !ui->deviceCombo->currentData().toString().isEmpty()) {
            requestCurrentHistory();
        }
    });
    refreshTimer->start();
}

bool HistoryPage::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
        if (m_chartView && watched == m_chartView->viewport()) {
            openChartZoomDialog(m_selectedMetricKey);
            return true;
        }
        if (watched == m_primaryStatCard) {
            const QStringList canonicalKeys = getCanonicalMetricKeys(m_cachedKeys);
            QStringList plotable;
            for (const QString &k : canonicalKeys)
                if (k != QStringLiteral("ir_detected")) plotable.append(k);
            openChartZoomDialog(plotable.value(0, m_selectedMetricKey));
            return true;
        }
        if (watched == m_secondaryStatCard) {
            const QStringList canonicalKeys = getCanonicalMetricKeys(m_cachedKeys);
            QStringList plotable;
            for (const QString &k : canonicalKeys)
                if (k != QStringLiteral("ir_detected")) plotable.append(k);
            openChartZoomDialog(plotable.value(1, m_selectedMetricKey));
            return true;
        }
        if (watched == m_summaryStatCard) {
            const QStringList canonicalKeys = getCanonicalMetricKeys(m_cachedKeys);
            QStringList plotable;
            for (const QString &k : canonicalKeys)
                if (k != QStringLiteral("ir_detected")) plotable.append(k);
            openChartZoomDialog(plotable.value(2, QStringLiteral("all")));
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

    ui->deviceCombo->setMinimumWidth(compact ? 130 : 200);
    ui->deviceCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->periodCombo->setMinimumWidth(compact ? 70 : 100);
    ui->dateEdit->setMinimumWidth(compact ? 90 : 130);
    ui->searchButton->setMinimumWidth(compact ? 50 : 80);
    m_chartView->setMinimumHeight(compact ? 190 : 260);
    ui->historyTable->setMinimumHeight(compact ? 100 : 160);
    ui->verticalLayout->setContentsMargins(compact ? 6 : 12, compact ? 4 : 8,
                                           compact ? 6 : 12, compact ? 4 : 8);
    ui->verticalLayout->setSpacing(compact ? 4 : 8);
}

HistoryPage::~HistoryPage()
{
    delete ui;
}

void HistoryPage::setDevices(const QJsonArray &devices)
{
    // Avoid re-requesting history every 1.5s poll if the device list has not changed
    bool unchanged = (devices.size() == ui->deviceCombo->count());
    if (unchanged && devices.size() > 0) {
        for (int i = 0; i < devices.size(); ++i) {
            const QJsonObject device = devices.at(i).toObject();
            const QString id = device.value(QStringLiteral("device_id")).toString();
            if (ui->deviceCombo->itemData(i).toString() != id) {
                unchanged = false;
                break;
            }
        }
    } else {
        unchanged = false;
    }

    if (unchanged) {
        return;
    }

    const QString selected = ui->deviceCombo->currentData().toString();
    ui->deviceCombo->blockSignals(true);
    ui->deviceCombo->clear();
    for (const QJsonValue &value : devices) {
        const QJsonObject device = value.toObject();
        const QString id = device.value(QStringLiteral("device_id")).toString();
        const QString name = device.value(QStringLiteral("name")).toString();
        const QString type = device.value(QStringLiteral("device_type")).toString();
        const QString addedBy = device.value(QStringLiteral("added_by")).toString();
        QString itemText = QStringLiteral("%1  ·  %2").arg(name, id);
        if (!addedBy.isEmpty()) {
            itemText += tr(" (Thêm bởi: %1)").arg(addedBy);
        }
        ui->deviceCombo->addItem(itemText, id);
        ui->deviceCombo->setItemData(ui->deviceCombo->count() - 1, type, Qt::UserRole + 1);
    }
    const int previous = ui->deviceCombo->findData(selected);
    if (previous >= 0)
        ui->deviceCombo->setCurrentIndex(previous);
    ui->deviceCombo->blockSignals(false);
    requestCurrentHistory();
}

void HistoryPage::setPeriod(const QString &period)
{
    const int idx = ui->periodCombo->findData(period);
    if (idx >= 0 && idx != ui->periodCombo->currentIndex()) {
        ui->periodCombo->setCurrentIndex(idx);
    }
}

void HistoryPage::showTableView(bool showTable)
{
    if (showTable) {
        ui->tableTabButton->setChecked(true);
        ui->viewStack->setCurrentIndex(1);
    } else {
        ui->chartTabButton->setChecked(true);
        ui->viewStack->setCurrentIndex(0);
    }
}

void HistoryPage::requestCurrentHistory()
{
    const QString deviceId = ui->deviceCombo->currentData().toString();
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
    emit historyRequested(deviceId, ui->periodCombo->currentData().toString(),
                          ui->dateEdit->date().toString(Qt::ISODate));
}

void HistoryPage::updateMetricSelector()
{
    m_metricCombo->blockSignals(true);
    m_metricCombo->clear();

    const QStringList canonicalKeys = getCanonicalMetricKeys(m_cachedKeys);
    if (canonicalKeys.isEmpty()) {
        m_metricCombo->hide();
        m_metricCombo->blockSignals(false);
        return;
    }

    QStringList plotableKeys;
    for (const QString &keyStr : canonicalKeys) {
        if (keyStr != QStringLiteral("ir_detected"))
            plotableKeys.append(keyStr);
    }

    if (plotableKeys.size() > 1) {
        m_metricCombo->addItem(tr("Tất cả chỉ số"), QStringLiteral("all"));
        for (const QString &key : plotableKeys) {
            m_metricCombo->addItem(metricTitle(key), key);
        }
        int idx = m_metricCombo->findData(m_selectedMetricKey);
        if (idx >= 0) {
            m_metricCombo->setCurrentIndex(idx);
        } else {
            m_metricCombo->setCurrentIndex(0);
            m_selectedMetricKey = QStringLiteral("all");
        }
        m_metricCombo->show();
    } else {
        m_metricCombo->hide();
        m_selectedMetricKey = QStringLiteral("all");
    }
    m_metricCombo->blockSignals(false);
}

void HistoryPage::setHistory(const QJsonObject &history)
{
    m_cachedKeys = history.value(QStringLiteral("metric_keys")).toArray();
    m_cachedRows = history.value(QStringLiteral("data")).toArray();
    const QStringList canonicalKeys = getCanonicalMetricKeys(m_cachedKeys);
    const QJsonArray rows = m_cachedRows;

    const QString addedBy = history.value(QStringLiteral("added_by")).toString();
    const QString addedAt = history.value(QStringLiteral("added_at")).toString();
    if (!addedBy.isEmpty() && addedBy != QStringLiteral("Chưa gán")) {
        QDateTime addTime = QDateTime::fromString(addedAt, Qt::ISODateWithMs);
        if (!addTime.isValid()) addTime = QDateTime::fromString(addedAt, Qt::ISODate);
        const QString addTimeStr = addTime.isValid() ? addTime.toLocalTime().toString(QStringLiteral("dd/MM/yyyy HH:mm")) : addedAt;
        m_headerSubtitle->setText(
            tr("Thiết bị: %1 · Người thêm: %2 (%3) · Bấm vào biểu đồ để phóng to.")
                .arg(ui->deviceCombo->currentText(), addedBy, addTimeStr));
    }

    const QString currentPeriod = ui->periodCombo->currentData().toString();
    ui->historyTable->setUpdatesEnabled(false);
    ui->historyTable->clear();
    ui->historyTable->setRowCount(rows.size());

    // Build the ordered list of columns for the table:
    // 1. Độ sáng (Lux)
    // 2. Phát hiện người (PIR)
    // 3. Trạng thái đèn
    QStringList tableKeys;
    tableKeys.append(QStringLiteral("light_lux"));
    tableKeys.append(QStringLiteral("motion_detected"));
    tableKeys.append(QStringLiteral("relay"));
    for (const QString &k : canonicalKeys) {
        if (!tableKeys.contains(k) && k != QStringLiteral("ir_detected")) {
            tableKeys.append(k);
        }
    }

    ui->historyTable->setColumnCount(tableKeys.size() + 1);
    QStringList headers{currentPeriod == QStringLiteral("year") ? tr("Tháng") : (currentPeriod == QStringLiteral("month") ? tr("Ngày") : tr("Thời gian"))};
    for (const QString &key : tableKeys) {
        if (key == QStringLiteral("motion_detected")) {
            headers.append(tr("Phát hiện người (PIR)"));
        } else if (key == QStringLiteral("relay")) {
            headers.append(tr("Trạng thái đèn"));
        } else {
            headers.append(metricTitle(key));
        }
    }
    ui->historyTable->setHorizontalHeaderLabels(headers);
    for (int row = 0; row < rows.size(); ++row) {
        const QJsonObject entry = rows.at(row).toObject();
        QString recordedAtStr = entry.value(QStringLiteral("recorded_at")).toString();
        QDateTime time = QDateTime::fromString(recordedAtStr, Qt::ISODateWithMs);
        if (!time.isValid())
            time = QDateTime::fromString(recordedAtStr, Qt::ISODate);
        if (!time.isValid())
            time = QDateTime::fromString(recordedAtStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        time = time.toLocalTime();

        QString displayTime;
        if (entry.contains(QStringLiteral("bucket_label"))) {
            displayTime = entry.value(QStringLiteral("bucket_label")).toString();
        } else if (currentPeriod == QStringLiteral("year")) {
            displayTime = time.isValid() ? time.toString(QStringLiteral("MM/yyyy")) : recordedAtStr;
        } else if (currentPeriod == QStringLiteral("month")) {
            displayTime = time.isValid() ? time.toString(QStringLiteral("dd/MM/yyyy")) : recordedAtStr;
        } else {
            displayTime = time.isValid() ? time.toString(QStringLiteral("dd/MM/yyyy HH:mm:ss")) : recordedAtStr;
        }

        auto *timeItem = new QTableWidgetItem(displayTime);
        timeItem->setTextAlignment(Qt::AlignCenter);
        timeItem->setForeground(QBrush(QColor("#f8fafc")));
        ui->historyTable->setItem(row, 0, timeItem);

        const QJsonObject metrics = entry.value(QStringLiteral("metrics")).toObject();
        for (int col = 0; col < tableKeys.size(); ++col) {
            const QString key = tableKeys.at(col);
            double val = 0.0;
            QTableWidgetItem *valItem = nullptr;
            if (extractMetricValue(metrics, key, val)) {
                if (key == QStringLiteral("motion_detected")) {
                    if (currentPeriod == QStringLiteral("year") || currentPeriod == QStringLiteral("month")) {
                        const bool hasMotion = (val > 0.01);
                        valItem = new QTableWidgetItem(hasMotion ? tr("Có người (%1%)").arg(QString::number(val * 100, 'f', 0)) : tr("Không có"));
                        valItem->setForeground(QBrush(hasMotion ? QColor("#f59e0b") : QColor("#94a3b8")));
                    } else {
                        const bool detected = (val >= 0.5);
                        valItem = new QTableWidgetItem(detected ? tr("● Có người") : tr("Bình thường"));
                        valItem->setForeground(QBrush(detected ? QColor("#f59e0b") : QColor("#94a3b8")));
                    }
                } else if (key == QStringLiteral("relay")) {
                    if (currentPeriod == QStringLiteral("year") || currentPeriod == QStringLiteral("month")) {
                        const bool hasRelay = (val > 0.01);
                        valItem = new QTableWidgetItem(hasRelay ? tr("Bật (%1%)").arg(QString::number(val * 100, 'f', 0)) : tr("Tắt"));
                        valItem->setForeground(QBrush(hasRelay ? QColor("#10b981") : QColor("#94a3b8")));
                    } else {
                        const bool on = (val >= 0.5);
                        valItem = new QTableWidgetItem(on ? tr("● Đang Bật") : tr("Đang Tắt"));
                        valItem->setForeground(QBrush(on ? QColor("#10b981") : QColor("#94a3b8")));
                    }
                } else {
                    valItem = new QTableWidgetItem(QString::number(val, 'f', 2));
                    valItem->setForeground(QBrush(QColor("#38bdf8")));
                }
            } else {
                valItem = new QTableWidgetItem(QStringLiteral("—"));
                valItem->setForeground(QBrush(QColor("#64748b")));
            }
            valItem->setTextAlignment(Qt::AlignCenter);
            ui->historyTable->setItem(row, col + 1, valItem);
        }
    }
    ui->historyTable->setUpdatesEnabled(true);

    const int total = history.value(QStringLiteral("total")).toInt();
    ui->recordCountLabel->setText(tr("%1 bản ghi").arg(total));

    if (!rows.isEmpty()) {
        double latestLux = 0.0;
        bool hasLux = false;
        double minLux = std::numeric_limits<double>::max();
        double maxLux = std::numeric_limits<double>::lowest();
        double sumLux = 0.0;
        int countLux = 0;

        double latestMotion = 0.0;
        bool hasMotion = false;
        double latestRelay = 0.0;
        bool hasRelay = false;

        for (int r = 0; r < rows.size(); ++r) {
            const QJsonObject metrics = rows.at(r).toObject().value(QStringLiteral("metrics")).toObject();
            double v = 0.0;
            if (extractMetricValue(metrics, QStringLiteral("light_lux"), v)) {
                if (!hasLux) { latestLux = v; hasLux = true; }
                minLux = qMin(minLux, v);
                maxLux = qMax(maxLux, v);
                sumLux += v;
                ++countLux;
            }
            if (!hasMotion && extractMetricValue(metrics, QStringLiteral("motion_detected"), v)) {
                latestMotion = v;
                hasMotion = true;
            }
            if (!hasRelay && extractMetricValue(metrics, QStringLiteral("relay"), v)) {
                latestRelay = v;
                hasRelay = true;
            }
        }

        if (hasLux) {
            m_primaryStat->setText(tr("%1 Lux (Mới nhất)").arg(QString::number(latestLux, 'f', 1)));
        } else {
            m_primaryStat->setText(QStringLiteral("--"));
        }

        if (hasMotion) {
            m_secondaryStat->setText(latestMotion >= 0.5 ? tr("● Có người") : tr("Bình thường"));
        } else if (hasLux && countLux > 0) {
            m_secondaryStat->setText(tr("Min: %1 · Max: %2 Lux").arg(QString::number(minLux, 'f', 1), QString::number(maxLux, 'f', 1)));
        } else {
            m_secondaryStat->setText(QStringLiteral("--"));
        }

        if (hasRelay) {
            m_thirdStat->setText(latestRelay >= 0.5 ? tr("● Đèn Bật (%1 bg)").arg(total) : tr("Đèn Tắt (%1 bg)").arg(total));
        } else if (hasLux && countLux > 0) {
            m_thirdStat->setText(tr("TB: %1 Lux (%2 bg)").arg(QString::number(sumLux / countLux, 'f', 1), QString::number(total)));
        } else {
            m_thirdStat->setText(tr("%1 bản ghi").arg(total));
        }
    } else {
        m_primaryStat->setText(QStringLiteral("--"));
        m_secondaryStat->setText(QStringLiteral("--"));
        m_thirdStat->setText(tr("0 bản ghi"));
    }

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
    const QStringList canonicalKeys = getCanonicalMetricKeys(m_cachedKeys);
    const QJsonArray &rows = m_cachedRows;

    if (canonicalKeys.isEmpty() || rows.isEmpty()) {
        m_chartTitle->setText(tr("Không có dữ liệu"));
        m_chartHint->setText(tr("Không có dữ liệu trong khoảng thời gian đã chọn."));
        return;
    }

    const bool isIrOnly = canonicalKeys.size() == 1 && canonicalKeys.at(0) == QStringLiteral("ir_detected");
    if (isIrOnly) {
        auto *set = new QBarSet(tr("Có vật"));
        set->setColor(QColor("#21a67a"));
        QStringList categories;
        int used = 0;
        for (int row = rows.size() - 1; row >= 0 && used < 12; --row, ++used) {
            const QJsonObject entry = rows.at(row).toObject();
            QString recordedAtStr = entry.value(QStringLiteral("recorded_at")).toString();
            QDateTime time = QDateTime::fromString(recordedAtStr, Qt::ISODateWithMs);
            if (!time.isValid())
                time = QDateTime::fromString(recordedAtStr, Qt::ISODate);
            time = time.toLocalTime();
            categories << (time.isValid() ? time.toString(QStringLiteral("HH:mm:ss")) : QStringLiteral("--"));
            double irVal = 0.0;
            extractMetricValue(entry.value(QStringLiteral("metrics")).toObject(), QStringLiteral("ir_detected"), irVal);
            *set << (int)irVal;
        }
        auto *series = new QBarSeries(m_chart);
        series->append(set);
        m_chart->addSeries(series);
        auto *axisX = new QBarCategoryAxis(m_chart);
        axisX->append(categories);
        auto *axisY = new QValueAxis(m_chart);
        axisY->setRange(0, 1);
        axisY->setTickCount(2);
        axisY->setLabelFormat("%d");
        m_chart->addAxis(axisX, Qt::AlignBottom);
        m_chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
        m_chartTitle->setText(tr("Biểu đồ trạng thái IR · %1").arg(ui->deviceCombo->currentText()));
        m_chartHint->setText(tr("IR dùng biểu đồ cột để thể hiện trạng thái phát hiện vật theo thời gian."));
        return;
    }

    auto *axisX = new QDateTimeAxis(m_chart);
    QFont axisFont;
    axisFont.setPixelSize(9);
    axisX->setLabelsFont(axisFont);
    axisX->setLabelsColor(QColor("#94a3b8"));
    QPen gridPen(QColor("#16233f"));
    gridPen.setStyle(Qt::DashLine);
    axisX->setGridLinePen(gridPen);
    QPen linePen(QColor("#334155"));
    axisX->setLinePen(linePen);
    m_chart->addAxis(axisX, Qt::AlignBottom);

    qint64 minimumTime = std::numeric_limits<qint64>::max();
    qint64 maximumTime = std::numeric_limits<qint64>::min();
    const QList<QColor> colors{QColor("#f59e0b"), QColor("#38bdf8"),
                               QColor("#10b981"), QColor("#a855f7"),
                               QColor("#f43f5e"), QColor("#64748b")};

    QStringList activeKeys;
    if (m_selectedMetricKey.isEmpty() || m_selectedMetricKey == QStringLiteral("all")) {
        for (const QString &k : canonicalKeys) {
            if (k != QStringLiteral("ir_detected"))
                activeKeys.append(k);
        }
    } else {
        const QString canon = canonicalMetricKey(m_selectedMetricKey);
        if (canonicalKeys.contains(canon))
            activeKeys.append(canon);
        else if (!canonicalKeys.isEmpty())
            activeKeys.append(canonicalKeys.first());
    }

    int visibleSeries = 0;
    for (int metricIndex = 0; metricIndex < activeKeys.size(); ++metricIndex) {
        const QString key = activeKeys.at(metricIndex);
        auto *lineSeries = new QLineSeries;
        lineSeries->setName(metricTitle(key));

        double minimum = std::numeric_limits<double>::max();
        double maximum = std::numeric_limits<double>::lowest();

        for (int row = rows.size() - 1; row >= 0; --row) {
            const QJsonObject entry = rows.at(row).toObject();
            const QJsonObject metrics = entry.value(QStringLiteral("metrics")).toObject();
            double number = 0.0;
            if (!extractMetricValue(metrics, key, number))
                continue;

            QString recordedAtStr = entry.value(QStringLiteral("recorded_at")).toString();
            QDateTime time = QDateTime::fromString(recordedAtStr, Qt::ISODateWithMs);
            if (!time.isValid())
                time = QDateTime::fromString(recordedAtStr, Qt::ISODate);
            if (!time.isValid())
                time = QDateTime::fromString(recordedAtStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            time = time.toLocalTime();
            if (!time.isValid())
                continue;

            const qint64 timestamp = time.toMSecsSinceEpoch();
            lineSeries->append(timestamp, number);
            minimumTime = qMin(minimumTime, timestamp);
            maximumTime = qMax(maximumTime, timestamp);
            minimum = qMin(minimum, number);
            maximum = qMax(maximum, number);
        }

        if (lineSeries->count() == 0) {
            delete lineSeries;
            continue;
        }

        // Show dots if point count is small, especially for month/year charts
        const QString periodVal = ui->periodCombo->currentData().toString();
        if (periodVal == QStringLiteral("year") || periodVal == QStringLiteral("month") || lineSeries->count() <= 25) {
            lineSeries->setPointsVisible(true);
            lineSeries->setMarkerSize(periodVal == QStringLiteral("year") ? 5.5 : 3.5);
        } else {
            lineSeries->setPointsVisible(false);
        }

        QColor baseColor = colors.at(metricIndex % colors.size());
        if (key == QStringLiteral("light_lux")) {
            baseColor = QColor("#f59e0b");
        } else if (key == QStringLiteral("motion_detected")) {
            baseColor = QColor("#10b981");
        } else if (key == QStringLiteral("temperature_c")) {
            baseColor = QColor("#f43f5e");
        } else if (key == QStringLiteral("humidity_percent")) {
            baseColor = QColor("#06b6d4");
        }

        QAbstractSeries *seriesToAdd = nullptr;
        if (activeKeys.size() == 1) {
            // Luminous gradient area chart for single metric
            auto *areaSeries = new QAreaSeries(lineSeries);
            lineSeries->setParent(areaSeries);
            areaSeries->setName(metricTitle(key));

            QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
            gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
            gradient.setColorAt(0.0, QColor(baseColor.red(), baseColor.green(), baseColor.blue(), 150));
            gradient.setColorAt(0.8, QColor(baseColor.red(), baseColor.green(), baseColor.blue(), 25));
            gradient.setColorAt(1.0, QColor(baseColor.red(), baseColor.green(), baseColor.blue(), 0));
            areaSeries->setBrush(gradient);

            QPen pen(baseColor);
            pen.setWidthF(2.2);
            areaSeries->setPen(pen);

            seriesToAdd = areaSeries;
        } else {
            // Multi-metric line chart with clean strokes
            QPen pen(baseColor);
            pen.setWidthF(2.2);
            lineSeries->setPen(pen);
            seriesToAdd = lineSeries;
        }

        m_chart->addSeries(seriesToAdd);
        seriesToAdd->attachAxis(axisX);

        auto *axisY = new QValueAxis(m_chart);
        axisY->setTitleText(compactMetricTitle(key));
        axisY->setLabelsColor(QColor("#94a3b8"));
        axisY->setTitleBrush(baseColor);
        axisY->setLabelsFont(axisFont);
        axisY->setTitleFont(axisFont);
        axisY->setGridLinePen(gridPen);
        axisY->setLinePen(linePen);
        axisY->setTickCount(5);

        if (minimum > maximum) {
            minimum = 0;
            maximum = 10;
        }
        const double diff = maximum - minimum;
        const double padding = qMax(0.5, (diff == 0.0 ? (qAbs(maximum) > 0 ? qAbs(maximum) * 0.15 + 0.5 : 1.0) : diff * 0.15));
        double yMin = (minimum >= 0.0) ? qMax(0.0, minimum - padding) : (minimum - padding);
        double yMax = maximum + padding;
        if (yMax <= yMin) yMax = yMin + 10.0;
        axisY->setRange(yMin, yMax);
        axisY->setLabelFormat("%.1f");

        m_chart->addAxis(axisY, visibleSeries == 0 ? Qt::AlignLeft : Qt::AlignRight);
        seriesToAdd->attachAxis(axisY);
        ++visibleSeries;
    }

    if (visibleSeries == 0) {
        m_chartTitle->setText(tr("Không có dữ liệu biểu đồ"));
        m_chartHint->setText(tr("Các bản ghi không chứa giá trị số phù hợp để vẽ biểu đồ."));
        return;
    }

    const QString currentPeriod = ui->periodCombo->currentData().toString();
    const QDate selectedDate = ui->dateEdit->date();

    if (currentPeriod == QStringLiteral("year")) {
        const int yr = selectedDate.year();
        const QDateTime yearStart(QDate(yr, 1, 1), QTime(0, 0, 0));
        const QDateTime yearEnd(QDate(yr, 12, 31), QTime(23, 59, 59));
        axisX->setRange(yearStart, yearEnd);
        axisX->setFormat(QStringLiteral("MM/yyyy"));
        axisX->setTickCount(12);
    } else if (currentPeriod == QStringLiteral("month")) {
        const int yr = selectedDate.year();
        const int mo = selectedDate.month();
        const int daysInMonth = selectedDate.daysInMonth();
        const QDateTime monthStart(QDate(yr, mo, 1), QTime(0, 0, 0));
        const QDateTime monthEnd(QDate(yr, mo, daysInMonth), QTime(23, 59, 59));
        axisX->setRange(monthStart, monthEnd);
        axisX->setFormat(QStringLiteral("dd/MM"));
        axisX->setTickCount(qMin(7, daysInMonth));
    } else if (minimumTime <= maximumTime) {
        const qint64 timeSpan = maximumTime - minimumTime;
        if (timeSpan < 60 * 1000) { // under 1 min or single point
            minimumTime -= 30 * 1000;
            maximumTime += 30 * 1000;
            axisX->setFormat(QStringLiteral("HH:mm:ss"));
            axisX->setTickCount(qMin(5, (int)rows.size() + 2));
        } else if (timeSpan < 3600 * 1000) { // under 1 hour
            axisX->setFormat(QStringLiteral("HH:mm:ss"));
            axisX->setTickCount(5);
        } else {
            axisX->setFormat(QStringLiteral("HH:mm"));
            axisX->setTickCount(6);
        }
        axisX->setRange(QDateTime::fromMSecsSinceEpoch(minimumTime),
                        QDateTime::fromMSecsSinceEpoch(maximumTime));
    }

    m_chart->legend()->setVisible(activeKeys.size() > 1);

    if (activeKeys.size() == 1) {
        const QString k = activeKeys.first();
        m_chartTitle->setText(tr("Biểu đồ %1 · %2").arg(metricTitle(k), ui->deviceCombo->currentText()));
        m_chartHint->setText(tr("Đang hiển thị chỉ số %1 · Thiết bị: %2").arg(metricTitle(k), ui->deviceCombo->currentText()));
    } else {
        m_chartTitle->setText(tr("Thống kê · %1").arg(ui->deviceCombo->currentText()));
        m_chartHint->setText(tr("Đang hiển thị tất cả chỉ số. Bạn có thể chọn từng chỉ số ở menu trên góc phải."));
    }
}

QString HistoryPage::currentDeviceType() const
{
    return ui->deviceCombo->currentData(Qt::UserRole + 1).toString();
}

QString HistoryPage::metricTitle(const QString &key)
{
    static const QHash<QString, QString> names{
        {"light_lux", tr("Độ sáng (Lux)")}, {"lux", tr("Độ sáng (Lux)")},
        {"motion_detected", tr("Chuyển động (PIR)")}, {"motion", tr("Chuyển động (PIR)")},
        {"relay", tr("Đèn phòng")}, {"relay_on", tr("Đèn phòng")},
        {"temperature_c", tr("Nhiệt độ (°C)")}, {"humidity_percent", tr("Độ ẩm (%)")},
        {"pressure_hpa", tr("Áp suất (hPa)")}, {"uv_index", tr("UV Index")},
        {"uv_voltage", tr("Điện áp UV (V)")}, {"sound_vpp", tr("Âm thanh (Vpp)")},
        {"current_a", tr("Dòng điện (A)")}, {"voltage_v", tr("Điện áp (V)")},
        {"distance_cm", tr("Khoảng cách (cm)")},
        {"flow_l_min", tr("Lưu lượng (L/m)")}, {"total_liters", tr("Tổng nước (L)")},
        {"ir_detected", tr("IR")}};
    return names.value(key, key);
}

QString HistoryPage::compactMetricTitle(const QString &key)
{
    static const QHash<QString, QString> names{
        {"light_lux", tr("Lux")}, {"lux", tr("Lux")},
        {"motion_detected", tr("PIR")}, {"motion", tr("PIR")},
        {"relay", tr("Đèn")}, {"relay_on", tr("Đèn")},
        {"temperature_c", tr("°C")}, {"humidity_percent", tr("%")},
        {"pressure_hpa", tr("hPa")}, {"uv_index", tr("UV")},
        {"uv_voltage", tr("V")}, {"sound_vpp", tr("Vpp")},
        {"current_a", tr("A")}, {"voltage_v", tr("V")},
        {"distance_cm", tr("cm")},
        {"flow_l_min", tr("L/m")}, {"total_liters", tr("L")},
        {"ir_detected", tr("IR")}};
    return names.value(key, key);
}

QString HistoryPage::metricUnit(const QString &key)
{
    return compactMetricTitle(key);
}

void HistoryPage::openChartZoomDialog(const QString &initialMetricKey)
{
    if (m_cachedKeys.isEmpty() || m_cachedRows.isEmpty()) {
        return;
    }

    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("chartZoomDialog"));
    dialog.setWindowTitle(tr("Phóng to biểu đồ cảm biến"));
    dialog.setModal(true);

    const int availableWidth = parentWidget() ? parentWidget()->width() - 16 : 760;
    const int availableHeight = parentWidget() ? parentWidget()->height() - 16 : 460;
    dialog.resize(qMax(360, availableWidth), qMax(280, availableHeight));

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(10);

    auto *headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(10);

    auto *titleBlock = new QVBoxLayout;
    titleBlock->setContentsMargins(0, 0, 0, 0);
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

    const QStringList canonicalKeys = getCanonicalMetricKeys(m_cachedKeys);
    QStringList plotableKeys;
    for (const QString &keyStr : canonicalKeys) {
        if (keyStr != QStringLiteral("ir_detected"))
            plotableKeys.append(keyStr);
    }

    if (plotableKeys.size() > 1) {
        metricCombo->addItem(tr("Tất cả chỉ số"), QStringLiteral("all"));
        for (const QString &key : plotableKeys) {
            metricCombo->addItem(metricTitle(key), key);
        }
    } else if (plotableKeys.size() == 1) {
        metricCombo->addItem(metricTitle(plotableKeys.first()), plotableKeys.first());
    }

    QString selectedKey = initialMetricKey.isEmpty() ? m_selectedMetricKey : initialMetricKey;
    selectedKey = canonicalMetricKey(selectedKey);
    if (selectedKey.isEmpty() || metricCombo->findData(selectedKey) < 0)
        selectedKey = plotableKeys.isEmpty() ? QStringLiteral("all") : plotableKeys.first();

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
    zoomChart->setBackgroundBrush(QColor("#0d1527"));
    zoomChart->setPlotAreaBackgroundBrush(QColor("#080e1e"));
    zoomChart->setPlotAreaBackgroundVisible(true);
    zoomChart->setAnimationOptions(QChart::NoAnimation);
    zoomChart->legend()->setAlignment(Qt::AlignBottom);
    zoomChart->legend()->setLabelColor(QColor("#94a3b8"));

    auto *zoomChartView = new QChartView(zoomChart, &dialog);
    zoomChartView->setObjectName(QStringLiteral("chartZoomView"));
    zoomChartView->setRenderHint(QPainter::Antialiasing);
    zoomChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    root->addWidget(zoomChartView, 1);

    auto *statsLayout = new QHBoxLayout;
    statsLayout->setContentsMargins(0, 0, 0, 0);
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
    bottomClose->setMinimumWidth(90);
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
        const QStringList canonicalKeys = getCanonicalMetricKeys(m_cachedKeys);
        const QJsonArray &rows = m_cachedRows;

        QStringList activeKeys;
        if (activeMetric.isEmpty() || activeMetric == QStringLiteral("all")) {
            for (const QString &k : canonicalKeys) {
                if (k != QStringLiteral("ir_detected"))
                    activeKeys.append(k);
            }
            dialogTitle->setText(tr("Biểu đồ tổng quan các chỉ số"));
        } else {
            const QString canon = canonicalMetricKey(activeMetric);
            if (canonicalKeys.contains(canon))
                activeKeys.append(canon);
            else if (!canonicalKeys.isEmpty())
                activeKeys.append(canonicalKeys.first());
            dialogTitle->setText(tr("Biểu đồ chi tiết: %1").arg(metricTitle(activeKeys.first())));
        }

        auto *axisX = new QDateTimeAxis(zoomChart);
        axisX->setLabelsColor(QColor("#94a3b8"));
        QPen gridPen(QColor("#16233f"));
        gridPen.setStyle(Qt::DashLine);
        axisX->setGridLinePen(gridPen);
        QPen linePen(QColor("#334155"));
        axisX->setLinePen(linePen);
        zoomChart->addAxis(axisX, Qt::AlignBottom);

        qint64 minimumTime = std::numeric_limits<qint64>::max();
        qint64 maximumTime = std::numeric_limits<qint64>::min();
        const QList<QColor> colors{QColor("#f59e0b"), QColor("#38bdf8"),
                                   QColor("#10b981"), QColor("#a855f7"),
                                   QColor("#f43f5e"), QColor("#64748b")};

        double overallMin = std::numeric_limits<double>::max();
        double overallMax = std::numeric_limits<double>::lowest();
        double overallSum = 0;
        int overallCount = 0;

        int visibleSeries = 0;
        for (int i = 0; i < activeKeys.size(); ++i) {
            const QString key = activeKeys.at(i);
            auto *lineSeries = new QLineSeries;
            lineSeries->setName(metricTitle(key));

            double minimum = std::numeric_limits<double>::max();
            double maximum = std::numeric_limits<double>::lowest();

            for (int r = rows.size() - 1; r >= 0; --r) {
                const QJsonObject entry = rows.at(r).toObject();
                const QJsonObject metrics = entry.value(QStringLiteral("metrics")).toObject();
                double num = 0.0;
                if (!extractMetricValue(metrics, key, num))
                    continue;

                QString recordedAtStr = entry.value(QStringLiteral("recorded_at")).toString();
                QDateTime time = QDateTime::fromString(recordedAtStr, Qt::ISODateWithMs);
                if (!time.isValid())
                    time = QDateTime::fromString(recordedAtStr, Qt::ISODate);
                if (!time.isValid())
                    time = QDateTime::fromString(recordedAtStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
                time = time.toLocalTime();
                if (!time.isValid())
                    continue;

                const qint64 ts = time.toMSecsSinceEpoch();
                lineSeries->append(ts, num);
                minimumTime = qMin(minimumTime, ts);
                maximumTime = qMax(maximumTime, ts);
                minimum = qMin(minimum, num);
                maximum = qMax(maximum, num);
                overallMin = qMin(overallMin, num);
                overallMax = qMax(overallMax, num);
                overallSum += num;
                ++overallCount;
            }

            if (lineSeries->count() == 0) {
                delete lineSeries;
                continue;
            }

            const QString periodVal = ui->periodCombo->currentData().toString();
            if (periodVal == QStringLiteral("year") || periodVal == QStringLiteral("month") || lineSeries->count() <= 30) {
                lineSeries->setPointsVisible(true);
                lineSeries->setMarkerSize(periodVal == QStringLiteral("year") ? 5.5 : 4.0);
            } else {
                lineSeries->setPointsVisible(false);
            }

            QColor baseColor = colors.at(i % colors.size());
            if (key == QStringLiteral("light_lux")) {
                baseColor = QColor("#f59e0b");
            } else if (key == QStringLiteral("motion_detected")) {
                baseColor = QColor("#10b981");
            } else if (key == QStringLiteral("temperature_c")) {
                baseColor = QColor("#f43f5e");
            } else if (key == QStringLiteral("humidity_percent")) {
                baseColor = QColor("#06b6d4");
            }

            QAbstractSeries *seriesToAdd = nullptr;
            if (activeKeys.size() == 1) {
                auto *areaSeries = new QAreaSeries(lineSeries);
                lineSeries->setParent(areaSeries);
                areaSeries->setName(metricTitle(key));

                QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
                gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
                gradient.setColorAt(0.0, QColor(baseColor.red(), baseColor.green(), baseColor.blue(), 150));
                gradient.setColorAt(0.8, QColor(baseColor.red(), baseColor.green(), baseColor.blue(), 25));
                gradient.setColorAt(1.0, QColor(baseColor.red(), baseColor.green(), baseColor.blue(), 0));
                areaSeries->setBrush(gradient);

                QPen pen(baseColor);
                pen.setWidthF(2.4);
                areaSeries->setPen(pen);

                seriesToAdd = areaSeries;
            } else {
                QPen pen(baseColor);
                pen.setWidthF(2.4);
                lineSeries->setPen(pen);
                seriesToAdd = lineSeries;
            }

            zoomChart->addSeries(seriesToAdd);
            seriesToAdd->attachAxis(axisX);

            auto *axisY = new QValueAxis(zoomChart);
            axisY->setTitleText(compactMetricTitle(key));
            axisY->setLabelsColor(QColor("#94a3b8"));
            axisY->setTitleBrush(baseColor);
            axisY->setGridLinePen(gridPen);
            axisY->setLinePen(linePen);
            axisY->setTickCount(5);

            if (minimum > maximum) { minimum = 0; maximum = 10; }
            const double diff = maximum - minimum;
            const double padding = qMax(0.5, (diff == 0.0 ? (qAbs(maximum) > 0 ? qAbs(maximum) * 0.15 + 0.5 : 1.0) : diff * 0.15));
            double yMin = (minimum >= 0.0) ? qMax(0.0, minimum - padding) : (minimum - padding);
            double yMax = maximum + padding;
            if (yMax <= yMin) yMax = yMin + 10.0;
            axisY->setRange(yMin, yMax);
            axisY->setLabelFormat("%.2f");

            zoomChart->addAxis(axisY, visibleSeries == 0 ? Qt::AlignLeft : Qt::AlignRight);
            seriesToAdd->attachAxis(axisY);
            ++visibleSeries;
        }

        const QString currentPeriod = ui->periodCombo->currentData().toString();
        const QDate selectedDate = ui->dateEdit->date();

        if (currentPeriod == QStringLiteral("year")) {
            const int yr = selectedDate.year();
            const QDateTime yearStart(QDate(yr, 1, 1), QTime(0, 0, 0));
            const QDateTime yearEnd(QDate(yr, 12, 31), QTime(23, 59, 59));
            axisX->setRange(yearStart, yearEnd);
            axisX->setFormat(QStringLiteral("MM/yyyy"));
            axisX->setTickCount(12);
        } else if (currentPeriod == QStringLiteral("month")) {
            const int yr = selectedDate.year();
            const int mo = selectedDate.month();
            const int daysInMonth = selectedDate.daysInMonth();
            const QDateTime monthStart(QDate(yr, mo, 1), QTime(0, 0, 0));
            const QDateTime monthEnd(QDate(yr, mo, daysInMonth), QTime(23, 59, 59));
            axisX->setRange(monthStart, monthEnd);
            axisX->setFormat(QStringLiteral("dd/MM"));
            axisX->setTickCount(qMin(7, daysInMonth));
        } else if (minimumTime <= maximumTime) {
            const qint64 timeSpan = maximumTime - minimumTime;
            if (timeSpan < 60 * 1000) {
                minimumTime -= 30 * 1000;
                maximumTime += 30 * 1000;
                axisX->setFormat(QStringLiteral("HH:mm:ss"));
                axisX->setTickCount(qMin(5, (int)rows.size() + 2));
            } else if (timeSpan < 3600 * 1000) {
                axisX->setFormat(QStringLiteral("HH:mm:ss"));
                axisX->setTickCount(6);
            } else {
                axisX->setFormat(QStringLiteral("HH:mm"));
                axisX->setTickCount(6);
            }
            axisX->setRange(QDateTime::fromMSecsSinceEpoch(minimumTime),
                            QDateTime::fromMSecsSinceEpoch(maximumTime));
        }

        zoomChart->legend()->setVisible(activeKeys.size() > 1);

        if (overallCount > 0 && activeKeys.size() == 1) {
            const QString unit = compactMetricTitle(activeKeys.first());
            minBadge->setText(tr("Min: %1 %2").arg(QString::number(overallMin, 'f', 2), unit));
            maxBadge->setText(tr("Max: %1 %2").arg(QString::number(overallMax, 'f', 2), unit));
            avgBadge->setText(tr("TB: %1 %2").arg(QString::number(overallSum / overallCount, 'f', 2), unit));
            minBadge->show();
            maxBadge->show();
            avgBadge->show();
        } else {
            minBadge->hide();
            maxBadge->hide();
            avgBadge->hide();
        }
    };

    connect(metricCombo, &QComboBox::currentIndexChanged, &dialog, [renderZoomChart](int) {
        renderZoomChart();
    });

    renderZoomChart();
    dialog.exec();

    const QString finalKey = metricCombo->currentData().toString();
    int idx = m_metricCombo->findData(finalKey);
    if (idx >= 0 && idx != m_metricCombo->currentIndex()) {
        m_metricCombo->setCurrentIndex(idx);
    }
}

