#include "DashboardPage.h"
#include "ui_DashboardPage.h"

#include <QChart>
#include <QChartView>
#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineSeries>
#include <QLocale>
#include <QPainter>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QValueAxis>
#include <QVBoxLayout>

namespace {
QLabel *textLabel(const QString &text, const char *objectName)
{
    auto *label = new QLabel(text);
    label->setObjectName(QString::fromLatin1(objectName));
    label->setWordWrap(true);
    return label;
}

QFrame *frame(const char *objectName)
{
    auto *card = new QFrame;
    card->setObjectName(QString::fromLatin1(objectName));
    card->setFrameShape(QFrame::NoFrame);
    return card;
}

QFrame *kpiCard(const QString &title, QLabel **value, const QString &icon, bool primary = false)
{
    auto *card = frame(primary ? "lenamKpiPrimary" : "lenamKpiCard");
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(4);
    auto *head = new QHBoxLayout;
    head->addWidget(textLabel(title, primary ? "lenamKpiTitleLight" : "lenamKpiTitle"));
    head->addStretch();
    head->addWidget(textLabel(icon, primary ? "lenamKpiIconLight" : "lenamKpiIcon"));
    layout->addLayout(head);
    *value = textLabel(QStringLiteral("--"), primary ? "lenamKpiValueLight" : "lenamKpiValue");
    layout->addWidget(*value);
    return card;
}

QChartView *lineChart(QLineSeries *series, const QString &color, double minY, double maxY)
{
    auto *chart = new QChart;
    chart->setBackgroundVisible(false);
    chart->legend()->hide();
    chart->setMargins(QMargins(2, 2, 2, 2));
    series->setPen(QPen(QColor(color), 3));
    chart->addSeries(series);

    auto *axisX = new QValueAxis(chart);
    axisX->setRange(0, 20);
    axisX->setTickCount(6);
    axisX->setGridLineColor(QColor("#e7edf3"));
    axisX->setLabelsColor(QColor("#7f8fa3"));
    auto *axisY = new QValueAxis(chart);
    axisY->setRange(minY, maxY);
    axisY->setTickCount(5);
    axisY->setGridLineColor(QColor("#e7edf3"));
    axisY->setLabelsColor(QColor("#7f8fa3"));
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    auto *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setObjectName(QStringLiteral("lenamChartView"));
    return view;
}

QPushButton *miniNav(const QString &icon, const QString &text)
{
    auto *button = new QPushButton(icon + QStringLiteral("   ") + text);
    button->setObjectName(QStringLiteral("lenamMiniNav"));
    button->setCursor(Qt::PointingHandCursor);
    return button;
}
}

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::DashboardPage),
      m_pressureSeries(new QLineSeries(this)),
      m_distanceSeries(new QLineSeries(this))
{
    ui->setupUi(this);
    setObjectName(QStringLiteral("LeNamDashboardPage"));
    ui->verticalLayout->setContentsMargins(0, 0, 0, 0);
    ui->verticalLayout->setSpacing(0);

    auto *shell = new QFrame(this);
    shell->setObjectName(QStringLiteral("lenamDashboardShell"));
    auto *shellLayout = new QHBoxLayout(shell);
    shellLayout->setContentsMargins(0, 0, 0, 0);
    shellLayout->setSpacing(0);
    ui->verticalLayout->addWidget(shell, 1);

    auto *content = new QFrame(shell);
    content->setObjectName(QStringLiteral("lenamDashboardContent"));
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(14, 8, 14, 10);
    contentLayout->setSpacing(8);
    shellLayout->addWidget(content, 1);

    auto *top = new QHBoxLayout;
    m_titleLabel = textLabel(tr("Dashboard User"), "lenamDashboardHeading");
    top->addWidget(m_titleLabel);
    top->addStretch();
    m_updatedAt = textLabel(QStringLiteral("☰"), "lenamMenuIcon");
    top->addWidget(m_updatedAt);
    contentLayout->addLayout(top);

    auto *grid = new QGridLayout;
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(8);
    contentLayout->addLayout(grid, 1);

    QLabel *earningValue = nullptr;
    grid->addWidget(kpiCard(tr("Temperature"), &earningValue, QStringLiteral("●"), true), 0, 0);
    m_temperatureChip = earningValue;
    grid->addWidget(kpiCard(tr("Sound"), &m_distanceChip, QStringLiteral("◆")), 0, 1);
    grid->addWidget(kpiCard(tr("Pressure"), &m_pressureChip, QStringLiteral("■")), 0, 2);
    grid->addWidget(kpiCard(tr("Rating"), &m_alertValue, QStringLiteral("★")), 0, 3);

    auto *resultCard = frame("lenamChartCard");
    auto *resultLayout = new QVBoxLayout(resultCard);
    resultLayout->setContentsMargins(20, 14, 20, 16);
    resultLayout->setSpacing(8);
    auto *resultHead = new QHBoxLayout;
    resultHead->addWidget(textLabel(tr("Result"), "lenamSectionTitle"));
    resultHead->addStretch();
    auto *check = new QPushButton(tr("Check Now"));
    check->setObjectName(QStringLiteral("lenamOrangeButton"));
    resultHead->addWidget(check);
    resultLayout->addLayout(resultHead);
    resultLayout->addWidget(lineChart(m_pressureSeries, QStringLiteral("#ffad18"), 985, 1035), 1);
    grid->addWidget(resultCard, 1, 0, 1, 3);

    auto *gauge = frame("lenamGaugeCard");
    auto *gaugeLayout = new QVBoxLayout(gauge);
    gaugeLayout->setContentsMargins(18, 18, 18, 18);
    gaugeLayout->setSpacing(10);
    m_pressureValue = textLabel(QStringLiteral("45%"), "lenamGaugeValue");
    m_pressureValue->setAlignment(Qt::AlignCenter);
    gaugeLayout->addWidget(m_pressureValue);
    gaugeLayout->addWidget(textLabel(tr("Realtime health"), "lenamGaugeText"));
    gaugeLayout->addWidget(textLabel(tr("Pressure"), "lenamGaugeLine"));
    gaugeLayout->addWidget(textLabel(tr("Temperature"), "lenamGaugeLine"));
    gaugeLayout->addWidget(textLabel(tr("Sound"), "lenamGaugeLine"));
    auto *check2 = new QPushButton(tr("Check Now"));
    check2->setObjectName(QStringLiteral("lenamOrangeButton"));
    gaugeLayout->addWidget(check2, 0, Qt::AlignCenter);
    grid->addWidget(gauge, 1, 3, 2, 1);

    auto *waveCard = frame("lenamWaveCard");
    auto *waveLayout = new QHBoxLayout(waveCard);
    waveLayout->setContentsMargins(20, 14, 20, 14);
    waveLayout->setSpacing(12);
    auto *legend = new QVBoxLayout;
    legend->addWidget(textLabel(QStringLiteral("●  Temperature"), "lenamLegendOrange"));
    legend->addWidget(textLabel(QStringLiteral("●  Distance"), "lenamLegendBlue"));
    legend->addStretch();
    waveLayout->addLayout(legend);
    waveLayout->addWidget(lineChart(m_distanceSeries, QStringLiteral("#194d75"), 0, 100), 1);
    grid->addWidget(waveCard, 2, 0, 1, 3);

    auto *tableCard = frame("lenamTableCard");
    auto *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(18, 14, 18, 14);
    tableLayout->setSpacing(10);
    tableLayout->addWidget(textLabel(tr("Latest telemetry"), "lenamSectionTitle"));
    m_history = new QTableWidget(0, 3);
    m_history->setObjectName(QStringLiteral("lenamHistoryTable"));
    m_history->setHorizontalHeaderLabels({tr("Time"), tr("Device"), tr("Metrics")});
    m_history->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_history->verticalHeader()->hide();
    m_history->setShowGrid(false);
    tableLayout->addWidget(m_history, 1);
    grid->addWidget(tableCard, 3, 0, 1, 4);

    m_temperatureChip->setText(QStringLiteral("--"));
    m_distanceChip->setText(QStringLiteral("--"));
    m_pressureChip->setText(QStringLiteral("--"));
    m_alertValue->setText(QStringLiteral("--"));
    m_pressureValue->setText(QStringLiteral("--"));
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

void DashboardPage::setUsername(const QString &username)
{
    const QString displayName = username.trimmed().isEmpty() ? tr("USER") : username.trimmed().toUpper();
    m_titleLabel->setText(tr("Dashboard %1").arg(displayName));
}

void DashboardPage::appendSeriesPoint(QLineSeries *series, double value,
                                      double fallbackMin, double fallbackMax)
{
    series->append(m_sampleIndex, value);
    while (series->count() > 24)
        series->remove(0);
    if (auto *axisX = qobject_cast<QValueAxis *>(series->chart()->axes(Qt::Horizontal).value(0)))
        axisX->setRange(qMax(0, m_sampleIndex - 23), qMax(23, m_sampleIndex));
    if (auto *axisY = qobject_cast<QValueAxis *>(series->chart()->axes(Qt::Vertical).value(0))) {
        Q_UNUSED(fallbackMin)
        Q_UNUSED(fallbackMax)
        axisY->setRange(axisY->min(), axisY->max());
    }
}


void DashboardPage::setDevices(const QJsonArray &devices)
{
    double temperature = std::numeric_limits<double>::quiet_NaN();
    double sound = std::numeric_limits<double>::quiet_NaN();
    double pressure = std::numeric_limits<double>::quiet_NaN();
    double distanceOrIr = std::numeric_limits<double>::quiet_NaN();
    bool hasIrDetected = false;
    int onlineCount = 0;

    for (const QJsonValue &value : devices) {
        const QJsonObject device = value.toObject();
        if (device.value(QStringLiteral("online")).toBool())
            ++onlineCount;
        const QJsonObject metrics = device.value(QStringLiteral("metrics")).toObject();
        if (metrics.contains(QStringLiteral("temperature_c")))
            temperature = metrics.value(QStringLiteral("temperature_c")).toDouble();
        if (metrics.contains(QStringLiteral("sound_vpp")))
            sound = metrics.value(QStringLiteral("sound_vpp")).toDouble();
        if (metrics.contains(QStringLiteral("pressure_hpa")))
            pressure = metrics.value(QStringLiteral("pressure_hpa")).toDouble();
        if (metrics.contains(QStringLiteral("distance_cm")))
            distanceOrIr = metrics.value(QStringLiteral("distance_cm")).toDouble();
        if (metrics.contains(QStringLiteral("ir_detected"))) {
            hasIrDetected = metrics.value(QStringLiteral("ir_detected")).toInt() != 0;
            distanceOrIr = hasIrDetected ? 1.0 : 0.0;
        }
        if (!metrics.isEmpty())
            addTelemetryRow(device, metrics);
    }

    m_temperatureChip->setText(std::isnan(temperature)
        ? QStringLiteral("--") : QStringLiteral("%1°C").arg(temperature, 0, 'f', 1));
    m_distanceChip->setText(std::isnan(sound)
        ? QStringLiteral("--") : QStringLiteral("%1 Vpp").arg(sound, 0, 'f', 3));
    m_pressureChip->setText(std::isnan(pressure)
        ? QStringLiteral("--") : QStringLiteral("%1 hPa").arg(pressure, 0, 'f', 0));
    m_alertValue->setText(QStringLiteral("%1/%2").arg(onlineCount).arg(devices.size()));

    int health = 0;
    int healthParts = 0;
    if (!std::isnan(temperature)) { health += temperature < 45.0 ? 35 : 15; healthParts += 35; }
    if (!std::isnan(pressure)) { health += (pressure >= 990.0 && pressure <= 1030.0) ? 35 : 15; healthParts += 35; }
    if (!std::isnan(sound)) { health += sound < 1.5 ? 30 : 12; healthParts += 30; }
    const int percent = healthParts == 0 ? 0 : qBound(0, health * 100 / healthParts, 100);
    m_pressureValue->setText(healthParts == 0 ? QStringLiteral("--") : QStringLiteral("%1%").arg(percent));
    m_updatedAt->setText(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm")));

    if (!std::isnan(pressure))
        appendSeriesPoint(m_pressureSeries, pressure, 985, 1035);
    if (!std::isnan(distanceOrIr))
        appendSeriesPoint(m_distanceSeries, distanceOrIr, 0, hasIrDetected ? 1 : 100);
    if (!std::isnan(pressure) || !std::isnan(distanceOrIr))
        ++m_sampleIndex;
}

void DashboardPage::updateReading(const SensorReading &reading)
{
    m_temperatureChip->setText(QStringLiteral("%1°C").arg(reading.temperatureC, 0, 'f', 1));
    m_distanceChip->setText(QString::number(reading.distanceCm, 'f', 0));
    m_pressureChip->setText(QString::number(reading.pressureHpa, 'f', 0));
    m_alertValue->setText(reading.distanceCm < 20.0 ? QStringLiteral("!") : QStringLiteral("8,5"));
    m_pressureValue->setText(QStringLiteral("%1%")
        .arg(qBound(0, int((reading.pressureHpa - 980.0) / 70.0 * 100.0), 100)));
    m_updatedAt->setText(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm")));
    appendSeriesPoint(m_pressureSeries, reading.pressureHpa, 985, 1035);
    appendSeriesPoint(m_distanceSeries, reading.distanceCm, 0, 100);
    ++m_sampleIndex;
    addHistory(reading);
}

void DashboardPage::addHistory(const SensorReading &reading)
{
    m_history->insertRow(0);
    m_history->setItem(0, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"))));
    m_history->setItem(0, 1, new QTableWidgetItem(QStringLiteral("%1 hPa").arg(reading.pressureHpa, 0, 'f', 1)));
    m_history->setItem(0, 2, new QTableWidgetItem(QStringLiteral("%1 cm").arg(reading.distanceCm, 0, 'f', 1)));
    while (m_history->rowCount() > 5)
        m_history->removeRow(m_history->rowCount() - 1);
}


void DashboardPage::addTelemetryRow(const QJsonObject &device, const QJsonObject &metrics)
{
    QStringList parts;
    if (metrics.contains(QStringLiteral("temperature_c")))
        parts << QStringLiteral("%1°C").arg(metrics.value(QStringLiteral("temperature_c")).toDouble(), 0, 'f', 1);
    if (metrics.contains(QStringLiteral("sound_vpp")))
        parts << QStringLiteral("%1 Vpp").arg(metrics.value(QStringLiteral("sound_vpp")).toDouble(), 0, 'f', 3);
    if (metrics.contains(QStringLiteral("pressure_hpa")))
        parts << QStringLiteral("%1 hPa").arg(metrics.value(QStringLiteral("pressure_hpa")).toDouble(), 0, 'f', 1);
    if (metrics.contains(QStringLiteral("uv_index")))
        parts << QStringLiteral("UV %1").arg(metrics.value(QStringLiteral("uv_index")).toDouble(), 0, 'f', 1);
    if (metrics.contains(QStringLiteral("ir_detected")))
        parts << (metrics.value(QStringLiteral("ir_detected")).toInt() ? tr("IR: Có vật") : tr("IR: Không có vật"));

    m_history->insertRow(0);
    m_history->setItem(0, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"))));
    m_history->setItem(0, 1, new QTableWidgetItem(device.value(QStringLiteral("name")).toString(
        device.value(QStringLiteral("device_id")).toString())));
    m_history->setItem(0, 2, new QTableWidgetItem(parts.join(QStringLiteral("  •  "))));
    while (m_history->rowCount() > 8)
        m_history->removeRow(m_history->rowCount() - 1);
}
