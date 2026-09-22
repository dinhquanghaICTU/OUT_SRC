#include "HistoryPage.h"

#include "ui_HistoryPage.h"

#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QButtonGroup>
#include <QChart>
#include <QChartView>
#include <QDate>
#include <QDateTime>
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
#include <QTimer>
#include <QValueAxis>
#include <QVBoxLayout>

#include <cmath>
#include <limits>

namespace {
static QColor getColorForKey(const QString &k, int fallbackIdx = 0) {
    if (k == QStringLiteral("distance_cm")) return QColor("#10b981"); // Xanh lục Khoảng cách (Theme Son)
    if (k == QStringLiteral("flow_l_min")) return QColor("#0284c7");   // Xanh dương Lưu lượng
    if (k == QStringLiteral("total_liters")) return QColor("#f59e0b"); // Cam Vàng Tổng nước
    if (k == QStringLiteral("temperature_c")) return QColor("#ef4444"); // Đỏ Nhiệt độ
    if (k == QStringLiteral("humidity_percent")) return QColor("#06b6d4"); // Cyan Độ ẩm
    static const QList<QColor> fallback{QColor("#10b981"), QColor("#0284c7"), QColor("#f59e0b"), QColor("#8b5cf6")};
    return fallback.at(fallbackIdx % fallback.size());
}

static QString metricShortName(const QString &key) {
    static const QHash<QString, QString> names{
        {QStringLiteral("distance_cm"), QObject::tr("Khoảng cách")},
        {QStringLiteral("flow_l_min"), QObject::tr("Lưu lượng")},
        {QStringLiteral("total_liters"), QObject::tr("Tổng nước")},
        {QStringLiteral("temperature_c"), QObject::tr("Nhiệt độ")},
        {QStringLiteral("humidity_percent"), QObject::tr("Độ ẩm")}};
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
    m_headerSubtitle->hide();

    ui->recordCountLabel->setObjectName(QStringLiteral("historyRecordBadge"));
    ui->chartTabButton->setObjectName(QStringLiteral("deviceViewTabButton"));
    ui->tableTabButton->setObjectName(QStringLiteral("deviceViewTabButton"));
    ui->deviceCombo->setObjectName(QStringLiteral("historyDeviceCombo"));
    ui->periodCombo->setObjectName(QStringLiteral("historyPeriodCombo"));
    ui->dateCombo->setObjectName(QStringLiteral("historyDateCombo"));
    ui->searchButton->setObjectName(QStringLiteral("historySearchButton"));
    ui->filterLayout->setSpacing(6);

    auto *tabGroup = new QButtonGroup(this);
    tabGroup->addButton(ui->chartTabButton);
    tabGroup->addButton(ui->tableTabButton);
    tabGroup->setExclusive(true);

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

    auto makeStatCard = [this](QLabel *&titleOut, const QString &defaultTitle, QLabel *value, const QString &icon) {
        auto *card = new QFrame(this);
        card->setObjectName(QStringLiteral("historyStatCard"));
        card->setCursor(Qt::PointingHandCursor);
        card->setToolTip(tr("Bấm để xem phóng to biểu đồ chỉ số này"));
        auto *layout = new QHBoxLayout(card);
        layout->setContentsMargins(8, 4, 8, 4);
        layout->setSpacing(6);
        auto *iconLabel = new QLabel(icon, card);
        iconLabel->setObjectName(QStringLiteral("historyStatIcon"));
        iconLabel->setAlignment(Qt::AlignCenter);

        auto *textLayout = new QVBoxLayout;
        textLayout->setContentsMargins(0, 0, 0, 0);
        textLayout->setSpacing(1);
        titleOut = new QLabel(defaultTitle, card);
        titleOut->setObjectName(QStringLiteral("historyStatTitle"));
        value->setObjectName(QStringLiteral("historyStatValue"));
        value->setText(QStringLiteral("--"));
        value->setWordWrap(false);
        textLayout->addWidget(titleOut);
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
    m_chart->legend()->setVisible(false); // Clean modern look matching Trung Kien
    m_chart->setMargins(QMargins(4, 4, 4, 4));
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
    m_metricCombo->setMinimumWidth(130);
    m_metricCombo->hide();
    m_chartHeaderLayout->addWidget(m_metricCombo, 0, Qt::AlignVCenter | Qt::AlignRight);

    m_zoomBtn = new QPushButton(tr("⛶ Phóng to"), m_chartCard);
    m_zoomBtn->setObjectName(QStringLiteral("historyZoomButton"));
    m_zoomBtn->setCursor(Qt::PointingHandCursor);
    m_zoomBtn->setToolTip(tr("Phóng to biểu đồ"));
    connect(m_zoomBtn, &QPushButton::clicked, this, [this] {
        openChartZoomDialog(m_selectedMetricKey);
    });
    m_chartHeaderLayout->addWidget(m_zoomBtn, 0, Qt::AlignVCenter | Qt::AlignRight);

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
    m_primaryStatCard = makeStatCard(m_primaryStatTitle, tr("Chỉ số chính"), m_primaryStat, QStringLiteral("↯"));
    m_secondaryStatCard = makeStatCard(m_secondaryStatTitle, tr("Chỉ số phụ"), m_secondaryStat, QStringLiteral("◍"));
    m_summaryStatCard = makeStatCard(m_summaryStatTitle, tr("Tóm tắt"), m_thirdStat, QStringLiteral("▥"));
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

    ui->periodCombo->clear();
    ui->periodCombo->addItem(tr("Ngày"), QStringLiteral("day"));
    ui->periodCombo->addItem(tr("Tháng"), QStringLiteral("month"));
    ui->periodCombo->addItem(tr("Năm"), QStringLiteral("year"));

    rebuildDateOptions();

    connect(ui->searchButton, &QPushButton::clicked, this, &HistoryPage::requestCurrentHistory);
    connect(ui->deviceCombo, &QComboBox::currentIndexChanged, this, [this](int) { requestCurrentHistory(); });
    connect(ui->periodCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        rebuildDateOptions();
        requestCurrentHistory();
    });
    connect(ui->dateCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        const QVariant val = ui->dateCombo->currentData();
        if (val.isValid() && val.canConvert<QDate>()) {
            m_selectedDate = val.toDate();
        }
        requestCurrentHistory();
    });

