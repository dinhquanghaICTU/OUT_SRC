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
#include <QtWidgets/QDateEdit>
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
    QVBoxLayout *verticalLayout;
    QHBoxLayout *headerLayout;
    QLabel *titleLabel;
    QSpacerItem *headerSpacer;
    QLabel *recordCountLabel;
    QHBoxLayout *filterLayout;
    QLabel *deviceLabel;
    QComboBox *deviceCombo;
    QLabel *periodLabel;
    QComboBox *periodCombo;
    QDateEdit *dateEdit;
    QPushButton *searchButton;
    QSpacerItem *filterSpacer;
    QLabel *averagesLabel;
    QTableWidget *historyTable;

    void setupUi(QWidget *HistoryPage)
    {
        if (HistoryPage->objectName().isEmpty())
            HistoryPage->setObjectName("HistoryPage");
        verticalLayout = new QVBoxLayout(HistoryPage);
        verticalLayout->setSpacing(12);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(24, 20, 24, 20);
        headerLayout = new QHBoxLayout();
        headerLayout->setObjectName("headerLayout");
        titleLabel = new QLabel(HistoryPage);
        titleLabel->setObjectName("titleLabel");

        headerLayout->addWidget(titleLabel);

        headerSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        headerLayout->addItem(headerSpacer);

        recordCountLabel = new QLabel(HistoryPage);
        recordCountLabel->setObjectName("recordCountLabel");

        headerLayout->addWidget(recordCountLabel);


        verticalLayout->addLayout(headerLayout);

        filterLayout = new QHBoxLayout();
        filterLayout->setObjectName("filterLayout");
        deviceLabel = new QLabel(HistoryPage);
        deviceLabel->setObjectName("deviceLabel");

        filterLayout->addWidget(deviceLabel);

        deviceCombo = new QComboBox(HistoryPage);
        deviceCombo->setObjectName("deviceCombo");
        deviceCombo->setMinimumSize(QSize(0, 36));

        filterLayout->addWidget(deviceCombo);

        periodLabel = new QLabel(HistoryPage);
        periodLabel->setObjectName("periodLabel");

        filterLayout->addWidget(periodLabel);

        periodCombo = new QComboBox(HistoryPage);
        periodCombo->addItem(QString());
        periodCombo->addItem(QString());
        periodCombo->addItem(QString());
        periodCombo->setObjectName("periodCombo");

        filterLayout->addWidget(periodCombo);

        dateEdit = new QDateEdit(HistoryPage);
        dateEdit->setObjectName("dateEdit");
        dateEdit->setCalendarPopup(true);

        filterLayout->addWidget(dateEdit);

        searchButton = new QPushButton(HistoryPage);
        searchButton->setObjectName("searchButton");

        filterLayout->addWidget(searchButton);

        filterSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        filterLayout->addItem(filterSpacer);


        verticalLayout->addLayout(filterLayout);

        averagesLabel = new QLabel(HistoryPage);
        averagesLabel->setObjectName("averagesLabel");
        averagesLabel->setWordWrap(true);

        verticalLayout->addWidget(averagesLabel);

        historyTable = new QTableWidget(HistoryPage);
        if (historyTable->columnCount() < 1)
            historyTable->setColumnCount(1);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        historyTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        historyTable->setObjectName("historyTable");
        historyTable->setAlternatingRowColors(true);
        historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        historyTable->setColumnCount(1);
        historyTable->setRowCount(0);

        verticalLayout->addWidget(historyTable);


        retranslateUi(HistoryPage);

        QMetaObject::connectSlotsByName(HistoryPage);
    } // setupUi

    void retranslateUi(QWidget *HistoryPage)
    {
        titleLabel->setText(QCoreApplication::translate("HistoryPage", "L\341\273\212CH S\341\273\254 THI\341\272\276T B\341\273\212", nullptr));
        recordCountLabel->setText(QCoreApplication::translate("HistoryPage", "0 b\341\272\243n ghi", nullptr));
        deviceLabel->setText(QCoreApplication::translate("HistoryPage", "Thi\341\272\277t b\341\273\213", nullptr));
        periodLabel->setText(QCoreApplication::translate("HistoryPage", "Th\341\273\221ng k\303\252 theo", nullptr));
        periodCombo->setItemText(0, QCoreApplication::translate("HistoryPage", "Ng\303\240y", nullptr));
        periodCombo->setItemText(1, QCoreApplication::translate("HistoryPage", "Th\303\241ng", nullptr));
        periodCombo->setItemText(2, QCoreApplication::translate("HistoryPage", "N\304\203m", nullptr));

        dateEdit->setDisplayFormat(QCoreApplication::translate("HistoryPage", "dd/MM/yyyy", nullptr));
        searchButton->setText(QCoreApplication::translate("HistoryPage", "T\303\254m ki\341\272\277m", nullptr));
        averagesLabel->setText(QCoreApplication::translate("HistoryPage", "Ch\341\273\215n m\341\273\231t thi\341\272\277t b\341\273\213 \304\221\341\273\203 xem l\341\273\213ch s\341\273\255 v\303\240 gi\303\241 tr\341\273\213 trung b\303\254nh.", nullptr));
        QTableWidgetItem *___qtablewidgetitem = historyTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("HistoryPage", "Th\341\273\235i gian", nullptr));
        (void)HistoryPage;
    } // retranslateUi

};

namespace Ui {
    class HistoryPage: public Ui_HistoryPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HISTORYPAGE_H
