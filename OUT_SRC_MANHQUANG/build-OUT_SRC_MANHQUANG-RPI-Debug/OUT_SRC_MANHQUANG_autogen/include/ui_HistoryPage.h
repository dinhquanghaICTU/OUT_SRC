/********************************************************************************
** Form generated from reading UI file 'HistoryPage.ui'
**
** Created by: Qt User Interface Compiler version 6.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HISTORYPAGE_H
#define UI_HISTORYPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_HistoryPage
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *filterLayout;
    QLabel *titleLabel;
    QSpacerItem *spacer1;
    QLabel *devSelectLabel;
    QComboBox *deviceComboBox;
    QComboBox *periodComboBox;
    QPushButton *refreshBtn;
    QTableWidget *historyTable;

    void setupUi(QWidget *HistoryPage)
    {
        if (HistoryPage->objectName().isEmpty())
            HistoryPage->setObjectName("HistoryPage");
        HistoryPage->resize(800, 480);
        mainLayout = new QVBoxLayout(HistoryPage);
        mainLayout->setSpacing(8);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(12, 10, 12, 10);
        filterLayout = new QHBoxLayout();
        filterLayout->setSpacing(10);
        filterLayout->setObjectName("filterLayout");
        titleLabel = new QLabel(HistoryPage);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setStyleSheet(QString::fromUtf8("color: #38bdf8; font-size: 14px; font-weight: 800;"));

        filterLayout->addWidget(titleLabel);

        spacer1 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        filterLayout->addItem(spacer1);

        devSelectLabel = new QLabel(HistoryPage);
        devSelectLabel->setObjectName("devSelectLabel");
        devSelectLabel->setStyleSheet(QString::fromUtf8("color: #94a3b8; font-size: 11px;"));

        filterLayout->addWidget(devSelectLabel);

        deviceComboBox = new QComboBox(HistoryPage);
        deviceComboBox->setObjectName("deviceComboBox");
        deviceComboBox->setMinimumSize(QSize(160, 30));
        deviceComboBox->setStyleSheet(QString::fromUtf8("QComboBox { background-color: #1e293b; color: white; border: 1px solid #334155; border-radius: 6px; padding: 4px 8px; font-size: 11px; }"));

        filterLayout->addWidget(deviceComboBox);

        periodComboBox = new QComboBox(HistoryPage);
        periodComboBox->addItem(QString());
        periodComboBox->addItem(QString());
        periodComboBox->addItem(QString());
        periodComboBox->setObjectName("periodComboBox");
        periodComboBox->setMinimumSize(QSize(100, 30));
        periodComboBox->setStyleSheet(QString::fromUtf8("QComboBox { background-color: #1e293b; color: white; border: 1px solid #334155; border-radius: 6px; padding: 4px 8px; font-size: 11px; }"));

        filterLayout->addWidget(periodComboBox);

        refreshBtn = new QPushButton(HistoryPage);
        refreshBtn->setObjectName("refreshBtn");
        refreshBtn->setMinimumSize(QSize(80, 30));
        refreshBtn->setCursor(QCursor(Qt::PointingHandCursor));
        refreshBtn->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #0284c7; color: white; border: none; border-radius: 6px; font-size: 11px; font-weight: 700; } QPushButton:hover { background-color: #0369a1; }"));

        filterLayout->addWidget(refreshBtn);


        mainLayout->addLayout(filterLayout);

        historyTable = new QTableWidget(HistoryPage);
        if (historyTable->columnCount() < 5)
            historyTable->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        historyTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        historyTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        historyTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        historyTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        historyTable->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        historyTable->setObjectName("historyTable");
        historyTable->setColumnCount(5);
        historyTable->setStyleSheet(QString::fromUtf8("QTableWidget { background-color: #0f172a; border: 1.5px solid #1e293b; color: #f1f5f9; border-radius: 10px; gridline-color: #1e293b; } QHeaderView::section { background-color: #1e293b; color: #38bdf8; font-weight: 700; padding: 6px; border: none; }"));

        mainLayout->addWidget(historyTable);


        retranslateUi(HistoryPage);

        QMetaObject::connectSlotsByName(HistoryPage);
    } // setupUi

    void retranslateUi(QWidget *HistoryPage)
    {
        titleLabel->setText(QCoreApplication::translate("HistoryPage", "\360\237\223\234 NH\341\272\254T K\303\235 RA V\303\200O & L\341\273\212CH S\341\273\254 C\341\273\254A", nullptr));
        devSelectLabel->setText(QCoreApplication::translate("HistoryPage", "Thi\341\272\277t b\341\273\213:", nullptr));
        periodComboBox->setItemText(0, QCoreApplication::translate("HistoryPage", "H\303\264m nay", nullptr));
        periodComboBox->setItemText(1, QCoreApplication::translate("HistoryPage", "7 ng\303\240y qua", nullptr));
        periodComboBox->setItemText(2, QCoreApplication::translate("HistoryPage", "30 ng\303\240y qua", nullptr));

        refreshBtn->setText(QCoreApplication::translate("HistoryPage", "\360\237\224\204 T\341\272\243i d\341\273\257 li\341\273\207u", nullptr));
        QTableWidgetItem *___qtablewidgetitem = historyTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("HistoryPage", "Th\341\273\235i \304\221i\341\273\203m ghi nh\341\272\255n", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = historyTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("HistoryPage", "Tr\341\272\241ng th\303\241i c\341\273\255a", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = historyTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("HistoryPage", "C\341\272\243m bi\341\272\277n SR602 (PIR)", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = historyTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("HistoryPage", "C\341\272\243m bi\341\272\277n IR (Ch\341\273\221ng k\341\272\271t)", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = historyTable->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("HistoryPage", "V\341\273\213 tr\303\255 & T\341\273\221c \304\221\341\273\231 b\306\260\341\273\233c", nullptr));
        (void)HistoryPage;
    } // retranslateUi

};

namespace Ui {
    class HistoryPage: public Ui_HistoryPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HISTORYPAGE_H
