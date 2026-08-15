#include "HistoryPage.h"

#include "ui_HistoryPage.h"

#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QChartView>
#include <QDate>
#include <QDateTimeAxis>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLegend>
#include <QLineSeries>
#include <QPainter>
#include <QResizeEvent>
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
    ui->titleLabel->setText(tr("Thống kê thiết bị"));
    ui->titleLabel->setObjectName(QStringLiteral("historyPageTitle"));
    ui->recordCountLabel->setObjectName(QStringLiteral("historyRecordBadge"));
    ui->deviceLabel->setObjectName(QStringLiteral("historyFilterLabel"));
    ui->periodLabel->setObjectName(QStringLiteral("historyFilterLabel"));
    ui->deviceCombo->setObjectName(QStringLiteral("historyDeviceCombo"));
    ui->periodCombo->setObjectName(QStringLiteral("historyPeriodCombo"));
    ui->dateEdit->setObjectName(QStringLiteral("historyDateEdit"));
    ui->searchButton->setObjectName(QStringLiteral("historySearchButton"));
    ui->filterLayout->setSpacing(10);

    m_headerSubtitle->setText(
        tr("Chọn thiết bị và khoảng thời gian để xem biểu đồ phù hợp với từng loại cảm biến."));
    m_headerSubtitle->setObjectName(QStringLiteral("historyPageSubtitle"));
    m_headerSubtitle->setWordWrap(true);
    ui->headerLayout->insertWidget(1, m_headerSubtitle, 1);

    ui->historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->historyTable->verticalHeader()->hide();
    ui->historyTable->setMinimumHeight(260);
    ui->historyTable->setObjectName(QStringLiteral("historyTableSmart"));
    ui->averagesLabel->hide();

    auto makeStatCard = [this](const QString &title, QLabel *value, const QString &icon) {
        auto *card = new QFrame(this);
        card->setObjectName(QStringLiteral("historyStatCard"));
        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(18, 16, 18, 16);
        layout->setSpacing(8);
        auto *iconLabel = new QLabel(icon, card);
        iconLabel->setObjectName(QStringLiteral("historyStatIcon"));
        iconLabel->setAlignment(Qt::AlignCenter);
        auto *titleLabel = new QLabel(title, card);
        titleLabel->setObjectName(QStringLiteral("historyStatTitle"));
        value->setObjectName(QStringLiteral("historyStatValue"));
        value->setText(QStringLiteral("--"));
        value->setWordWrap(true);
        layout->addWidget(iconLabel);
        layout->addWidget(titleLabel);
        layout->addStretch();
        layout->addWidget(value);
        return card;
    };

    m_chartView->setObjectName(QStringLiteral("historyChart"));
    m_chartView->setMinimumHeight(330);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chart->setTitle(tr("Thống kê dữ liệu cảm biến"));
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    m_chartCard->setObjectName(QStringLiteral("historyChartCard"));
    auto *chartLayout = new QVBoxLayout(m_chartCard);
    chartLayout->setContentsMargins(18, 16, 18, 14);
    chartLayout->setSpacing(8);
    auto *chartTitle = new QLabel(tr("Thống kê dữ liệu cảm biến"), m_chartCard);
    chartTitle->setObjectName(QStringLiteral("historyCardTitle"));
    m_chartHint->setObjectName(QStringLiteral("historyChartHint"));
    m_chartHint->setText(tr("Chọn thiết bị để xem biểu đồ phù hợp."));
    m_chartHint->setWordWrap(true);
    chartLayout->addWidget(chartTitle);
    chartLayout->addWidget(m_chartHint);
    chartLayout->addWidget(m_chartView, 1);

    m_analyticsGrid->setContentsMargins(0, 0, 0, 0);
    m_analyticsGrid->setHorizontalSpacing(14);
    m_analyticsGrid->setVerticalSpacing(14);
    m_primaryStatCard = makeStatCard(tr("Chỉ số chính"), m_primaryStat, QStringLiteral("↯"));
    m_secondaryStatCard = makeStatCard(tr("Chỉ số phụ"), m_secondaryStat, QStringLiteral("◍"));
    m_summaryStatCard = makeStatCard(tr("Tóm tắt"), m_thirdStat, QStringLiteral("▥"));
    ui->verticalLayout->insertLayout(3, m_analyticsGrid, 4);
    applyResponsiveLayout();

    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit->setMinimumWidth(142);
    ui->dateEdit->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    ui->periodCombo->setMinimumWidth(110);
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
}

void HistoryPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    applyResponsiveLayout();
}

