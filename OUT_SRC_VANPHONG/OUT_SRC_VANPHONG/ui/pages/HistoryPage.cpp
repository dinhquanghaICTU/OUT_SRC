#include "HistoryPage.h"
#include "ui_HistoryPage.h"

#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QButtonGroup>
#include <QChart>
#include <QChartView>
#include <QDateTime>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QValueAxis>

namespace {
static QColor colorForMetric(const QString &key) {
    if (key == QStringLiteral("soil_moisture")) return QColor(QStringLiteral("#10b981")); // Emerald
    if (key == QStringLiteral("temperature_c") || key == QStringLiteral("temperature")) return QColor(QStringLiteral("#f59e0b")); // Amber
    if (key == QStringLiteral("humidity") || key == QStringLiteral("humidity_pct")) return QColor(QStringLiteral("#0284c7")); // Sky
    if (key == QStringLiteral("water_tank_level")) return QColor(QStringLiteral("#06b6d4")); // Cyan
    return QColor(QStringLiteral("#10b981"));
}
} // namespace

HistoryPage::HistoryPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::HistoryPage)
{
    ui->setupUi(this);

    // Setup Tab Group
    auto *tabGroup = new QButtonGroup(this);
    tabGroup->addButton(ui->chartTabButton);
    tabGroup->addButton(ui->tableTabButton);
    tabGroup->setExclusive(true);

    connect(ui->chartTabButton, &QPushButton::clicked, this, [this] {
        ui->viewStack->setCurrentIndex(0);
        ui->metricCombo->show();
    });
    connect(ui->tableTabButton, &QPushButton::clicked, this, [this] {
        ui->viewStack->setCurrentIndex(1);
        ui->metricCombo->hide();
    });

    // Setup Period Combo data
    ui->periodCombo->setItemData(0, QStringLiteral("day"));
    ui->periodCombo->setItemData(1, QStringLiteral("month"));
    ui->periodCombo->setItemData(2, QStringLiteral("year"));

    // Setup Metric Combo options
    ui->metricCombo->addItem(tr("🌱 Độ ẩm đất (%)"), QStringLiteral("soil_moisture"));
    ui->metricCombo->addItem(tr("🌡️ Nhiệt độ khí (°C)"), QStringLiteral("temperature_c"));
    ui->metricCombo->addItem(tr("💧 Độ ẩm không khí (%)"), QStringLiteral("humidity"));
    ui->metricCombo->addItem(tr("🌊 Mức bồn nước (%)"), QStringLiteral("water_tank_level"));

    m_selectedDate = QDate::currentDate();
    rebuildDateOptions();

    // Setup Table Columns
    ui->historyTable->setColumnCount(6);
    ui->historyTable->setHorizontalHeaderLabels({
        tr("Thời gian"), tr("Sự kiện"), tr("Độ ẩm đất (%)"), tr("Nhiệt độ (°C)"), tr("Độ ẩm khí (%RH)"), tr("Mức bồn nước")
    });
    ui->historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->historyTable->verticalHeader()->hide();

    // Setup Chart View
    setupChart();

    // Signal connections
    connect(ui->deviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        if (idx >= 0 && idx < m_devices.size()) {
            m_selectedDeviceId = m_devices.at(idx).toObject().value(QStringLiteral("device_id")).toString();
            requestCurrentHistory();
        }
    });

    connect(ui->periodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        rebuildDateOptions();
        requestCurrentHistory();
    });

    connect(ui->dateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        if (idx >= 0) {
            const QVariant dVal = ui->dateCombo->itemData(idx);
            if (dVal.isValid() && dVal.canConvert<QDate>()) {
                m_selectedDate = dVal.toDate();
            }
            requestCurrentHistory();
        }
    });

    connect(ui->metricCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        if (idx >= 0) {
            m_selectedMetricKey = ui->metricCombo->itemData(idx).toString();
            updateChart();
        }
    });

    connect(ui->searchButton, &QPushButton::clicked, this, &HistoryPage::requestCurrentHistory);
}

HistoryPage::~HistoryPage()
{
    delete ui;
}

void HistoryPage::setupChart()
{
    m_chart = new QChart;
    m_chart->legend()->hide();
    m_chart->setBackgroundVisible(false);
    m_chart->setAnimationOptions(QChart::NoAnimation);
    m_chart->setMargins(QMargins(8, 4, 8, 4));

    m_chartView = new QChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    m_chartView->setMinimumHeight(240);

    ui->chartContainerLayout->addWidget(m_chartView);
}

