#include "AlertPage.h"
#include "ui_AlertPage.h"

#include <QDateTime>
#include <QHeaderView>

AlertPage::AlertPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::AlertPage)
{
    ui->setupUi(this);
    ui->alertTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->alertTable->verticalHeader()->hide();

    connect(ui->btnClearAlerts, &QPushButton::clicked, this, [this] {
        ui->alertTable->setRowCount(0);
    });
}

AlertPage::~AlertPage()
{
    delete ui;
}

void AlertPage::addAlert(const QString &source, const QString &severity, const QString &message)
{
    const int row = ui->alertTable->rowCount();
    ui->alertTable->insertRow(row);

    ui->alertTable->setItem(row, 0, new QTableWidgetItem(
        QDateTime::currentDateTime().toString(QStringLiteral("dd/MM HH:mm:ss"))));
    ui->alertTable->setItem(row, 1, new QTableWidgetItem(source));

    auto *sevItem = new QTableWidgetItem(severity);
    if (severity.contains(QStringLiteral("danger"), Qt::CaseInsensitive) || severity.contains(QStringLiteral("critical"), Qt::CaseInsensitive))
        sevItem->setForeground(QColor(QStringLiteral("#ef4444")));
    else
        sevItem->setForeground(QColor(QStringLiteral("#f59e0b")));
    ui->alertTable->setItem(row, 2, sevItem);

    ui->alertTable->setItem(row, 3, new QTableWidgetItem(message));
}