    m_liveTimer = new QTimer(this);
    m_liveTimer->setInterval(3000);
    connect(m_liveTimer, &QTimer::timeout, this, [this] {
        if (!isVisible()) return;
        requestCurrentHistory();
    });

    applyResponsiveLayout();
}

bool HistoryPage::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
        if (m_chartView && watched == m_chartView->viewport()) {
            openChartZoomDialog(m_selectedMetricKey);
            return true;
        }
        if (watched == m_primaryStatCard) {
            QString target = QStringLiteral("distance_cm");
            if (m_cachedKeys.contains(target)) {
                setMetric(target);
                openChartZoomDialog(target);
            } else if (!m_cachedKeys.isEmpty()) {
                openChartZoomDialog(m_cachedKeys.first().toString());
            }
            return true;
        }
        if (watched == m_secondaryStatCard) {
            QString target = QStringLiteral("flow_l_min");
            if (m_cachedKeys.contains(target)) {
                setMetric(target);
                openChartZoomDialog(target);
            } else if (m_cachedKeys.size() > 1) {
                openChartZoomDialog(m_cachedKeys.at(1).toString());
            }
            return true;
        }
        if (watched == m_summaryStatCard) {
            QString target = QStringLiteral("total_liters");
            if (m_cachedKeys.contains(target)) {
                setMetric(target);
                openChartZoomDialog(target);
            } else if (m_cachedKeys.size() > 2) {
                openChartZoomDialog(m_cachedKeys.at(2).toString());
            } else {
                openChartZoomDialog(m_selectedMetricKey);
            }
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

    ui->deviceCombo->setMinimumWidth(compact ? 140 : 210);
    ui->deviceCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->periodCombo->setMinimumWidth(compact ? 65 : 85);
    ui->dateCombo->setMinimumWidth(compact ? 120 : 155);
    ui->searchButton->setMinimumWidth(compact ? 48 : 70);
    if (m_metricCombo)
        m_metricCombo->setMinimumWidth(compact ? 120 : 150);
    if (m_zoomBtn)
        m_zoomBtn->setMinimumWidth(compact ? 75 : 90);
    m_chartView->setMinimumHeight(compact ? 190 : 255);
    ui->historyTable->setMinimumHeight(compact ? 100 : 160);
    ui->verticalLayout->setContentsMargins(compact ? 6 : 12, compact ? 4 : 8,
                                           compact ? 6 : 12, compact ? 4 : 8);
    ui->verticalLayout->setSpacing(compact ? 4 : 8);
}