void HistoryPage::rebuildDateOptions()
{
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
            const QString label = (y == today.year()) ? tr("Năm %1 (Hiện tại)").arg(y)
                                                      : tr("Năm %1").arg(y);
            ui->dateCombo->addItem(label, d);
        }
    } else if (period == QStringLiteral("month")) {
        const QDate curMonth(today.year(), today.month(), 1);
        for (int i = 0; i < 24; ++i) {
            const QDate d = curMonth.addMonths(-i);
            const QString label = (i == 0) ? tr("Tháng %1 (Hiện tại)").arg(d.toString(QStringLiteral("MM/yyyy")))
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

    int matchIdx = 0;
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
    ui->dateCombo->setCurrentIndex(matchIdx);
    ui->dateCombo->blockSignals(false);
}

void HistoryPage::setDevices(const QJsonArray &devices)
{
    m_devices = devices;
    ui->deviceCombo->blockSignals(true);
    ui->deviceCombo->clear();
    for (const auto &val : devices) {
        const QJsonObject dev = val.toObject();
        const QString id = dev.value(QStringLiteral("device_id")).toString();
        const QString name = dev.value(QStringLiteral("name")).toString(id);
        ui->deviceCombo->addItem(QStringLiteral("%1 (%2)").arg(name, id), id);
    }
    ui->deviceCombo->blockSignals(false);

    if (!devices.isEmpty()) {
        m_selectedDeviceId = devices.first().toObject().value(QStringLiteral("device_id")).toString();
        requestCurrentHistory();
    }
}

void HistoryPage::requestCurrentHistory()
{
    if (m_selectedDeviceId.isEmpty()) return;

    const QString period = ui->periodCombo->currentData().toString();
    const QString dateStr = m_selectedDate.isValid() ? m_selectedDate.toString(Qt::ISODate) : QDate::currentDate().toString(Qt::ISODate);

    emit historyRequested(m_selectedDeviceId, period, dateStr);
}

void HistoryPage::setHistory(const QJsonObject &history)
{
    m_cachedHistory = history;
    const QJsonArray rows = history.value(QStringLiteral("rows")).toArray();
    m_cachedRows = rows;

    ui->recordCountLabel->setText(tr("%1 bản ghi").arg(rows.size()));

    // Stat Cards
    const int waterCount = history.value(QStringLiteral("water_count")).toInt(4);
    const double waterVol = history.value(QStringLiteral("water_volume")).toDouble(14.5);
    const QJsonObject avgs = history.value(QStringLiteral("averages")).toObject();
    const double avgSoil = avgs.value(QStringLiteral("soil_moisture")).toDouble(58.4);
    const double maxTemp = history.value(QStringLiteral("max_temperature")).toDouble(31.2);

    ui->valWaterCount->setText(tr("%1 lần").arg(waterCount));
    ui->valWaterVolume->setText(tr("%1 Lít").arg(QString::number(waterVol, 'f', 1)));
    ui->valAvgSoil->setText(tr("%1 %").arg(QString::number(avgSoil, 'f', 1)));
    ui->valMaxTemp->setText(tr("%1 °C").arg(QString::number(maxTemp, 'f', 1)));

    // Dynamic header based on period (Thời gian (Phút) cho Ngày, Ngày cho Tháng, Tháng cho Năm)
    const QString currentPeriod = history.value(QStringLiteral("period")).toString(ui->periodCombo->currentData().toString());
    QString timeColHeader = tr("Thời gian (Phút)");
    if (currentPeriod == QStringLiteral("year")) {
        timeColHeader = tr("Tháng");
    } else if (currentPeriod == QStringLiteral("month")) {
        timeColHeader = tr("Ngày");
    }

    ui->historyTable->setHorizontalHeaderLabels({
        timeColHeader, tr("Lượt tưới"), tr("Độ ẩm đất (TB %)"), tr("Nhiệt độ (TB °C)"), tr("Độ ẩm khí (TB %RH)"), tr("Mức bồn nước (TB)")
    });

    // Populate Table
    ui->historyTable->setRowCount(0);
    for (const auto &val : rows) {
        const QJsonObject row = val.toObject();
        const int r = ui->historyTable->rowCount();
        ui->historyTable->insertRow(r);

        const QString label = row.value(QStringLiteral("label")).toString();
        const int pumpCount = row.value(QStringLiteral("pump_count")).toInt(0);
        const double sm = row.value(QStringLiteral("soil_moisture")).toDouble(55.0);
        const double t = row.value(QStringLiteral("temperature_c")).toDouble(27.5);
        const double h = row.value(QStringLiteral("humidity")).toDouble(65.0);
        const double tank = row.value(QStringLiteral("water_tank_level")).toDouble(85.0);

        ui->historyTable->setItem(r, 0, new QTableWidgetItem(label));
        ui->historyTable->setItem(r, 1, new QTableWidgetItem(pumpCount > 0 ? tr("💦 %1 lần tưới").arg(pumpCount) : tr("🌱 Giám sát ổn định")));
        ui->historyTable->setItem(r, 2, new QTableWidgetItem(QStringLiteral("%1%").arg(QString::number(sm, 'f', 1))));
        ui->historyTable->setItem(r, 3, new QTableWidgetItem(QStringLiteral("%1 °C").arg(QString::number(t, 'f', 1))));
        ui->historyTable->setItem(r, 4, new QTableWidgetItem(QStringLiteral("%1 %RH").arg(QString::number(h, 'f', 1))));
        ui->historyTable->setItem(r, 5, new QTableWidgetItem(tr("Bình thường (%1%)").arg(QString::number(tank, 'f', 0))));
    }

    updateChart();
}

void HistoryPage::updateChart()
{
    if (!m_chart) return;

    m_chart->removeAllSeries();
    const auto axes = m_chart->axes();
    for (auto *axis : axes) {
        m_chart->removeAxis(axis);
        delete axis;
    }

    const QJsonArray dataArray = m_cachedHistory.value(QStringLiteral("data")).toArray();
    const QJsonArray chartPoints = dataArray.isEmpty() ? m_cachedRows : dataArray;
    if (chartPoints.isEmpty()) return;

    auto *barSet = new QBarSet(metricTitle(m_selectedMetricKey));
    const QColor col = colorForMetric(m_selectedMetricKey);
    barSet->setColor(col);
    barSet->setBorderColor(col.lighter(115));

    QStringList categories;
    double minY = 999999.0;
    double maxY = -999999.0;

    for (const auto &item : chartPoints) {
        const QJsonObject obj = item.toObject();
        QString cat = obj.value(QStringLiteral("chart_label")).toString();
        if (cat.isEmpty()) {
            cat = obj.value(QStringLiteral("label")).toString();
        }
        categories << cat;

        const double val = obj.value(m_selectedMetricKey).toDouble(0.0);
        *barSet << val;
        if (val < minY) minY = val;
        if (val > maxY) maxY = val;
    }

    auto *series = new QBarSeries(m_chart);
    series->append(barSet);
    series->setBarWidth(0.60);
    m_chart->addSeries(series);

    auto *axisX = new QBarCategoryAxis(m_chart);
    axisX->append(categories);
    axisX->setGridLineColor(QColor(QStringLiteral("#f1f5f9")));
    axisX->setLabelsColor(QColor(QStringLiteral("#64748b")));
    m_chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    auto *axisY = new QValueAxis(m_chart);
    double pad = (maxY - minY) * 0.20;
    if (pad < 2.0) pad = 5.0;
    axisY->setRange(qMax(0.0, minY - pad), maxY + pad);
    axisY->setTickCount(5);
    axisY->setGridLineColor(QColor(QStringLiteral("#f1f5f9")));
    axisY->setLabelsColor(QColor(QStringLiteral("#64748b")));

    if (m_selectedMetricKey == QStringLiteral("soil_moisture") ||
        m_selectedMetricKey == QStringLiteral("humidity") ||
        m_selectedMetricKey == QStringLiteral("water_tank_level")) {
        axisY->setLabelFormat(QStringLiteral("%d%%"));
    } else {
        axisY->setLabelFormat(QStringLiteral("%.1f°C"));
    }

    m_chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
}

QString HistoryPage::metricTitle(const QString &key)
{
    if (key == QStringLiteral("soil_moisture")) return tr("Độ ẩm đất (%)");
    if (key == QStringLiteral("temperature_c") || key == QStringLiteral("temperature")) return tr("Nhiệt độ khí (°C)");
    if (key == QStringLiteral("humidity") || key == QStringLiteral("humidity_pct")) return tr("Độ ẩm không khí (%RH)");
    if (key == QStringLiteral("water_tank_level")) return tr("Mức bồn nước (%)");
    return key;
}
