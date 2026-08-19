#include "DashboardPage.h"
#include "ui_DashboardPage.h"
#include "ui/widgets/PlantSoilVisualizerWidget.h"

#include <QChart>
#include <QChartView>
#include <QDateTime>
#include <QValueAxis>

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::DashboardPage)
{
    ui->setupUi(this);

    m_plantVisualizer = new PlantSoilVisualizerWidget(this);
    ui->plantCanvasContainer->addWidget(m_plantVisualizer);

    setupCharts();

    connect(ui->btnTogglePumpMaster, &QPushButton::clicked, this, [this] {
        const bool active = ui->btnTogglePumpMaster->isChecked();
        emit pumpCommandRequested(active);
    });

    connect(ui->btnTimer30s, &QPushButton::clicked, this, [this] {
        emit pumpCommandRequested(true);
    });
    connect(ui->btnTimer1m, &QPushButton::clicked, this, [this] {
        emit pumpCommandRequested(true);
    });
    connect(ui->btnTimer3m, &QPushButton::clicked, this, [this] {
        emit pumpCommandRequested(true);
    });

    connect(ui->btnAutoModeCapsule, &QPushButton::toggled, this, [this](bool checked) {
        ui->btnAutoModeCapsule->setText(checked ? tr("⚡ TỰ ĐỘNG THEO ĐỘ ẨM: BẬT") : tr("🖐 CHẾ ĐỘ THỦ CÔNG"));
    });

    connect(ui->btnSimDrySoil, &QPushButton::clicked, this, [this] {
        emit simDrySoilRequested();
    });
    connect(ui->btnSimMoistSoil, &QPushButton::clicked, this, [this] {
        emit simMoistSoilRequested();
    });
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

void DashboardPage::setupCharts()
{
    m_soilSeries = new QLineSeries;
    m_soilSeries->setName(tr("Độ ẩm đất (%)"));
    m_soilSeries->setPen(QPen(QColor(QStringLiteral("#10b981")), 2.5));

    m_humiditySeries = new QLineSeries;
    m_humiditySeries->setName(tr("Độ ẩm không khí (%)"));
    m_humiditySeries->setPen(QPen(QColor(QStringLiteral("#38bdf8")), 1.8, Qt::DashLine));

    m_chart = new QChart;
    m_chart->addSeries(m_soilSeries);
    m_chart->addSeries(m_humiditySeries);
    m_chart->legend()->hide();
    m_chart->setBackgroundVisible(false);
    m_chart->setMargins(QMargins(4, 0, 4, 0));

    auto *axisX = new QValueAxis(m_chart);
    axisX->setRange(0, 30);
    axisX->setLabelFormat(QStringLiteral("%d"));
    axisX->setGridLineColor(QColor(QStringLiteral("#1b4332")));
    axisX->setLabelsColor(QColor(QStringLiteral("#6ee7b7")));

    auto *axisY = new QValueAxis(m_chart);
    axisY->setRange(0, 105);
    axisY->setTickCount(4);
    axisY->setLabelFormat(QStringLiteral("%d%%"));
    axisY->setGridLineColor(QColor(QStringLiteral("#1b4332")));
    axisY->setLabelsColor(QColor(QStringLiteral("#6ee7b7")));

    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_soilSeries->attachAxis(axisX);
    m_soilSeries->attachAxis(axisY);
    m_humiditySeries->attachAxis(axisX);
    m_humiditySeries->attachAxis(axisY);

    m_chartView = new QChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    ui->chartContainerLayout->addWidget(m_chartView);
}

void DashboardPage::setUsername(const QString &username)
{
    Q_UNUSED(username);
}

void DashboardPage::updateReading(const SensorReading &reading)
{
    // Update Canvas Visualizer
    m_plantVisualizer->setSoilMoisture(reading.soilMoisturePct);
    m_plantVisualizer->setTemperature(reading.temperatureC);
    m_plantVisualizer->setHumidity(reading.humidityPct);
    m_plantVisualizer->setPumpActive(reading.pumpActive);

    // Update Master Pump Button state
    ui->btnTogglePumpMaster->setChecked(reading.pumpActive);
    ui->btnTogglePumpMaster->setText(reading.pumpActive ? tr("🛑 TẮT BƠM TƯỚI") : tr("💦 BẬT BƠM TƯỚI"));

    // Metric 1: Soil Moisture
    ui->soilMoistureValueLabel->setText(QStringLiteral("%1%").arg(QString::number(reading.soilMoisturePct, 'f', 1)));
    if (reading.soilMoisturePct < 40.0) {
        ui->soilMoistureSubLabel->setText(tr("🍂 ĐẤT KHÔ HẠN"));
        ui->soilMoistureSubLabel->setStyleSheet(QStringLiteral("color: #f59e0b; font-size: 9px; font-weight: 700;"));
    } else if (reading.soilMoisturePct > 80.0) {
        ui->soilMoistureSubLabel->setText(tr("🌊 ĐẤT QUÁ ẨM"));
        ui->soilMoistureSubLabel->setStyleSheet(QStringLiteral("color: #06b6d4; font-size: 9px; font-weight: 700;"));
    } else {
        ui->soilMoistureSubLabel->setText(tr("🌱 ĐỘ ẨM LÝ TƯỞNG"));
        ui->soilMoistureSubLabel->setStyleSheet(QStringLiteral("color: #34d399; font-size: 9px; font-weight: 700;"));
    }

    // Metric 2: DHT11 Temp
    ui->tempValueLabel->setText(QStringLiteral("%1 °C").arg(QString::number(reading.temperatureC, 'f', 1)));
    ui->tempSubLabel->setText(reading.temperatureC > 35.0 ? tr("⚠️ NÓNG BỨC") : tr("Mát mẻ"));

    // Metric 3: DHT11 Hum
    ui->humValueLabel->setText(QStringLiteral("%1 %RH").arg(QString::number(reading.humidityPct, 'f', 1)));
    ui->humSubLabel->setText(tr("Tán lá: %1").arg(reading.humidityPct > 70.0 ? tr("Cao") : tr("Tốt")));

    // Tank & Pump
    ui->tankLevelLabel->setText(QStringLiteral("🚰 Bồn nước: %1%").arg(QString::number(reading.waterTankLevelPct, 'f', 0)));
    ui->tankSubLabel->setText(tr("Đã tưới: %1 lần (%2 L)")
        .arg(reading.totalWateringCountToday)
        .arg(QString::number(reading.totalWaterUsedLiters, 'f', 1)));

    // Update Chart
    if (m_soilSeries && m_humiditySeries) {
        m_soilSeries->append(m_sampleIndex, reading.soilMoisturePct);
        m_humiditySeries->append(m_sampleIndex, reading.humidityPct);
        m_sampleIndex++;

        if (m_soilSeries->count() > 30) {
            m_soilSeries->remove(0);
            m_humiditySeries->remove(0);
            if (auto *axisX = qobject_cast<QValueAxis *>(m_chart->axes(Qt::Horizontal).value(0))) {
                axisX->setRange(m_sampleIndex - 30, m_sampleIndex);
            }
        }
    }
}