HistoryPage::~HistoryPage()
{
    delete ui;
}

void HistoryPage::setViewTab(int tabIndex)
{
    if (tabIndex == 1) {
        ui->tableTabButton->setChecked(true);
        ui->viewStack->setCurrentIndex(1);
        if (m_metricCombo) m_metricCombo->hide();
        if (m_zoomBtn) m_zoomBtn->hide();
    } else {
        ui->chartTabButton->setChecked(true);
        ui->viewStack->setCurrentIndex(0);
        if (m_metricCombo && m_metricCombo->count() > 0) m_metricCombo->show();
        if (m_zoomBtn) m_zoomBtn->show();
    }
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
        const bool online = device.value(QStringLiteral("online")).toBool(false);
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

void HistoryPage::requestCurrentHistory()
{
    const QString deviceId = ui->deviceCombo->currentData().toString();
    const QString period = ui->periodCombo->currentData().toString();
    const QDate curDate = selectedDate();
    const bool isToday = (curDate == QDate::currentDate());
    const bool isOnline = m_deviceOnline.value(deviceId, false);

    if (m_liveTimer) {
        if (period == QStringLiteral("day") && isToday && !deviceId.isEmpty() && isOnline) {
            if (!m_liveTimer->isActive())
                m_liveTimer->start();
        } else {
            if (m_liveTimer->isActive())
                m_liveTimer->stop();
        }
    }

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
            QString cleanName = metricTitle(key);
            m_metricCombo->addItem(tr("📊 %1").arg(cleanName), key);
        }
        int idx = m_metricCombo->findData(m_selectedMetricKey);
        if (idx >= 0 && m_selectedMetricKey != QStringLiteral("all")) {
            m_metricCombo->setCurrentIndex(idx);
        } else {
            int defaultIdx = m_metricCombo->findData(QStringLiteral("distance_cm"));
            if (defaultIdx < 0)
                defaultIdx = m_metricCombo->findData(QStringLiteral("flow_l_min"));
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

    const QString devId = ui->deviceCombo->currentData().toString();
    const bool isOnline = m_deviceOnline.value(devId, false);

    const QString addedBy = history.value(QStringLiteral("added_by")).toString();
    const QString addedAt = history.value(QStringLiteral("added_at")).toString();
    if (!addedBy.isEmpty() && addedBy != QStringLiteral("Chưa gán")) {
        QDateTime addTime = QDateTime::fromString(addedAt, Qt::ISODateWithMs);
        if (!addTime.isValid()) addTime = QDateTime::fromString(addedAt, Qt::ISODate);
        const QString addTimeStr = addTime.isValid() ? addTime.toLocalTime().toString(QStringLiteral("dd/MM/yyyy HH:mm")) : addedAt;
        const QString statusHint = isOnline
            ? tr("● Trực tuyến (Đang cập nhật thời gian thực)")
            : tr("○ Ngoại tuyến (Đã ngắt kết nối · Dừng cập nhật)");
        m_headerSubtitle->setText(
            tr("Thiết bị: %1 · %2 · Người thêm: %3 (%4) · Bấm vào biểu đồ để phóng to.")
                .arg(ui->deviceCombo->currentText(), statusHint, addedBy, addTimeStr));
    }

    ui->historyTable->clear();
    ui->historyTable->setRowCount(rows.size());
    ui->historyTable->setColumnCount(keys.size() + 1);
    ui->historyTable->setAlternatingRowColors(false);

    QStringList headers;
    if (m_cachedPeriod == QStringLiteral("day")) {
        headers.append(tr("Thời gian"));
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
        QString recordedAtStr = entry.value(QStringLiteral("recorded_at")).toString();
        QString displayTime = recordedAtStr;
        if (m_cachedPeriod == QStringLiteral("month")) {
            displayTime = entry.value(QStringLiteral("label")).toString();
            if (displayTime.isEmpty()) displayTime = recordedAtStr;
        } else if (m_cachedPeriod == QStringLiteral("year")) {
            displayTime = entry.value(QStringLiteral("label")).toString();
            if (displayTime.isEmpty()) displayTime = recordedAtStr;
        } else {
            QDateTime time = QDateTime::fromString(recordedAtStr, Qt::ISODateWithMs);
            if (!time.isValid()) time = QDateTime::fromString(recordedAtStr, Qt::ISODate);
            if (!time.isValid()) time = QDateTime::fromString(recordedAtStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            if (time.isValid()) {
                displayTime = time.toLocalTime().toString(QStringLiteral("dd/MM/yyyy HH:mm:ss"));
            }
        }
        ui->historyTable->setItem(row, 0, new QTableWidgetItem(displayTime));
        const QJsonObject metrics = entry.value(QStringLiteral("metrics")).toObject();
        for (int column = 0; column < keys.size(); ++column) {
            const QJsonValue value = metrics.value(keys.at(column).toString());
            ui->historyTable->setItem(row, column + 1, new QTableWidgetItem(
                value.isDouble() ? QString::number(value.toDouble(), 'f', 2) : QStringLiteral("—")));
        }
    }

    const int total = history.value(QStringLiteral("total")).toInt();
    ui->recordCountLabel->setText(tr("%1 bản ghi").arg(total));

    const QJsonObject averages = history.value(QStringLiteral("averages")).toObject();
    if (averages.value(QStringLiteral("distance_cm")).isDouble()) {
        if (m_primaryStatTitle) m_primaryStatTitle->setText(tr("KHOẢNG CÁCH (TB)"));
        m_primaryStat->setText(QStringLiteral("%1 cm").arg(QString::number(averages.value(QStringLiteral("distance_cm")).toDouble(), 'f', 1)));
        m_primaryStat->setStyleSheet(QStringLiteral("color: #10b981; font-size: 13px; font-weight: 900;"));
    } else {
        if (m_primaryStatTitle) m_primaryStatTitle->setText(tr("KHOẢNG CÁCH"));
        m_primaryStat->setText(QStringLiteral("-- cm"));
        m_primaryStat->setStyleSheet(QStringLiteral("color: #94a3b8; font-size: 13px; font-weight: 900;"));
    }

    if (averages.value(QStringLiteral("flow_l_min")).isDouble()) {
        if (m_secondaryStatTitle) m_secondaryStatTitle->setText(tr("LƯU LƯỢNG (TB)"));
        m_secondaryStat->setText(QStringLiteral("%1 L/m").arg(QString::number(averages.value(QStringLiteral("flow_l_min")).toDouble(), 'f', 2)));
        m_secondaryStat->setStyleSheet(QStringLiteral("color: #0284c7; font-size: 13px; font-weight: 900;"));
    } else {
        if (m_secondaryStatTitle) m_secondaryStatTitle->setText(tr("LƯU LƯỢNG"));
        m_secondaryStat->setText(QStringLiteral("-- L/m"));
        m_secondaryStat->setStyleSheet(QStringLiteral("color: #94a3b8; font-size: 13px; font-weight: 900;"));
    }

    if (averages.value(QStringLiteral("total_liters")).isDouble()) {
        if (m_summaryStatTitle) m_summaryStatTitle->setText(tr("TỔNG NƯỚC BƠM"));
        m_thirdStat->setText(QStringLiteral("%1 L").arg(QString::number(averages.value(QStringLiteral("total_liters")).toDouble(), 'f', 2)));
        m_thirdStat->setStyleSheet(QStringLiteral("color: #f59e0b; font-size: 13px; font-weight: 900;"));
    } else {
        if (m_summaryStatTitle) m_summaryStatTitle->setText(tr("TỔNG SỐ BẢN GHI"));
        m_thirdStat->setText(tr("%1 mẫu").arg(total));
        m_thirdStat->setStyleSheet(QStringLiteral("color: #172b22; font-size: 13px; font-weight: 900;"));
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
    const QJsonArray &keys = m_cachedKeys;
    const QJsonArray &rows = m_cachedRows;

    if (keys.isEmpty() || rows.isEmpty()) {
        m_chartTitle->setText(tr("Không có dữ liệu"));
        m_chartHint->setText(tr("Không có dữ liệu trong khoảng thời gian đã chọn."));
        return;
    }

    // Determine the active metric key
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
        const QDate today = QDate::currentDate();
        for (const QJsonObject &entry : chronologicalRows) {
            const QString recStr = entry.value(QStringLiteral("recorded_at")).toString();
            const QDate d = QDate::fromString(recStr, QStringLiteral("yyyy-MM-dd"));
            if (d.isValid() && d > today) {
                continue; // Không hiển thị các ngày trong tương lai
            }
            categories << (d.isValid() ? QStringLiteral("Ngày %1").arg(d.day(), 2, 10, QChar('0')) : recStr);
            const QJsonObject m = entry.value(QStringLiteral("metrics")).toObject();
            const double val = m.value(activeKey).toDouble(0.0);
            *barSet << val;
            overallMin = qMin(overallMin, val);
            overallMax = qMax(overallMax, val);
        }
        const QDate parsedMonth = QDate::fromString(m_cachedSelectedDate.left(7), QStringLiteral("yyyy-MM"));
        const QString formattedMonth = parsedMonth.isValid() ? parsedMonth.toString(QStringLiteral("MM/yyyy")) : m_cachedSelectedDate.left(7);
        m_chartTitle->setText(tr("Biểu đồ cột %1 theo ngày · Tháng %2").arg(metricShortName(activeKey), formattedMonth));
        m_chartHint->setText(tr("Mỗi cột đại diện cho giá trị %1 trung bình của một ngày.").arg(metricTitle(activeKey)));
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
        m_chartTitle->setText(tr("Biểu đồ cột %1 theo tháng · Năm %2").arg(metricShortName(activeKey), m_cachedSelectedDate.left(4)));
        m_chartHint->setText(tr("Mỗi cột đại diện cho giá trị %1 trung bình của một tháng.").arg(metricTitle(activeKey)));
    } else {
        // currentPeriod == "day" -> Biểu đồ cột tổng hợp theo khoảng thời gian thông minh (chống nhiễu)
        struct RawSample {
            QDateTime dt;
            double val;
        };
        QList<RawSample> rawSamples;
        for (const QJsonObject &entry : chronologicalRows) {
            const QJsonValue v = entry.value(QStringLiteral("metrics")).toObject().value(activeKey);
            if (!v.isDouble()) continue;
            double num = v.toDouble();

            // Lọc nhiễu cảm biến (sensor noise & timeouts HC-SR04)
            if (activeKey == QStringLiteral("distance_cm")) {
                if (num < 0.0 || num > 300.0) continue; // HC-SR04 timeout / ngoài tầm đo
            } else if (activeKey == QStringLiteral("flow_l_min")) {
                if (num < 0.0 || num > 60.0) continue;
            } else if (activeKey == QStringLiteral("total_liters")) {
                if (num < 0.0) continue;
            }

            QString recStr = entry.value(QStringLiteral("recorded_at")).toString();
            QDateTime dt = QDateTime::fromString(recStr, Qt::ISODateWithMs);
            if (!dt.isValid()) dt = QDateTime::fromString(recStr, Qt::ISODate);
            if (!dt.isValid()) dt = QDateTime::fromString(recStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            dt = dt.toLocalTime();
            if (dt.isValid()) {
                rawSamples.append({dt, num});
            }
        }

        if (rawSamples.isEmpty()) {
            m_chartTitle->setText(tr("Không có dữ liệu biểu đồ"));
            m_chartHint->setText(tr("Các bản ghi không chứa giá trị số phù hợp để vẽ biểu đồ."));
            return;
        }

        if (rawSamples.size() <= 20) {
            bool hasDuplicateMinutes = false;
            QSet<QString> seenMinutes;
            for (const auto &s : rawSamples) {
                QString hm = s.dt.toString(QStringLiteral("HH:mm"));
                if (seenMinutes.contains(hm)) {
                    hasDuplicateMinutes = true;
                    break;
                }
                seenMinutes.insert(hm);
            }
            for (const auto &s : rawSamples) {
                categories.append(s.dt.toString(hasDuplicateMinutes ? QStringLiteral("HH:mm:ss") : QStringLiteral("HH:mm")));
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

        const QDate parsedDate = QDate::fromString(m_cachedSelectedDate.left(10), Qt::ISODate);
        const QString formattedDate = parsedDate.isValid() ? parsedDate.toString(QStringLiteral("dd/MM/yyyy")) : m_cachedSelectedDate.left(10);
        m_chartTitle->setText(tr("Biểu đồ cột %1 · Ngày %2").arg(metricShortName(activeKey), formattedDate));
        m_chartHint->setText(tr("Mỗi cột đại diện cho giá trị %1 trung bình theo mốc thời gian trong ngày.").arg(metricTitle(activeKey)));
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
    axisX->setLabelsColor(QColor("#71837b"));
    axisX->setGridLineColor(QColor("#d3dfda"));
    axisX->append(categories);
    m_chart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    auto *axisY = new QValueAxis(m_chart);
    axisY->setLabelsFont(axisFont);
    axisY->setLabelsColor(col);
    axisY->setTitleBrush(col);
    axisY->setTitleFont(axisFont);
    axisY->setGridLineColor(QColor("#d3dfda"));
    axisY->setTitleText(compactMetricTitle(activeKey));

    if (overallMin > overallMax) {
        overallMin = 0.0;
        overallMax = 10.0;
    }

    // Thiết lập giới hạn Y không bao giờ âm (loại bỏ giá trị vô lý như -63.8 cm)
    if (activeKey == QStringLiteral("distance_cm")) {
        axisY->setRange(0.0, qMax(40.0, overallMax + 5.0));
        axisY->setLabelFormat("%.1f");
    } else if (activeKey == QStringLiteral("flow_l_min")) {
        axisY->setRange(0.0, qMax(10.0, overallMax + 1.0));
        axisY->setLabelFormat("%.2f");
    } else if (activeKey == QStringLiteral("total_liters")) {
        axisY->setRange(0.0, qMax(20.0, overallMax * 1.1));
        axisY->setLabelFormat("%.1f");
    } else {
        const double diff = overallMax - overallMin;
        const double padding = qMax(0.5, (diff == 0.0 ? (qAbs(overallMax) > 0 ? qAbs(overallMax) * 0.15 + 0.5 : 1.0) : diff * 0.15));
        axisY->setRange(qMax(0.0, overallMin - padding), overallMax + padding);
        axisY->setLabelFormat("%.1f");
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
        {"temperature_c", tr("Nhiệt độ (°C)")}, {"humidity_percent", tr("Độ ẩm (%)")},
        {"pressure_hpa", tr("Áp suất (hPa)")}, {"uv_index", tr("UV Index")},
        {"uv_voltage", tr("Điện áp UV (V)")}, {"sound_vpp", tr("Âm thanh (Vpp)")},
        {"current_a", tr("Dòng điện (A)")}, {"voltage_v", tr("Điện áp (V)")},
        {"distance_cm", tr("Khoảng cách (cm)")}, {"lux", tr("Độ sáng (Lux)")},
        {"flow_l_min", tr("Lưu lượng (L/m)")}, {"total_liters", tr("Tổng nước (L)")},
        {"ir_detected", tr("IR")}};
    return names.value(key, key);
}

QString HistoryPage::compactMetricTitle(const QString &key)
{
    static const QHash<QString, QString> names{
        {"temperature_c", tr("°C")}, {"humidity_percent", tr("%")},
        {"pressure_hpa", tr("hPa")}, {"uv_index", tr("UV")},
        {"uv_voltage", tr("V")}, {"sound_vpp", tr("Vpp")},
        {"current_a", tr("A")}, {"voltage_v", tr("V")},
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
        metricCombo->addItem(tr("📈 %1").arg(metricTitle(key)), key);
    }

    QString selectedKey = initialMetricKey.isEmpty() ? m_selectedMetricKey : initialMetricKey;
    if (selectedKey.isEmpty() || selectedKey == QStringLiteral("all") || metricCombo->findData(selectedKey) < 0)
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
    zoomChart->legend()->setVisible(false);

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
        const QJsonArray &keys = m_cachedKeys;
        const QJsonArray &rows = m_cachedRows;

        QString activeKey = activeMetric;
        if (activeKey.isEmpty() || activeKey == QStringLiteral("all")) {
            for (const QJsonValue &k : keys) {
                const QString keyStr = k.toString();
                if (keyStr != QStringLiteral("ir_detected")) {
                    activeKey = keyStr;
                    break;
                }
            }
        }

        dialogTitle->setText(tr("Biểu đồ chi tiết: %1").arg(metricTitle(activeKey)));

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
            const QDate today = QDate::currentDate();
            for (const QJsonObject &entry : chronologicalRows) {
                const QString recStr = entry.value(QStringLiteral("recorded_at")).toString();
                const QDate d = QDate::fromString(recStr, QStringLiteral("yyyy-MM-dd"));
                if (d.isValid() && d > today) {
                    continue; // Không hiển thị các ngày trong tương lai
                }
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
                double num = v.toDouble();

                if (activeKey == QStringLiteral("distance_cm")) {
                    if (num < 0.0 || num > 300.0) continue;
                } else if (activeKey == QStringLiteral("flow_l_min")) {
                    if (num < 0.0 || num > 60.0) continue;
                } else if (activeKey == QStringLiteral("total_liters")) {
                    if (num < 0.0) continue;
                }

                QString recStr = entry.value(QStringLiteral("recorded_at")).toString();
                QDateTime dt = QDateTime::fromString(recStr, Qt::ISODateWithMs);
                if (!dt.isValid()) dt = QDateTime::fromString(recStr, Qt::ISODate);
                if (!dt.isValid()) dt = QDateTime::fromString(recStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
                dt = dt.toLocalTime();
                if (dt.isValid()) {
                    rawSamples.append({dt, num});
                }
            }

            if (rawSamples.size() <= 20) {
                bool hasDuplicateMinutes = false;
                QSet<QString> seenMinutes;
                for (const auto &s : rawSamples) {
                    QString hm = s.dt.toString(QStringLiteral("HH:mm"));
                    if (seenMinutes.contains(hm)) {
                        hasDuplicateMinutes = true;
                        break;
                    }
                    seenMinutes.insert(hm);
                }
                for (const auto &s : rawSamples) {
                    categories.append(s.dt.toString(hasDuplicateMinutes ? QStringLiteral("HH:mm:ss") : QStringLiteral("HH:mm")));
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
                    overallSum += avg;
                    ++overallCount;
                }
            }
            const QDate parsedDate = QDate::fromString(m_cachedSelectedDate.left(10), Qt::ISODate);
            const QString formattedDate = parsedDate.isValid() ? parsedDate.toString(QStringLiteral("dd/MM/yyyy")) : m_cachedSelectedDate.left(10);
            dialogTitle->setText(tr("Biểu đồ chi tiết: %1 · Ngày %2").arg(metricTitle(activeKey), formattedDate));
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
        axisX->setLabelsColor(QColor("#71837b"));
        axisX->setGridLineColor(QColor("#d3dfda"));
        axisX->append(categories);
        zoomChart->addAxis(axisX, Qt::AlignBottom);
        barSeries->attachAxis(axisX);

        auto *axisY = new QValueAxis(zoomChart);
        axisY->setLabelsFont(axisFont);
        axisY->setLabelsColor(seriesColor);
        axisY->setTitleBrush(seriesColor);
        axisY->setTitleFont(axisFont);
        axisY->setGridLineColor(QColor("#d3dfda"));
        axisY->setTitleText(compactMetricTitle(activeKey));

        if (overallMin > overallMax) {
            overallMin = 0.0;
            overallMax = 10.0;
        }

        if (activeKey == QStringLiteral("distance_cm")) {
            axisY->setRange(0.0, qMax(40.0, overallMax + 5.0));
            axisY->setLabelFormat("%.1f");
        } else if (activeKey == QStringLiteral("flow_l_min")) {
            axisY->setRange(0.0, qMax(10.0, overallMax + 1.0));
            axisY->setLabelFormat("%.2f");
        } else if (activeKey == QStringLiteral("total_liters")) {
            axisY->setRange(0.0, qMax(20.0, overallMax * 1.1));
            axisY->setLabelFormat("%.1f");
        } else {
            const double diff = overallMax - overallMin;
            const double padding = qMax(0.5, (diff == 0.0 ? (qAbs(overallMax) > 0 ? qAbs(overallMax) * 0.15 + 0.5 : 1.0) : diff * 0.15));
            axisY->setRange(qMax(0.0, overallMin - padding), overallMax + padding);
            axisY->setLabelFormat("%.1f");
        }
        zoomChart->addAxis(axisY, Qt::AlignLeft);
        barSeries->attachAxis(axisY);

        const QString unit = metricUnit(activeKey);
        if (overallCount > 0) {
            minBadge->setText(tr("Min: %1 %2").arg(QString::number(overallMin, 'f', 2), unit));
            maxBadge->setText(tr("Max: %1 %2").arg(QString::number(overallMax, 'f', 2), unit));
            avgBadge->setText(tr("TB: %1 %2").arg(QString::number(overallSum / overallCount, 'f', 2), unit));
        } else {
            minBadge->setText(tr("Min: --"));
            maxBadge->setText(tr("Max: --"));
            avgBadge->setText(tr("TB: --"));
        }
    };

    connect(metricCombo, &QComboBox::currentIndexChanged, &dialog, [renderZoomChart](int) {
        renderZoomChart();
    });

    renderZoomChart();
    dialog.exec();
}
