#include "HistoryPage.h"
#include "ui_HistoryPage.h"

#include <QDateTime>
#include <QHeaderView>

HistoryPage::HistoryPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::HistoryPage)
{
    ui->setupUi(this);

    ui->historyTable->setColumnCount(6);
    ui->historyTable->setHorizontalHeaderLabels({
        tr("Thời gian"), tr("Sự kiện"), tr("Độ ẩm đất (%)"), tr("Nhiệt độ (°C)"), tr("Độ ẩm khí (%RH)"), tr("Trạng thái bồn nước")
    });
    ui->historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->historyTable->verticalHeader()->hide();

    connect(ui->deviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        if (idx >= 0 && idx < m_devices.size()) {
            m_selectedDeviceId = m_devices.at(idx).toObject().value(QStringLiteral("device_id")).toString();
            requestCurrentHistory();
        }
    });

    connect(ui->periodCombo, &QComboBox::currentTextChanged, this, &HistoryPage::requestCurrentHistory);
    connect(ui->btnRefreshHistory, &QPushButton::clicked, this, &HistoryPage::requestCurrentHistory);
}

HistoryPage::~HistoryPage()
{
    delete ui;
}

void HistoryPage::setDevices(const QJsonArray &devices)
{
    m_devices = devices;
    ui->deviceCombo->clear();
    for (const auto &val : devices) {
        const QJsonObject dev = val.toObject();
        const QString id = dev.value(QStringLiteral("device_id")).toString();
        const QString name = dev.value(QStringLiteral("name")).toString(id);
        ui->deviceCombo->addItem(QStringLiteral("%1 (%2)").arg(name, id), id);
    }
    if (!devices.isEmpty()) {
        m_selectedDeviceId = devices.first().toObject().value(QStringLiteral("device_id")).toString();
        requestCurrentHistory();
    }
}

void HistoryPage::requestCurrentHistory()
{
    if (m_selectedDeviceId.isEmpty()) return;
    emit historyRequested(m_selectedDeviceId, ui->periodCombo->currentText(), QString());
}

void HistoryPage::setHistory(const QJsonObject &history)
{
    ui->historyTable->setRowCount(0);
    const QJsonArray rows = history.value(QStringLiteral("rows")).toArray();

    for (const auto &val : rows) {
        const QJsonObject row = val.toObject();
        const int r = ui->historyTable->rowCount();
        ui->historyTable->insertRow(r);

        const QString dt = row.value(QStringLiteral("recorded_at")).toString();
        const double sm = row.value(QStringLiteral("soil_moisture")).toDouble(55.0);
        const double t = row.value(QStringLiteral("temperature_c")).toDouble(27.5);
        const double h = row.value(QStringLiteral("humidity")).toDouble(65.0);
        const bool pump = row.value(QStringLiteral("pump_active")).toBool();

        ui->historyTable->setItem(r, 0, new QTableWidgetItem(dt.isEmpty() ? QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss")) : dt.mid(11, 8)));
        ui->historyTable->setItem(r, 1, new QTableWidgetItem(pump ? tr("💦 Tưới nước tự động") : tr("Giám sát định kỳ")));
        ui->historyTable->setItem(r, 2, new QTableWidgetItem(QStringLiteral("%1%").arg(QString::number(sm, 'f', 1))));
        ui->historyTable->setItem(r, 3, new QTableWidgetItem(QStringLiteral("%1 °C").arg(QString::number(t, 'f', 1))));
        ui->historyTable->setItem(r, 4, new QTableWidgetItem(QStringLiteral("%1 %RH").arg(QString::number(h, 'f', 1))));
        ui->historyTable->setItem(r, 5, new QTableWidgetItem(tr("Bình thường (85%)")));
    }
}
