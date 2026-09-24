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
}

void AlertPage::addAlert(const QString &message, double value)
{
    const int row = ui->alertTable->rowCount();
    ui->alertTable->insertRow(row);
    ui->alertTable->setItem(row, 0, new QTableWidgetItem(
        QDateTime::currentDateTime().toString(QStringLiteral("dd/MM HH:mm:ss"))));
    ui->alertTable->setItem(row, 1, new QTableWidgetItem(message));
    ui->alertTable->setItem(row, 2, new QTableWidgetItem(QString::number(value, 'f', 1)));
    ui->alertTable->setItem(row, 3, new QTableWidgetItem(tr("Chưa xử lý")));
}

AlertPage::~AlertPage()
{
    delete ui;
}
