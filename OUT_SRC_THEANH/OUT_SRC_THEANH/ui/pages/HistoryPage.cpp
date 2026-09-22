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

#include <limits>

namespace {
static QColor getColorForKey(const QString &k, int fallbackIdx = 0) {
    if (k == QStringLiteral("current_a"))
        return QColor("#10b981"); // Emerald green - Dòng điện
    if (k == QStringLiteral("power_w"))
        return QColor("#38bdf8"); // Sky blue - Công suất
    if (k == QStringLiteral("voltage_v"))
        return QColor("#f59e0b"); // Amber - Điện áp
    static const QList<QColor> fallback{QColor("#10b981"), QColor("#38bdf8"),
                                        QColor("#f59e0b"), QColor("#a855f7")};
    return fallback.at(fallbackIdx % fallback.size());
}

static QString metricShortName(const QString &key) {
    static const QHash<QString, QString> names{
        {QStringLiteral("current_a"), QObject::tr("Dòng điện")},
        {QStringLiteral("voltage_v"), QObject::tr("Điện áp")},
        {QStringLiteral("power_w"), QObject::tr("Công suất")},
        {QStringLiteral("frequency_hz"), QObject::tr("Tần số")},
        {QStringLiteral("power_factor"), QObject::tr("cosφ")},
        {QStringLiteral("temperature_c"), QObject::tr("Nhiệt độ")},
        {QStringLiteral("humidity_percent"), QObject::tr("Độ ẩm")},
        {QStringLiteral("pressure_hpa"), QObject::tr("Áp suất")}
    };
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
      m_chartHint(new QLabel(this)),
      m_headerSubtitle(new QLabel(this)),
      m_analyticsGrid(new QGridLayout),
      m_chartCard(new QFrame(this)),
      m_primaryStatCard(nullptr),
      m_secondaryStatCard(nullptr),
      m_summaryStatCard(nullptr)
{
    ui->setupUi(this);
    auto *backBtn = new QPushButton(tr("← Giám sát"), this);
    backBtn->setObjectName(QStringLiteral("historyBackButton"));
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setToolTip(tr("Quay lại màn hình giám sát SCADA"));
    connect(backBtn, &QPushButton::clicked, this, &HistoryPage::backToDashboardRequested);
    ui->filterLayout->insertWidget(0, backBtn);

    ui->recordCountLabel->setObjectName(QStringLiteral("historyRecordBadge"));
    ui->chartTabButton->setObjectName(QStringLiteral("deviceViewTabButton"));
    ui->tableTabButton->setObjectName(QStringLiteral("deviceViewTabButton"));
    ui->deviceCombo->setObjectName(QStringLiteral("historyDeviceCombo"));
    ui->deviceCombo->setMinimumWidth(130);
    ui->deviceCombo->setMaximumWidth(180);
    ui->periodCombo->setObjectName(QStringLiteral("historyPeriodCombo"));
    ui->dateEdit->setObjectName(QStringLiteral("historyDateEdit"));
    ui->dateEdit->setMinimumWidth(115);
    ui->dateEdit->setMaximumWidth(125);
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
    ui->historyTable->verticalHeader()->setDefaultSectionSize(26);
    ui->historyTable->setAlternatingRowColors(false);
    ui->historyTable->setObjectName(QStringLiteral("historyTableSmart"));

    QPalette pal = ui->historyTable->palette();
    pal.setColor(QPalette::Base, QColor("#070d1e"));
    pal.setColor(QPalette::AlternateBase, QColor("#0f1c3f"));
    pal.setColor(QPalette::Text, QColor("#f8fafc"));
    pal.setColor(QPalette::WindowText, QColor("#f8fafc"));
    pal.setColor(QPalette::Highlight, QColor("#0284c7"));
    pal.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    ui->historyTable->setPalette(pal);

    ui->historyTable->setStyleSheet(QStringLiteral(
        "QTableWidget { "
        "  background-color: #070d1e; "
        "  alternate-background-color: #0f1c3f; "
        "  color: #f8fafc; "
        "  gridline-color: #1c2b54; "
        "  border: 1px solid #1c2b54; "
        "  font-size: 11px; "
        "} "
        "QHeaderView::section { "
        "  background-color: #0d1733; "
        "  color: #38bdf8; "
        "  font-weight: 800; "
        "  font-size: 11px; "
        "  padding: 6px 4px; "
        "  border: none; "
        "  border-right: 1px solid #1c2b54; "
        "  border-bottom: 2px solid #0284c7; "
        "} "
        "QTableWidget::item { "
        "  color: #f8fafc; "
        "  padding: 4px; "
        "  border-bottom: 1px solid #142040; "
        "} "
        "QTableWidget::item:selected { "
        "  background-color: #0284c7; "
        "  color: #ffffff; "
        "} "
    ));

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
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
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

    auto *zoomBtn = new QPushButton(tr("⛶ Phóng to"), m_chartCard);
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
    m_secondaryStatCard = makeStatCard(tr("Chỉ số phụ"), m_secondaryStat, QStringLiteral("◍"));
    m_summaryStatCard = makeStatCard(tr("Tóm tắt"), m_thirdStat, QStringLiteral("▥"));
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
            this, [this](int) {
                const QString p = ui->periodCombo->currentData().toString();
                if (p == QStringLiteral("year")) {
                    ui->dateEdit->setDisplayFormat(QStringLiteral("yyyy"));
                } else if (p == QStringLiteral("month")) {
                    ui->dateEdit->setDisplayFormat(QStringLiteral("MM/yyyy"));
                } else {
                    ui->dateEdit->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
                }
                requestCurrentHistory();
            });
    connect(ui->dateEdit, &QDateEdit::dateChanged,
            this, [this](const QDate &) { requestCurrentHistory(); });
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

bool HistoryPage::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
        if (m_chartView && watched == m_chartView->viewport()) {
            openChartZoomDialog(m_selectedMetricKey);
            return true;
        }
        if (watched == m_primaryStatCard) {
            QStringList plotable;
            for (const QJsonValue &k : m_cachedKeys)
                if (k.toString() != QStringLiteral("ir_detected")) plotable.append(k.toString());
            if (!plotable.isEmpty())
                setMetric(plotable.value(0, m_selectedMetricKey));
            return true;
        }
        if (watched == m_secondaryStatCard) {
            QStringList plotable;
            for (const QJsonValue &k : m_cachedKeys)
                if (k.toString() != QStringLiteral("ir_detected")) plotable.append(k.toString());
            if (plotable.size() > 1)
                setMetric(plotable.value(1, m_selectedMetricKey));
            return true;
        }
        if (watched == m_summaryStatCard) {
            QStringList plotable;
            for (const QJsonValue &k : m_cachedKeys)
                if (k.toString() != QStringLiteral("ir_detected")) plotable.append(k.toString());
            if (plotable.size() > 2)
                setMetric(plotable.value(2, m_selectedMetricKey));
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

    if (m_cachedKeys.isEmpty()) {
        m_metricCombo->hide();
        m_metricCombo->blockSignals(false);
        return;
    }

    QStringList plotableKeys;
    for (const QJsonValue &k : m_cachedKeys) {
        const QString keyStr = k.toString();
        if (keyStr != QStringLiteral("ir_detected"))
            plotableKeys.append(keyStr);
    }

    if (!plotableKeys.isEmpty()) {
        for (const QString &key : plotableKeys) {
            m_metricCombo->addItem(tr("📊 %1").arg(metricTitle(key)), key);
        }
        int idx = m_metricCombo->findData(m_selectedMetricKey);
        if (idx >= 0) {
            m_metricCombo->setCurrentIndex(idx);
        } else {
            int defaultIdx = m_metricCombo->findData(QStringLiteral("current_a"));
            if (defaultIdx < 0) defaultIdx = m_metricCombo->findData(QStringLiteral("voltage_v"));
            if (defaultIdx < 0) defaultIdx = 0;
            m_metricCombo->setCurrentIndex(defaultIdx);
            m_selectedMetricKey = m_metricCombo->itemData(defaultIdx).toString();
        }
        m_metricCombo->show();
    } else {
        m_metricCombo->hide();
    }
    m_metricCombo->blockSignals(false);
}

void HistoryPage::setHistory(const QJsonObject &history)
{
    m_cachedKeys = history.value(QStringLiteral("metric_keys")).toArray();
    m_cachedRows = history.value(QStringLiteral("data")).toArray();
    m_cachedPeriod = history.value(QStringLiteral("period")).toString();
    m_cachedSelectedDate = history.value(QStringLiteral("selected_date")).toString();
    const QJsonArray keys = m_cachedKeys;
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

    ui->historyTable->clear();
    ui->historyTable->setRowCount(rows.size());
    ui->historyTable->setColumnCount(keys.size() + 1);
    QStringList headers{tr("Thời gian / Mốc")};
    for (const QJsonValue &key : keys)
        headers.append(metricTitle(key.toString()));
    ui->historyTable->setHorizontalHeaderLabels(headers);
    for (int row = 0; row < rows.size(); ++row) {
        const QColor rowBg = (row % 2 == 0) ? QColor("#070d1e") : QColor("#0f1c3f");
        const QColor textColor = QColor("#f8fafc");

        const QJsonObject entry = rows.at(row).toObject();
        QString timeDisplay;
        if (entry.contains(QStringLiteral("label"))) {
            timeDisplay = entry.value(QStringLiteral("label")).toString();
        } else {
            QString recordedAtStr = entry.value(QStringLiteral("recorded_at")).toString();
            QDateTime time = QDateTime::fromString(recordedAtStr, Qt::ISODateWithMs);
            if (!time.isValid())
                time = QDateTime::fromString(recordedAtStr, Qt::ISODate);
            if (!time.isValid())
                time = QDateTime::fromString(recordedAtStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            time = time.toLocalTime();
            timeDisplay = time.isValid() ? time.toString(QStringLiteral("dd/MM/yyyy HH:mm:ss")) : recordedAtStr;
        }
        auto *timeItem = new QTableWidgetItem(timeDisplay);
        timeItem->setTextAlignment(Qt::AlignCenter);
        timeItem->setBackground(rowBg);
        timeItem->setForeground(textColor);
        ui->historyTable->setItem(row, 0, timeItem);

        const QJsonObject metrics = entry.value(QStringLiteral("metrics")).toObject();
        for (int column = 0; column < keys.size(); ++column) {
            const QJsonValue value = metrics.value(keys.at(column).toString());
            auto *valItem = new QTableWidgetItem(
                value.isDouble() ? QString::number(value.toDouble(), 'f', 2) : QStringLiteral("—"));
            valItem->setTextAlignment(Qt::AlignCenter);
            valItem->setBackground(rowBg);
            valItem->setForeground(textColor);
            ui->historyTable->setItem(row, column + 1, valItem);
        }
    }

    const int total = history.value(QStringLiteral("total")).toInt();
    ui->recordCountLabel->setText(tr("%1 bản ghi").arg(total));
    const QJsonObject averages = history.value(QStringLiteral("averages")).toObject();
    QStringList summary;
    for (const QJsonValue &key : keys) {
        const QString name = key.toString();
        if (averages.value(name).isDouble())
            summary.append(tr("%1: %2").arg(metricTitle(name),
                QString::number(averages.value(name).toDouble(), 'f', 2)));
    }
    m_primaryStat->setText(summary.value(0, QStringLiteral("--")));
    m_secondaryStat->setText(summary.value(1, QStringLiteral("--")));
    m_thirdStat->setText(summary.size() > 2
        ? summary.value(2)
        : tr("%1 bản ghi").arg(total));

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

    // Determine the single active metric key (no 'all' combination)
    QString activeKey = m_selectedMetricKey;
    if (activeKey.isEmpty() || activeKey == QStringLiteral("all")) {
        for (const QJsonValue &k : keys) {
            const QString keyStr = k.toString();
            if (keyStr != QStringLiteral("ir_detected")) {
                activeKey = keyStr;
                break;
            }
        }
        m_selectedMetricKey = activeKey;
    }

    if (activeKey.isEmpty()) {
        m_chartTitle->setText(tr("Không có dữ liệu biểu đồ"));
        m_chartHint->setText(tr("Các bản ghi không chứa giá trị số phù hợp để vẽ biểu đồ."));
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
            const QString recStr = entry.value(QStringLiteral("recorded_at")).toString();
            const QDate d = QDate::fromString(recStr, QStringLiteral("yyyy-MM-dd"));
            categories << (d.isValid() ? QStringLiteral("Ngày %1").arg(d.day(), 2, 10, QChar('0'))
                                       : recStr);
            const QJsonObject m = entry.value(QStringLiteral("metrics")).toObject();
            const double val = m.value(activeKey).toDouble(0.0);
            *barSet << val;
            overallMin = qMin(overallMin, val);
            overallMax = qMax(overallMax, val);
        }
        m_chartTitle->setText(
            tr("Biểu đồ cột %1 theo ngày · Tháng %2")
                .arg(metricShortName(activeKey), m_cachedSelectedDate.left(7)));
        m_chartHint->setText(
            tr("Mỗi cột đại diện cho giá trị %1 trung bình của một ngày.")
                .arg(metricTitle(activeKey)));
    } else if (currentPeriod == QStringLiteral("year")) {
        for (const QJsonObject &entry : chronologicalRows) {
            const QString recStr = entry.value(QStringLiteral("recorded_at")).toString();
            const int mNum = recStr.mid(5, 2).toInt();
            categories << QStringLiteral("Tháng %1").arg(mNum, 2, 10, QChar('0'));
            const QJsonObject m = entry.value(QStringLiteral("metrics")).toObject();
            const double val = m.value(activeKey).toDouble(0.0);
            *barSet << val;
            overallMin = qMin(overallMin, val);
            overallMax = qMax(overallMax, val);
        }
        m_chartTitle->setText(
            tr("Biểu đồ cột %1 theo tháng · Năm %2")
                .arg(metricShortName(activeKey), m_cachedSelectedDate.left(4)));
        m_chartHint->setText(
            tr("Mỗi cột đại diện cho giá trị %1 trung bình của một tháng.")
                .arg(metricTitle(activeKey)));
    } else {
        // currentPeriod == "day"
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
            m_chartHint->setText(tr("Các bản ghi không chứa giá trị số phù hợp để vẽ biểu đồ."));
            return;
        }

        if (rawSamples.size() <= 20) {
            for (const auto &s : rawSamples) {
                categories.append(s.dt.toString(QStringLiteral("HH:mm:ss")));
                *barSet << s.val;
                overallMin = qMin(overallMin, s.val);
                overallMax = qMax(overallMax, s.val);
            }
        } else {
            int binMinutes = 2;
            QMap<QString, QPair<double, int>> bins;
            QStringList binOrder;

            for (int bm : {2, 5, 10, 15, 30, 60}) {
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
                if (binOrder.size() <= 25) {
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

        m_chartTitle->setText(
            tr("Biểu đồ cột %1 · Ngày %2")
                .arg(metricShortName(activeKey), m_cachedSelectedDate.left(10)));
        m_chartHint->setText(
            tr("Mỗi cột đại diện cho giá trị %1 trung bình theo mốc thời gian trong ngày.")
                .arg(metricTitle(activeKey)));
    }

    const int numBars = categories.size();
    double barWidth = 0.60;
    if (numBars <= 1) {
        barWidth = 0.08;
    } else if (numBars <= 2) {
        barWidth = 0.14;
    } else if (numBars <= 3) {
        barWidth = 0.20;
    } else if (numBars <= 5) {
        barWidth = 0.32;
    } else if (numBars <= 8) {
        barWidth = 0.45;
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

    if (activeKey == QStringLiteral("current_a")) {
        const double diff = overallMax - overallMin;
        const double padding = qMax(0.5, (diff == 0.0 ? 1.0 : diff * 0.15));
        axisY->setRange(qMax(0.0, overallMin - padding), overallMax + padding);
        axisY->setLabelFormat("%.2f A");
    } else if (activeKey == QStringLiteral("voltage_v")) {
        const double diff = overallMax - overallMin;
        const double padding = qMax(1.0, (diff == 0.0 ? 5.0 : diff * 0.15));
        axisY->setRange(qMax(0.0, overallMin - padding), overallMax + padding);
        axisY->setLabelFormat("%.1f V");
    } else if (activeKey == QStringLiteral("power_w")) {
        const double diff = overallMax - overallMin;
        const double padding = qMax(2.0, (diff == 0.0 ? 10.0 : diff * 0.15));
        axisY->setRange(qMax(0.0, overallMin - padding), overallMax + padding);
        axisY->setLabelFormat("%.1f W");
    } else {
        const double diff = overallMax - overallMin;
        const double padding = qMax(0.5, (diff == 0.0 ? 1.0 : diff * 0.15));
        axisY->setRange(qMax(0.0, overallMin - padding), overallMax + padding);
        axisY->setLabelFormat("%.2f");
    }

    m_chart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);
}

QString HistoryPage::currentDeviceType() const
{
    return ui->deviceCombo->currentData(Qt::UserRole + 1).toString();
}

QString HistoryPage::metricTitle(const QString &key)
{
    static const QHash<QString, QString> names{
        {"voltage_v", tr("Điện áp RMS (V)")}, {"current_a", tr("Dòng điện tải (A)")},
        {"power_w", tr("Công suất (W)")}, {"frequency_hz", tr("Tần số (Hz)")},
        {"power_factor", tr("Hệ số cosφ")},
        {"temperature_c", tr("Nhiệt độ (°C)")}, {"humidity_percent", tr("Độ ẩm (%)")},
        {"pressure_hpa", tr("Áp suất (hPa)")}, {"uv_index", tr("UV Index")},
        {"uv_voltage", tr("Điện áp UV (V)")}, {"sound_vpp", tr("Âm thanh (Vpp)")},
        {"distance_cm", tr("Khoảng cách (cm)")}, {"lux", tr("Độ sáng (Lux)")},
        {"flow_l_min", tr("Lưu lượng (L/m)")}, {"total_liters", tr("Tổng nước (L)")},
        {"ir_detected", tr("IR")}};
    return names.value(key, key);
}

QString HistoryPage::compactMetricTitle(const QString &key)
{
    static const QHash<QString, QString> names{
        {"voltage_v", tr("V")}, {"current_a", tr("A")},
        {"power_w", tr("W")}, {"frequency_hz", tr("Hz")},
        {"power_factor", tr("cosφ")},
        {"temperature_c", tr("°C")}, {"humidity_percent", tr("%")},
        {"pressure_hpa", tr("hPa")}, {"uv_index", tr("UV")},
        {"uv_voltage", tr("V")}, {"sound_vpp", tr("Vpp")},
        {"distance_cm", tr("cm")}, {"lux", tr("Lux")},
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

    QStringList plotableKeys;
    for (const QJsonValue &k : m_cachedKeys) {
        const QString keyStr = k.toString();
        if (keyStr != QStringLiteral("ir_detected"))
            plotableKeys.append(keyStr);
    }

    for (const QString &key : plotableKeys) {
        metricCombo->addItem(tr("📊 %1").arg(metricTitle(key)), key);
    }

    QString selectedKey = initialMetricKey.isEmpty() ? m_selectedMetricKey : initialMetricKey;
    if (selectedKey.isEmpty() || metricCombo->findData(selectedKey) < 0)
        selectedKey = plotableKeys.isEmpty() ? QString() : plotableKeys.first();

    int foundIdx = metricCombo->findData(selectedKey);
    if (foundIdx >= 0)
        metricCombo->setCurrentIndex(foundIdx);

    headerLayout->addWidget(metricCombo, 0, Qt::AlignVCenter);

    auto *closeBtn = new QPushButton(QStringLiteral("✕"), &dialog);
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

        const QString activeKey = metricCombo->currentData().toString();
        const QJsonArray &rows = m_cachedRows;

        if (activeKey.isEmpty() || rows.isEmpty()) {
            dialogTitle->setText(tr("Không có dữ liệu biểu đồ"));
            minBadge->hide(); maxBadge->hide(); avgBadge->hide();
            return;
        }

        const QString currentPeriod = m_cachedPeriod.isEmpty()
                                          ? ui->periodCombo->currentData().toString()
                                          : m_cachedPeriod;

        const QColor seriesColor = getColorForKey(activeKey, 0);
        double overallMin = std::numeric_limits<double>::max();
        double overallMax = std::numeric_limits<double>::lowest();
        double overallSum = 0;
        int overallCount = 0;

        auto *barSeries = new QBarSeries(zoomChart);
        QStringList categories;
        QList<QJsonObject> chronologicalRows;
        for (int r = rows.size() - 1; r >= 0; --r) {
            chronologicalRows.append(rows.at(r).toObject());
        }

        auto *barSet = new QBarSet(metricTitle(activeKey));
        barSet->setColor(seriesColor);
        barSet->setBorderColor(seriesColor.lighter(120));

        if (currentPeriod == QStringLiteral("month")) {
            for (const QJsonObject &entry : chronologicalRows) {
                const QString recStr = entry.value(QStringLiteral("recorded_at")).toString();
                const QDate d = QDate::fromString(recStr, QStringLiteral("yyyy-MM-dd"));
                categories << (d.isValid() ? QStringLiteral("Ngày %1").arg(d.day(), 2, 10, QChar('0')) : recStr);
                const QJsonObject m = entry.value(QStringLiteral("metrics")).toObject();
                const double val = m.value(activeKey).toDouble(0.0);
                *barSet << val;
                overallMin = qMin(overallMin, val);
                overallMax = qMax(overallMax, val);
                overallSum += val;
                ++overallCount;
            }
            dialogTitle->setText(tr("Biểu đồ cột chi tiết theo ngày · Tháng %1").arg(m_cachedSelectedDate.left(7)));
        } else if (currentPeriod == QStringLiteral("year")) {
            for (const QJsonObject &entry : chronologicalRows) {
                const QString recStr = entry.value(QStringLiteral("recorded_at")).toString();
                const int mNum = recStr.mid(5, 2).toInt();
                categories << QStringLiteral("Tháng %1").arg(mNum, 2, 10, QChar('0'));
                const QJsonObject m = entry.value(QStringLiteral("metrics")).toObject();
                const double val = m.value(activeKey).toDouble(0.0);
                *barSet << val;
                overallMin = qMin(overallMin, val);
                overallMax = qMax(overallMax, val);
                overallSum += val;
                ++overallCount;
            }
            dialogTitle->setText(tr("Biểu đồ cột chi tiết theo tháng · Năm %1").arg(m_cachedSelectedDate.left(4)));
        } else {
            // currentPeriod == "day"
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
                if (dt.isValid()) rawSamples.append({dt, v.toDouble()});
            }

            if (rawSamples.isEmpty()) {
                dialogTitle->setText(tr("Không có dữ liệu biểu đồ"));
                minBadge->hide(); maxBadge->hide(); avgBadge->hide();
                return;
            }

            if (rawSamples.size() <= 20) {
                for (const auto &s : rawSamples) {
                    categories.append(s.dt.toString(QStringLiteral("HH:mm:ss")));
                    *barSet << s.val;
                    overallMin = qMin(overallMin, s.val);
                    overallMax = qMax(overallMax, s.val);
                    overallSum += s.val;
                    ++overallCount;
                }
            } else {
                int binMinutes = 2;
                QMap<QString, QPair<double, int>> bins;
                QStringList binOrder;

                for (int bm : {2, 5, 10, 15, 30, 60}) {
                    binMinutes = bm;
                    bins.clear();
                    binOrder.clear();
                    for (const auto &s : rawSamples) {
                        int m = s.dt.time().minute();
                        int b = (m / binMinutes) * binMinutes;
                        QTime binTime(s.dt.time().hour(), b, 0);
                        QString binKey = binTime.toString(QStringLiteral("HH:mm"));
                        if (!bins.contains(binKey)) binOrder.append(binKey);
                        bins[binKey].first += s.val;
                        bins[binKey].second += 1;
                    }
                    if (binOrder.size() <= 25) break;
                }

                for (const QString &binKey : binOrder) {
                    categories.append(binKey);
                    double avg = bins[binKey].first / bins[binKey].second;
                    *barSet << avg;
                    overallMin = qMin(overallMin, avg);
                    overallMax = qMax(overallMax, avg);
                    overallSum += avg;
                    ++overallCount;
                }
            }
            dialogTitle->setText(tr("Biểu đồ cột chi tiết %1 · Ngày %2").arg(metricShortName(activeKey), m_cachedSelectedDate.left(10)));
        }

        const int numBars = categories.size();
        double barWidth = 0.60;
        if (numBars <= 1) barWidth = 0.08;
        else if (numBars <= 2) barWidth = 0.14;
        else if (numBars <= 3) barWidth = 0.20;
        else if (numBars <= 5) barWidth = 0.32;
        else if (numBars <= 8) barWidth = 0.45;
        else barWidth = 0.65;
        barSeries->setBarWidth(barWidth);

        barSeries->append(barSet);
        zoomChart->addSeries(barSeries);

        auto *axisX = new QBarCategoryAxis(zoomChart);
        QFont axisFont;
        axisFont.setPixelSize(10);
        axisX->setLabelsFont(axisFont);
        axisX->setLabelsColor(QColor("#94a3b8"));
        axisX->setGridLineColor(QColor("#1c2b54"));
        axisX->append(categories);
        zoomChart->addAxis(axisX, Qt::AlignBottom);
        barSeries->attachAxis(axisX);

        auto *axisY = new QValueAxis(zoomChart);
        axisY->setLabelsFont(axisFont);
        axisY->setLabelsColor(seriesColor);
        axisY->setTitleBrush(seriesColor);
        axisY->setTitleFont(axisFont);
        axisY->setGridLineColor(QColor("#1c2b54"));
        axisY->setTitleText(compactMetricTitle(activeKey));

        if (overallMin > overallMax) { overallMin = 0.0; overallMax = 10.0; }
        const double diff = overallMax - overallMin;
        const double padding = qMax(0.5, (diff == 0.0 ? 1.0 : diff * 0.15));
        axisY->setRange(qMax(0.0, overallMin - padding), overallMax + padding);
        axisY->setLabelFormat("%.2f");
        zoomChart->addAxis(axisY, Qt::AlignLeft);
        barSeries->attachAxis(axisY);

        if (overallCount > 0) {
            const QString unit = compactMetricTitle(activeKey);
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

void HistoryPage::showTableView()
{
    ui->tableTabButton->setChecked(true);
    ui->viewStack->setCurrentIndex(1);
}

