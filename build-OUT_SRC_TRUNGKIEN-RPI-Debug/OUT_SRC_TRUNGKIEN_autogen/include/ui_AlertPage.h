/********************************************************************************
** Form generated from reading UI file 'AlertPage.ui'
**
** Created by: Qt User Interface Compiler version 6.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ALERTPAGE_H
#define UI_ALERTPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AlertPage
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *titleLabel;
    QTableWidget *alertTable;

    void setupUi(QWidget *AlertPage)
    {
        if (AlertPage->objectName().isEmpty())
            AlertPage->setObjectName("AlertPage");
        verticalLayout = new QVBoxLayout(AlertPage);
        verticalLayout->setObjectName("verticalLayout");
        titleLabel = new QLabel(AlertPage);
        titleLabel->setObjectName("titleLabel");

        verticalLayout->addWidget(titleLabel);

        alertTable = new QTableWidget(AlertPage);
        if (alertTable->columnCount() < 4)
            alertTable->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        alertTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        alertTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        alertTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        alertTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        alertTable->setObjectName("alertTable");
        alertTable->setAlternatingRowColors(true);
        alertTable->setColumnCount(4);
        alertTable->setRowCount(0);

        verticalLayout->addWidget(alertTable);


        retranslateUi(AlertPage);

        QMetaObject::connectSlotsByName(AlertPage);
    } // setupUi

    void retranslateUi(QWidget *AlertPage)
    {
        titleLabel->setText(QCoreApplication::translate("AlertPage", "C\341\272\242NH B\303\201O H\341\273\206 TH\341\273\220NG", nullptr));
        QTableWidgetItem *___qtablewidgetitem = alertTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("AlertPage", "Th\341\273\235i gian", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = alertTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("AlertPage", "Lo\341\272\241i c\341\272\243nh b\303\241o", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = alertTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("AlertPage", "Gi\303\241 tr\341\273\213", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = alertTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("AlertPage", "Tr\341\272\241ng th\303\241i", nullptr));
        (void)AlertPage;
    } // retranslateUi

};

namespace Ui {
    class AlertPage: public Ui_AlertPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ALERTPAGE_H