void HistoryPage::applyResponsiveLayout()
{
    if (!m_analyticsGrid || !m_chartCard || !m_primaryStatCard || !m_secondaryStatCard || !m_summaryStatCard)
        return;

    while (QLayoutItem *item = m_analyticsGrid->takeAt(0))
        delete item;

    for (int i = 0; i < 4; ++i) {
        m_analyticsGrid->setColumnStretch(i, 0);
        m_analyticsGrid->setRowStretch(i, 0);
    }

    const int pageWidth = width();
    const bool compact = pageWidth < 760;
    const bool medium = pageWidth >= 760 && pageWidth < 1050;

    m_headerSubtitle->setVisible(pageWidth >= 840);
    ui->deviceLabel->setVisible(pageWidth >= 680);
    ui->periodLabel->setVisible(pageWidth >= 680);
    ui->deviceCombo->setMinimumWidth(compact ? 150 : (medium ? 210 : 260));
    ui->periodCombo->setMinimumWidth(compact ? 92 : 110);
    ui->dateEdit->setMinimumWidth(compact ? 118 : 142);
    ui->searchButton->setMinimumWidth(compact ? 92 : 110);
    m_chartView->setMinimumHeight(compact ? 220 : (medium ? 260 : 330));
    ui->historyTable->setMinimumHeight(compact ? 210 : 260);

    if (compact) {
        m_analyticsGrid->addWidget(m_chartCard, 0, 0);
        m_analyticsGrid->addWidget(m_primaryStatCard, 1, 0);
        m_analyticsGrid->addWidget(m_secondaryStatCard, 2, 0);
        m_analyticsGrid->addWidget(m_summaryStatCard, 3, 0);
        m_analyticsGrid->setColumnStretch(0, 1);
        return;
    }

    if (medium) {
        m_analyticsGrid->addWidget(m_chartCard, 0, 0, 1, 2);
        m_analyticsGrid->addWidget(m_primaryStatCard, 1, 0);
        m_analyticsGrid->addWidget(m_secondaryStatCard, 1, 1);
        m_analyticsGrid->addWidget(m_summaryStatCard, 2, 0, 1, 2);
        m_analyticsGrid->setColumnStretch(0, 1);
        m_analyticsGrid->setColumnStretch(1, 1);
        return;
    }

    m_analyticsGrid->addWidget(m_chartCard, 0, 0, 2, 2);
    m_analyticsGrid->addWidget(m_primaryStatCard, 0, 2);
    m_analyticsGrid->addWidget(m_secondaryStatCard, 0, 3);
    m_analyticsGrid->addWidget(m_summaryStatCard, 1, 2, 1, 2);
    m_analyticsGrid->setColumnStretch(0, 2);
    m_analyticsGrid->setColumnStretch(1, 2);
    m_analyticsGrid->setColumnStretch(2, 1);
    m_analyticsGrid->setColumnStretch(3, 1);
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
        ui->deviceCombo->addItem(QStringLiteral("%1  ·  %2").arg(name, id), id);
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
        updateChart({}, {});
        return;
    }
    emit historyRequested(deviceId, ui->periodCombo->currentData().toString(),
                          ui->dateEdit->date().toString(Qt::ISODate));
}

void HistoryPage::setHistory(const QJsonObject &history)
{
    const QJsonArray keys = history.value(QStringLiteral("metric_keys")).toArray();
    const QJsonArray rows = history.value(QStringLiteral("data")).toArray();
    ui->historyTable->clear();
    ui->historyTable->setRowCount(rows.size());
    ui->historyTable->setColumnCount(keys.size() + 1);
    QStringList headers{tr("Thời gian")};
    for (const QJsonValue &key : keys)
        headers.append(metricTitle(key.toString()));
    ui->historyTable->setHorizontalHeaderLabels(headers);
    for (int row = 0; row < rows.size(); ++row) {
        const QJsonObject entry = rows.at(row).toObject();
        const QDateTime time = QDateTime::fromString(
            entry.value(QStringLiteral("recorded_at")).toString(), Qt::ISODateWithMs).toLocalTime();
        ui->historyTable->setItem(row, 0, new QTableWidgetItem(
            time.toString(QStringLiteral("dd/MM/yyyy HH:mm:ss"))));
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
        ? summary.mid(2).join(QStringLiteral("\n"))
        : tr("%1 bản ghi trong khoảng đã chọn").arg(total));
    updateChart(keys, rows);
}

