#include "AlertPage.h"
#include "ui_AlertPage.h"

#include <QDateTime>
#include <QHeaderView>

AlertPage::AlertPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::AlertPage)
{
    ui->setupUi(this);

    ui->alertTable->setColumnCount(4);
    ui->alertTable->setHorizontalHeaderLabels({
        tr("Thời gian"), tr("Nguồn phát"), tr("Mức độ"), tr("Nội dung cảnh báo")
    });
    ui->alertTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->alertTable->verticalHeader()->hide();

    connect(ui->btnClearAlerts, &QPushButton::clicked, this, &AlertPage::clearAlerts);
}

AlertPage::~AlertPage()
{
    delete ui;
}

void AlertPage::addAlert(const QString &source, const QString &severity, const QString &message)
{
    const int r = 0;
    ui->alertTable->insertRow(r);

    ui->alertTable->setItem(r, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"))));
    ui->alertTable->setItem(r, 1, new QTableWidgetItem(source));

    auto *sevItem = new QTableWidgetItem(severity.toUpper());
    if (severity == QStringLiteral("critical") || severity == QStringLiteral("danger")) {
        sevItem->setForeground(QColor(QStringLiteral("#ef4444")));
    } else {
        sevItem->setForeground(QColor(QStringLiteral("#f59e0b")));
    }
    ui->alertTable->setItem(r, 2, sevItem);
    ui->alertTable->setItem(r, 3, new QTableWidgetItem(message));
}

void AlertPage::clearAlerts()
{
    ui->alertTable->setRowCount(0);
}
