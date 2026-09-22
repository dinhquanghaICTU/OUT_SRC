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
    ui->alertTable->setAlternatingRowColors(false);

    QPalette pal = ui->alertTable->palette();
    pal.setColor(QPalette::Base, QColor("#070d1e"));
    pal.setColor(QPalette::AlternateBase, QColor("#0f1c3f"));
    pal.setColor(QPalette::Text, QColor("#f8fafc"));
    pal.setColor(QPalette::WindowText, QColor("#f8fafc"));
    ui->alertTable->setPalette(pal);
}

void AlertPage::addAlert(const QString &message, double value)
{
    const int row = ui->alertTable->rowCount();
    ui->alertTable->insertRow(row);
    const QColor rowBg = (row % 2 == 0) ? QColor("#070d1e") : QColor("#0f1c3f");
    const QColor textColor = QColor("#f8fafc");

    auto *item0 = new QTableWidgetItem(QDateTime::currentDateTime().toString(QStringLiteral("dd/MM HH:mm:ss")));
    item0->setBackground(rowBg);
    item0->setForeground(textColor);
    item0->setTextAlignment(Qt::AlignCenter);

    auto *item1 = new QTableWidgetItem(message);
    item1->setBackground(rowBg);
    item1->setForeground(textColor);

    auto *item2 = new QTableWidgetItem(QString::number(value, 'f', 1));
    item2->setBackground(rowBg);
    item2->setForeground(textColor);
    item2->setTextAlignment(Qt::AlignCenter);

    auto *item3 = new QTableWidgetItem(tr("Chưa xử lý"));
    item3->setBackground(rowBg);
    item3->setForeground(QColor("#f59e0b"));
    item3->setTextAlignment(Qt::AlignCenter);

    ui->alertTable->setItem(row, 0, item0);
    ui->alertTable->setItem(row, 1, item1);
    ui->alertTable->setItem(row, 2, item2);
    ui->alertTable->setItem(row, 3, item3);
}

AlertPage::~AlertPage()
{
    delete ui;
}
