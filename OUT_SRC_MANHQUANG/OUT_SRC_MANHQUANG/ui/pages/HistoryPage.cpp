#include "HistoryPage.h"
#include "ui_HistoryPage.h"

#include <QDateTime>
#include <QHeaderView>

HistoryPage::HistoryPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::HistoryPage)
{
    ui->setupUi(this);
    ui->historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->historyTable->verticalHeader()->hide();

    connect(ui->refreshBtn, &QPushButton::clicked, this, &HistoryPage::requestCurrentHistory);
    connect(ui->deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &HistoryPage::requestCurrentHistory);
    connect(ui->periodComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &HistoryPage::requestCurrentHistory);
}

HistoryPage::~HistoryPage()
{
    delete ui;
}

void HistoryPage::setDevices(const QJsonArray &devices)
{
    const QString currentSelected = ui->deviceComboBox->currentData().toString();
    ui->deviceComboBox->clear();

    for (const auto &val : devices) {
        const QJsonObject dev = val.toObject();
        const QString did = dev.value(QStringLiteral("device_id")).toString();
        const QString name = dev.value(QStringLiteral("name")).toString(did);
        ui->deviceComboBox->addItem(QStringLiteral("%1 (%2)").arg(name, did), did);
    }

    if (!currentSelected.isEmpty()) {
        const int idx = ui->deviceComboBox->findData(currentSelected);
        if (idx >= 0) ui->deviceComboBox->setCurrentIndex(idx);
    }
}

void HistoryPage::requestCurrentHistory()
{
    const QString did = ui->deviceComboBox->currentData().toString();
    if (did.isEmpty()) return;

    const QString period = ui->periodComboBox->currentIndex() == 0 ? QStringLiteral("today")
        : (ui->periodComboBox->currentIndex() == 1 ? QStringLiteral("7d") : QStringLiteral("30d"));

    emit historyRequested(did, period, QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd")));
}

void HistoryPage::setHistory(const QJsonObject &history)
{
    m_cachedRows = history.value(QStringLiteral("rows")).toArray();
    ui->historyTable->setRowCount(0);

    for (const auto &val : m_cachedRows) {
        const QJsonObject rowObj = val.toObject();
        const QString recordedAt = rowObj.value(QStringLiteral("recorded_at")).toString();

        const int row = ui->historyTable->rowCount();
        ui->historyTable->insertRow(row);

        ui->historyTable->setItem(row, 0, new QTableWidgetItem(recordedAt));
        ui->historyTable->setItem(row, 1, new QTableWidgetItem(
            rowObj.value(QStringLiteral("door_state")).toString(QStringLiteral("CLOSED"))));
        ui->historyTable->setItem(row, 2, new QTableWidgetItem(
            rowObj.value(QStringLiteral("motion_detected")).toBool() ? tr("🚶 Có người") : tr("Không có người")));
        ui->historyTable->setItem(row, 3, new QTableWidgetItem(
            rowObj.value(QStringLiteral("ir_blocked")).toBool() ? tr("⚠️ BỊ CHẮN") : tr("Thông suốt")));
        ui->historyTable->setItem(row, 4, new QTableWidgetItem(
            QStringLiteral("%1% (%2 RPM)")
                .arg(QString::number(rowObj.value(QStringLiteral("door_position_pct")).toDouble(), 'f', 0))
                .arg(QString::number(rowObj.value(QStringLiteral("motor_speed_rpm")).toDouble(), 'f', 0))));
    }
}
