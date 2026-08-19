#include "DashboardPage.h"
#include "ui_DashboardPage.h"
#include "ui/widgets/DoorVisualizerWidget.h"

#include <QChart>
#include <QChartView>
#include <QDateTime>
#include <QValueAxis>

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::DashboardPage)
{
    ui->setupUi(this);

    m_doorVisualizer = new DoorVisualizerWidget(this);
    ui->doorVisLayout->addWidget(m_doorVisualizer);

    setupCharts();

    connect(ui->btnOpenDoor, &QPushButton::clicked, this, [this] {
        emit doorCommandRequested(QStringLiteral("open"));
    });
    connect(ui->btnCloseDoor, &QPushButton::clicked, this, [this] {
        emit doorCommandRequested(QStringLiteral("close"));
    });
    connect(ui->btnHoldOpen, &QPushButton::clicked, this, [this] {
        emit doorCommandRequested(QStringLiteral("hold_open"));
    });
    connect(ui->btnStopEmergency, &QPushButton::clicked, this, [this] {
        emit doorCommandRequested(QStringLiteral("stop"));
    });
    connect(ui->btnSimMotion, &QPushButton::clicked, this, [this] {
        emit simMotionRequested();
    });
    connect(ui->btnSimObstacle, &QPushButton::clicked, this, [this] {
        m_obstacleActive = !m_obstacleActive;
        ui->btnSimObstacle->setText(m_obstacleActive ? tr("🚧 Bỏ Chắn IR") : tr("🚧 Test Vật Cản IR"));
        emit simObstacleRequested(m_obstacleActive);
    });
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

void DashboardPage::setupCharts()
{
    m_positionSeries = new QLineSeries;
    m_positionSeries->setName(tr("Vị trí cửa (%)"));

    QPen pen(QColor(QStringLiteral("#06b6d4")), 2.5);
    m_positionSeries->setPen(pen);

    m_chart = new QChart;
    m_chart->addSeries(m_positionSeries);
    m_chart->legend()->hide();
    m_chart->setBackgroundVisible(false);
    m_chart->setMargins(QMargins(8, 0, 8, 0));

    auto *axisX = new QValueAxis(m_chart);
    axisX->setRange(0, 30);
    axisX->setLabelFormat(QStringLiteral("%d"));
    axisX->setGridLineColor(QColor(QStringLiteral("#1e293b")));
    axisX->setLabelsColor(QColor(QStringLiteral("#64748b")));

    auto *axisY = new QValueAxis(m_chart);
    axisY->setRange(0, 105);
    axisY->setTickCount(3);
    axisY->setLabelFormat(QStringLiteral("%d%%"));
    axisY->setGridLineColor(QColor(QStringLiteral("#1e293b")));
    axisY->setLabelsColor(QColor(QStringLiteral("#64748b")));

    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_positionSeries->attachAxis(axisX);
    m_positionSeries->attachAxis(axisY);

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
    // Update Door Visualizer
    m_doorVisualizer->setDoorPosition(reading.doorPositionPct);
    m_doorVisualizer->setMotionDetected(reading.motionDetected);
    m_doorVisualizer->setIrBlocked(reading.irBlocked);
    m_doorVisualizer->setDoorState(reading.doorState);
    m_doorVisualizer->setMotorSpeed(reading.motorSpeedRpm);

    // Update Card 1: SR602
    ui->sr602StatusLabel->setText(reading.motionDetected ? tr("🚶 Có người lại gần") : tr("Thông thoáng"));
    ui->sr602StatusLabel->setStyleSheet(reading.motionDetected
        ? QStringLiteral("color: #10b981; font-size: 14px; font-weight: 800;")
        : QStringLiteral("color: #94a3b8; font-size: 14px; font-weight: 800;"));
    ui->sr602TriggerCountLabel->setText(tr("Lượt kích hoạt: %1").arg(reading.pirTriggerCount));

    // Update Card 2: IR Safety
    if (reading.irBlocked) {
        ui->irStatusLabel->setText(tr("⚠️ BỊ CHE CHẮN!"));
        ui->irStatusLabel->setStyleSheet(QStringLiteral("color: #ef4444; font-size: 14px; font-weight: 800;"));
    } else {
        ui->irStatusLabel->setText(tr("Chùm tia an toàn"));
        ui->irStatusLabel->setStyleSheet(QStringLiteral("color: #10b981; font-size: 14px; font-weight: 800;"));
    }
    ui->irSubLabel->setText(reading.antiPinchActive ? tr("Đang kích hoạt chống kẹt") : tr("Tự đảo chiều: Bật"));

    // Update Card 3: Stepper Motor
    ui->motorStepsLabel->setText(tr("%1 / 3200 steps (%2%)")
        .arg(reading.currentStep)
        .arg(QString::number(reading.doorPositionPct, 'f', 0)));
    ui->motorSpeedLabel->setText(tr("Tốc độ: %1 RPM | %2")
        .arg(QString::number(reading.motorSpeedRpm, 'f', 0), reading.motorDirection));

    // Update Card 4: Traffic Stats
    ui->passageCountLabel->setText(tr("%1 lượt").arg(reading.passageCount));
    ui->trafficSubLabel->setText(tr("Nhiệt độ: %1 °C | %2 mA")
        .arg(QString::number(reading.temperatureC, 'f', 1))
        .arg(QString::number(reading.motorCurrentMa, 'f', 0)));

    // Update Chart data
    if (m_positionSeries) {
        m_positionSeries->append(m_sampleIndex++, reading.doorPositionPct);
        if (m_positionSeries->count() > 30) {
            m_positionSeries->remove(0);
            if (auto *axisX = qobject_cast<QValueAxis *>(m_chart->axes(Qt::Horizontal).value(0))) {
                axisX->setRange(m_sampleIndex - 30, m_sampleIndex);
            }
        }
    }
}
