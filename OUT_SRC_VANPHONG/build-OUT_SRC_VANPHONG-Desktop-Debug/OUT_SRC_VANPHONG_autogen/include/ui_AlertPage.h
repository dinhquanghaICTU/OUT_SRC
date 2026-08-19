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
    QHBoxLayout *topBar;
    QLabel *alertTitle;
    QSpacerItem *alertSpacer;
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
        mainLayout->setContentsMargins(12, 8, 12, 8);
        topBar = new QHBoxLayout();
        topBar->setObjectName("topBar");
        alertTitle = new QLabel(AlertPage);
        alertTitle->setObjectName("alertTitle");
        alertTitle->setStyleSheet(QString::fromUtf8("color: #f59e0b; font-size: 13px; font-weight: 800;"));

        topBar->addWidget(alertTitle);

        alertSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        topBar->addItem(alertSpacer);

        btnClearAlerts = new QPushButton(AlertPage);
        btnClearAlerts->setObjectName("btnClearAlerts");
        btnClearAlerts->setCursor(QCursor(Qt::PointingHandCursor));
        btnClearAlerts->setStyleSheet(QString::fromUtf8("QPushButton { background: #3b1424; color: #f87171; border: 1px solid #7f1d1d; border-radius: 6px; padding: 4px 12px; font-weight: 700; font-size: 11px; } QPushButton:hover { background: #dc2626; color: white; }"));

        topBar->addWidget(btnClearAlerts);


        mainLayout->addLayout(topBar);

        alertTable = new QTableWidget(AlertPage);
        alertTable->setObjectName("alertTable");
        alertTable->setStyleSheet(QString::fromUtf8("QTableWidget { background-color: #0c2317; border: 1px solid #1b4332; border-radius: 8px; color: #f1f5f9; gridline-color: #1b4332; } QHeaderView::section { background-color: #133925; color: #34d399; font-weight: 700; padding: 6px; border: none; }"));

        mainLayout->addWidget(alertTable);


        retranslateUi(AlertPage);

        QMetaObject::connectSlotsByName(AlertPage);
    } // setupUi

    void retranslateUi(QWidget *AlertPage)
    {
        alertTitle->setText(QCoreApplication::translate("AlertPage", "\342\232\240\357\270\217 C\341\272\242NH B\303\201O AN TO\303\200N VI KH\303\215 H\341\272\254U & T\306\257\341\273\232I C\303\202Y", nullptr));
        btnClearAlerts->setText(QCoreApplication::translate("AlertPage", "\360\237\227\221\357\270\217 X\303\263a t\341\272\245t c\341\272\243", nullptr));
        (void)AlertPage;
    } // retranslateUi

};

namespace Ui {
    class AlertPage: public Ui_AlertPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ALERTPAGE_H
