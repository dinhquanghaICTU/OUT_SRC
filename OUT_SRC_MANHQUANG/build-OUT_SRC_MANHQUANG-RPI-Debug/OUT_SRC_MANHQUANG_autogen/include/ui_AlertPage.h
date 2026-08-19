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
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AlertPage
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *headerLayout;
    QLabel *titleLabel;
    QSpacerItem *headerSpacer;
    QPushButton *btnClearAlerts;
    QTableWidget *alertTable;

    void setupUi(QWidget *AlertPage)
    {
        if (AlertPage->objectName().isEmpty())
            AlertPage->setObjectName("AlertPage");
        AlertPage->resize(800, 480);
        mainLayout = new QVBoxLayout(AlertPage);
        mainLayout->setSpacing(8);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(12, 10, 12, 10);
        headerLayout = new QHBoxLayout();
        headerLayout->setObjectName("headerLayout");
        titleLabel = new QLabel(AlertPage);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setStyleSheet(QString::fromUtf8("color: #f87171; font-size: 14px; font-weight: 800;"));

        headerLayout->addWidget(titleLabel);

        headerSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        headerLayout->addItem(headerSpacer);

        btnClearAlerts = new QPushButton(AlertPage);
        btnClearAlerts->setObjectName("btnClearAlerts");
        btnClearAlerts->setCursor(QCursor(Qt::PointingHandCursor));
        btnClearAlerts->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #334155; color: #cbd5e1; border: none; border-radius: 6px; padding: 6px 12px; font-weight: 600; font-size: 11px; } QPushButton:hover { background-color: #475569; }"));

        headerLayout->addWidget(btnClearAlerts);


        mainLayout->addLayout(headerLayout);

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
        alertTable->setColumnCount(4);
        alertTable->setStyleSheet(QString::fromUtf8("QTableWidget { background-color: #0f172a; border: 1.5px solid #1e293b; color: #f1f5f9; border-radius: 10px; gridline-color: #1e293b; } QHeaderView::section { background-color: #1e293b; color: #f87171; font-weight: 700; padding: 6px; border: none; }"));

        mainLayout->addWidget(alertTable);


        retranslateUi(AlertPage);

        QMetaObject::connectSlotsByName(AlertPage);
    } // setupUi

    void retranslateUi(QWidget *AlertPage)
    {
        titleLabel->setText(QCoreApplication::translate("AlertPage", "\342\232\240\357\270\217 C\341\272\242NH B\303\201O AN TO\303\200N & S\341\273\260 C\341\273\220 C\341\273\254A", nullptr));
        btnClearAlerts->setText(QCoreApplication::translate("AlertPage", "\360\237\227\221\357\270\217 X\303\263a danh s\303\241ch", nullptr));
        QTableWidgetItem *___qtablewidgetitem = alertTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("AlertPage", "Th\341\273\235i gian", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = alertTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("AlertPage", "Ngu\341\273\223n c\341\272\243nh b\303\241o", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = alertTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("AlertPage", "M\341\273\251c \304\221\341\273\231", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = alertTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("AlertPage", "N\341\273\231i dung chi ti\341\272\277t", nullptr));
        (void)AlertPage;
    } // retranslateUi

};

namespace Ui {
    class AlertPage: public Ui_AlertPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ALERTPAGE_H