void HistoryPage::updateChart(const QJsonArray &keys, const QJsonArray &rows)
{
    m_chart->removeAllSeries();
    const QList<QAbstractAxis *> oldAxes = m_chart->axes();
    for (QAbstractAxis *axis : oldAxes) {
        m_chart->removeAxis(axis);
        axis->deleteLater();
    }
    if (keys.isEmpty() || rows.isEmpty()) {
        m_chart->setTitle(tr("Không có dữ liệu"));
        m_chartHint->setText(tr("Không có dữ liệu trong khoảng thời gian đã chọn."));
        return;
    }

    const bool isIrOnly = keys.size() == 1 && keys.at(0).toString() == QStringLiteral("ir_detected");
    if (isIrOnly) {
        auto *set = new QBarSet(tr("Có vật"));
        QStringList categories;
        int used = 0;
        for (int row = rows.size() - 1; row >= 0 && used < 12; --row, ++used) {
            const QJsonObject entry = rows.at(row).toObject();
            const QDateTime time = QDateTime::fromString(
                entry.value(QStringLiteral("recorded_at")).toString(), Qt::ISODateWithMs).toLocalTime();
            categories << time.toString(QStringLiteral("HH:mm"));
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
        m_chart->setTitle(tr("Biểu đồ trạng thái IR"));
        m_chartHint->setText(tr("IR dùng biểu đồ cột để thể hiện trạng thái phát hiện vật theo thời gian."));
        return;
    }

    auto *axisX = new QDateTimeAxis(m_chart);
    axisX->setFormat(ui->periodCombo->currentData().toString() == QStringLiteral("day")
                         ? QStringLiteral("HH:mm") : QStringLiteral("dd/MM"));
    axisX->setTickCount(7);
    m_chart->addAxis(axisX, Qt::AlignBottom);

    qint64 minimumTime = std::numeric_limits<qint64>::max();
    qint64 maximumTime = std::numeric_limits<qint64>::min();
    const QList<QColor> colors{QColor("#2d9cdb"), QColor("#21a67a"),
                               QColor("#e0a025"), QColor("#6750d8"),
                               QColor("#d84d76"), QColor("#64748b")};
    int visibleSeries = 0;
    for (int metricIndex = 0; metricIndex < keys.size(); ++metricIndex) {
        const QString key = keys.at(metricIndex).toString();
        if (key == QStringLiteral("ir_detected"))
            continue;
        auto *series = new QLineSeries(m_chart);
        series->setName(metricTitle(key));
        QPen pen(colors.at(visibleSeries % colors.size()));
        pen.setWidthF(2.8);
        series->setPen(pen);
        double minimum = std::numeric_limits<double>::max();
        double maximum = std::numeric_limits<double>::lowest();
        for (int row = rows.size() - 1; row >= 0; --row) {
            const QJsonObject entry = rows.at(row).toObject();
            const QJsonValue value = entry.value(QStringLiteral("metrics")).toObject().value(key);
            const QDateTime time = QDateTime::fromString(
                entry.value(QStringLiteral("recorded_at")).toString(), Qt::ISODateWithMs);
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
        axisY->setLabelsColor(colors.at(visibleSeries % colors.size()));
        axisY->setTitleBrush(colors.at(visibleSeries % colors.size()));
        const double padding = qMax(0.1, (maximum - minimum) * 0.16);
        axisY->setRange(minimum - padding, maximum + padding);
        axisY->setLabelFormat("%.1f");
        m_chart->addAxis(axisY, visibleSeries % 2 == 0 ? Qt::AlignLeft : Qt::AlignRight);
        series->attachAxis(axisY);
        ++visibleSeries;
    }
    if (minimumTime <= maximumTime) {
        if (minimumTime == maximumTime) {
            minimumTime -= 30000;
            maximumTime += 30000;
        }
        axisX->setRange(QDateTime::fromMSecsSinceEpoch(minimumTime),
                        QDateTime::fromMSecsSinceEpoch(maximumTime));
    }
    m_chart->setTitle(tr("Thống kê · %1").arg(ui->deviceCombo->currentText()));
    const QString type = currentDeviceType();
    if (type == QStringLiteral("temperature_sound"))
        m_chartHint->setText(tr("Biểu đồ đường 2 chỉ số: nhiệt độ và âm thanh."));
    else if (type == QStringLiteral("weather_pressure"))
        m_chartHint->setText(tr("Biểu đồ đường: nhiệt độ kết hợp áp suất khí quyển."));
    else if (type == QStringLiteral("uv_pressure"))
        m_chartHint->setText(tr("Biểu đồ đường: UV index kết hợp áp suất."));
    else
        m_chartHint->setText(tr("Biểu đồ được chọn theo loại dữ liệu cảm biến."));
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
        {"distance_cm", tr("Khoảng cách (cm)")}, {"ir_detected", tr("IR")}};
    return names.value(key, key);
}

QString HistoryPage::compactMetricTitle(const QString &key)
{
    static const QHash<QString, QString> names{
        {"temperature_c", tr("°C")}, {"humidity_percent", tr("%")},
        {"pressure_hpa", tr("hPa")}, {"uv_index", tr("UV")},
        {"uv_voltage", tr("V")}, {"sound_vpp", tr("Vpp")},
        {"distance_cm", tr("cm")}, {"ir_detected", tr("IR")}};
    return names.value(key, key);
}

QString HistoryPage::metricUnit(const QString &key)
{
    return compactMetricTitle(key);
}
