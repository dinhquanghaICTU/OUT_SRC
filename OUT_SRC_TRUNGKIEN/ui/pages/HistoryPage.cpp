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
            this, [this](int) { requestCurrentHistory(); });
    connect(ui->dateEdit, &QDateEdit::dateChanged,
            this, [this](const QDate &) { requestCurrentHistory(); });
    connect(ui->deviceCombo, &QComboBox::currentIndexChanged,
            this, [this](int) { requestCurrentHistory(); });
    connect(ui->periodCombo, &QComboBox::currentIndexChanged,
            this, [this](int) { requestCurrentHistory(); });
    connect(ui->dateEdit, &QDateEdit::dateChanged,
            this, [this](const QDate &) { requestCurrentHistory(); });
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
            openChartZoomDialog(plotable.value(0, m_selectedMetricKey));
            return true;
        }
        if (watched == m_secondaryStatCard) {
            QStringList plotable;
            for (const QJsonValue &k : m_cachedKeys)
                if (k.toString() != QStringLiteral("ir_detected")) plotable.append(k.toString());
            openChartZoomDialog(plotable.value(1, m_selectedMetricKey));
            return true;
        }
        if (watched == m_summaryStatCard) {
            QStringList plotable;
            for (const QJsonValue &k : m_cachedKeys)
                if (k.toString() != QStringLiteral("ir_detected")) plotable.append(k.toString());
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

    if (plotableKeys.size() > 1) {
        m_metricCombo->addItem(tr("📊 Tất cả chỉ số"), QStringLiteral("all"));
        for (const QString &key : plotableKeys) {
            m_metricCombo->addItem(tr("📈 %1").arg(metricTitle(key)), key);
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
    QStringList headers{tr("Thời gian")};
    for (const QJsonValue &key : keys)
        headers.append(metricTitle(key.toString()));
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
        ui->historyTable->setItem(row, 0, new QTableWidgetItem(
            time.isValid() ? time.toString(QStringLiteral("dd/MM/yyyy HH:mm:ss")) : recordedAtStr));
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

    const bool isIrOnly = keys.size() == 1 && keys.at(0).toString() == QStringLiteral("ir_detected");
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
            *set << entry.value(QStringLiteral("metrics")).toObject()
                        .value(QStringLiteral("ir_detected")).toInt();
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
    m_chart->addAxis(axisX, Qt::AlignBottom);

    qint64 minimumTime = std::numeric_limits<qint64>::max();
    qint64 maximumTime = std::numeric_limits<qint64>::min();
    const QList<QColor> colors{QColor("#15945a"), QColor("#2d9cdb"),
                               QColor("#e0a025"), QColor("#6750d8"),
                               QColor("#d84d76"), QColor("#64748b")};

    QStringList activeKeys;
    if (m_selectedMetricKey.isEmpty() || m_selectedMetricKey == QStringLiteral("all")) {
        for (const QJsonValue &k : keys) {
            const QString keyStr = k.toString();
            if (keyStr != QStringLiteral("ir_detected"))
                activeKeys.append(keyStr);
        }
    } else {
        activeKeys.append(m_selectedMetricKey);
    }

    int visibleSeries = 0;
    for (int metricIndex = 0; metricIndex < activeKeys.size(); ++metricIndex) {
        const QString key = activeKeys.at(metricIndex);
        auto *series = new QLineSeries(m_chart);
        series->setName(metricTitle(key));
        series->setPointsVisible(true);
        series->setMarkerSize(5.0);
        QPen pen(colors.at(metricIndex % colors.size()));
        pen.setWidthF(2.0);
        series->setPen(pen);
        double minimum = std::numeric_limits<double>::max();
        double maximum = std::numeric_limits<double>::lowest();
        for (int row = rows.size() - 1; row >= 0; --row) {
            const QJsonObject entry = rows.at(row).toObject();
            const QJsonValue value = entry.value(QStringLiteral("metrics")).toObject().value(key);
            QString recordedAtStr = entry.value(QStringLiteral("recorded_at")).toString();
            QDateTime time = QDateTime::fromString(recordedAtStr, Qt::ISODateWithMs);
            if (!time.isValid())
                time = QDateTime::fromString(recordedAtStr, Qt::ISODate);
            if (!time.isValid())
                time = QDateTime::fromString(recordedAtStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            time = time.toLocalTime();
            if (!value.isDouble() || !time.isValid())
                continue;
            const qint64 timestamp = time.toMSecsSinceEpoch();
            const double number = value.toDouble();
            series->append(timestamp, number);
            minimumTime = qMin(minimumTime, timestamp);
            maximumTime = qMax(maximumTime, timestamp);
            minimum = qMin(minimum, number);
            maximum = qMax(maximum, number);
        }
        if (series->count() == 0) {
            delete series;
            continue;
        }
        m_chart->addSeries(series);
        series->attachAxis(axisX);
        auto *axisY = new QValueAxis(m_chart);
        axisY->setTitleText(compactMetricTitle(key));
        axisY->setLabelsColor(colors.at(metricIndex % colors.size()));
        axisY->setTitleBrush(colors.at(metricIndex % colors.size()));
        axisY->setLabelsFont(axisFont);
        axisY->setTitleFont(axisFont);
        if (minimum > maximum) {
            minimum = 0;
            maximum = 10;
        }
        const double diff = maximum - minimum;
        const double padding = qMax(0.5, (diff == 0.0 ? (qAbs(maximum) > 0 ? qAbs(maximum) * 0.15 + 0.5 : 1.0) : diff * 0.15));
        axisY->setRange(minimum - padding, maximum + padding);
        axisY->setLabelFormat("%.1f");
        m_chart->addAxis(axisY, visibleSeries == 0 ? Qt::AlignLeft : Qt::AlignRight);
        series->attachAxis(axisY);
        ++visibleSeries;
    }

    if (visibleSeries == 0) {
        m_chartTitle->setText(tr("Không có dữ liệu biểu đồ"));
        m_chartHint->setText(tr("Các bản ghi không chứa giá trị số phù hợp để vẽ biểu đồ."));
        return;
    }

    if (minimumTime <= maximumTime) {
        const qint64 timeSpan = maximumTime - minimumTime;
        if (timeSpan < 60 * 1000) { // under 1 min or single point
            minimumTime -= 30 * 1000;
            maximumTime += 30 * 1000;
            axisX->setFormat(QStringLiteral("HH:mm:ss"));
            axisX->setTickCount(qMin(5, (int)rows.size() + 2));
        } else if (timeSpan < 3600 * 1000) { // under 1 hour
            axisX->setFormat(QStringLiteral("HH:mm:ss"));
            axisX->setTickCount(6);
        } else if (ui->periodCombo->currentData().toString() == QStringLiteral("day")) {
            axisX->setFormat(QStringLiteral("HH:mm"));
            axisX->setTickCount(6);
        } else {
            axisX->setFormat(QStringLiteral("dd/MM"));
            axisX->setTickCount(6);
        }
        axisX->setRange(QDateTime::fromMSecsSinceEpoch(minimumTime),
                        QDateTime::fromMSecsSinceEpoch(maximumTime));
    }

    if (activeKeys.size() == 1) {
        const QString k = activeKeys.first();
        m_chartTitle->setText(tr("Biểu đồ %1").arg(metricTitle(k)));
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

    if (plotableKeys.size() > 1) {
        metricCombo->addItem(tr("📊 Tất cả chỉ số"), QStringLiteral("all"));
        for (const QString &key : plotableKeys) {
            metricCombo->addItem(tr("📈 %1").arg(metricTitle(key)), key);
        }
    } else if (plotableKeys.size() == 1) {
        metricCombo->addItem(tr("📈 %1").arg(metricTitle(plotableKeys.first())), plotableKeys.first());
    }

    QString selectedKey = initialMetricKey.isEmpty() ? m_selectedMetricKey : initialMetricKey;
    if (selectedKey.isEmpty() || metricCombo->findData(selectedKey) < 0)
        selectedKey = plotableKeys.isEmpty() ? QStringLiteral("all") : plotableKeys.first();

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

        const QString activeMetric = metricCombo->currentData().toString();
        const QJsonArray &keys = m_cachedKeys;
        const QJsonArray &rows = m_cachedRows;

        QStringList activeKeys;
        if (activeMetric.isEmpty() || activeMetric == QStringLiteral("all")) {
            for (const QJsonValue &k : keys) {
                const QString keyStr = k.toString();
                if (keyStr != QStringLiteral("ir_detected"))
                    activeKeys.append(keyStr);
            }
            dialogTitle->setText(tr("Biểu đồ tổng quan các chỉ số"));
        } else {
            activeKeys.append(activeMetric);
            dialogTitle->setText(tr("Biểu đồ chi tiết: %1").arg(metricTitle(activeMetric)));
        }

        auto *axisX = new QDateTimeAxis(zoomChart);
        zoomChart->addAxis(axisX, Qt::AlignBottom);

        qint64 minimumTime = std::numeric_limits<qint64>::max();
        qint64 maximumTime = std::numeric_limits<qint64>::min();
        const QList<QColor> colors{QColor("#15945a"), QColor("#2d9cdb"),
                                   QColor("#e0a025"), QColor("#6750d8"),
                                   QColor("#d84d76"), QColor("#64748b")};

        double overallMin = std::numeric_limits<double>::max();
        double overallMax = std::numeric_limits<double>::lowest();
        double overallSum = 0;
        int overallCount = 0;

        int visibleSeries = 0;
        for (int i = 0; i < activeKeys.size(); ++i) {
            const QString key = activeKeys.at(i);
            auto *series = new QLineSeries(zoomChart);
            series->setName(metricTitle(key));
            series->setPointsVisible(true);
            series->setMarkerSize(8.0);
            QPen pen(colors.at(i % colors.size()));
            pen.setWidthF(3.0);
            series->setPen(pen);

            double minimum = std::numeric_limits<double>::max();
            double maximum = std::numeric_limits<double>::lowest();

            for (int r = rows.size() - 1; r >= 0; --r) {
                const QJsonObject entry = rows.at(r).toObject();
                const QJsonValue val = entry.value(QStringLiteral("metrics")).toObject().value(key);
                QString recordedAtStr = entry.value(QStringLiteral("recorded_at")).toString();
                QDateTime time = QDateTime::fromString(recordedAtStr, Qt::ISODateWithMs);
                if (!time.isValid())
                    time = QDateTime::fromString(recordedAtStr, Qt::ISODate);
                if (!time.isValid())
                    time = QDateTime::fromString(recordedAtStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
                time = time.toLocalTime();
                if (!val.isDouble() || !time.isValid())
                    continue;

                const qint64 ts = time.toMSecsSinceEpoch();
                const double num = val.toDouble();
                series->append(ts, num);
                minimumTime = qMin(minimumTime, ts);
                maximumTime = qMax(maximumTime, ts);
                minimum = qMin(minimum, num);
                maximum = qMax(maximum, num);
                overallMin = qMin(overallMin, num);
                overallMax = qMax(overallMax, num);
                overallSum += num;
                ++overallCount;
            }

            if (series->count() == 0) {
                delete series;
                continue;
            }

            zoomChart->addSeries(series);
            series->attachAxis(axisX);

            auto *axisY = new QValueAxis(zoomChart);
            axisY->setTitleText(compactMetricTitle(key));
            axisY->setLabelsColor(colors.at(i % colors.size()));
            axisY->setTitleBrush(colors.at(i % colors.size()));
            if (minimum > maximum) { minimum = 0; maximum = 10; }
            const double diff = maximum - minimum;
            const double padding = qMax(0.5, (diff == 0.0 ? (qAbs(maximum) > 0 ? qAbs(maximum) * 0.15 + 0.5 : 1.0) : diff * 0.15));
            axisY->setRange(minimum - padding, maximum + padding);
            axisY->setLabelFormat("%.2f");
            zoomChart->addAxis(axisY, visibleSeries == 0 ? Qt::AlignLeft : Qt::AlignRight);
            series->attachAxis(axisY);
            ++visibleSeries;
        }

        if (minimumTime <= maximumTime) {
            const qint64 timeSpan = maximumTime - minimumTime;
            if (timeSpan < 60 * 1000) {
                minimumTime -= 30 * 1000;
                maximumTime += 30 * 1000;
                axisX->setFormat(QStringLiteral("HH:mm:ss"));
                axisX->setTickCount(qMin(5, (int)rows.size() + 2));
            } else if (timeSpan < 3600 * 1000) {
                axisX->setFormat(QStringLiteral("HH:mm:ss"));
                axisX->setTickCount(6);
            } else if (ui->periodCombo->currentData().toString() == QStringLiteral("day")) {
                axisX->setFormat(QStringLiteral("HH:mm"));
                axisX->setTickCount(6);
            } else {
                axisX->setFormat(QStringLiteral("dd/MM"));
                axisX->setTickCount(6);
            }
            axisX->setRange(QDateTime::fromMSecsSinceEpoch(minimumTime),
                            QDateTime::fromMSecsSinceEpoch(maximumTime));
        }

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

