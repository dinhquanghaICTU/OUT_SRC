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
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_HistoryPage
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *filterLayout;
    QPushButton *chartTabButton;
    QPushButton *tableTabButton;
    QComboBox *deviceCombo;
    QComboBox *periodCombo;
    QDateEdit *dateEdit;
    QPushButton *searchButton;
    QSpacerItem *filterSpacer;
    QLabel *recordCountLabel;
    QStackedWidget *viewStack;
    QWidget *chartPage;
    QWidget *tablePage;
    QVBoxLayout *tablePageLayout;
    QTableWidget *historyTable;

    void setupUi(QWidget *HistoryPage)
    {
        if (HistoryPage->objectName().isEmpty())
            HistoryPage->setObjectName("HistoryPage");
        verticalLayout = new QVBoxLayout(HistoryPage);
        verticalLayout->setSpacing(4);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(8, 6, 8, 6);
        filterLayout = new QHBoxLayout();
        filterLayout->setSpacing(6);
        filterLayout->setObjectName("filterLayout");
        chartTabButton = new QPushButton(HistoryPage);
        chartTabButton->setObjectName("chartTabButton");
        chartTabButton->setMinimumSize(QSize(0, 26));
        chartTabButton->setCheckable(true);
        chartTabButton->setChecked(true);

        filterLayout->addWidget(chartTabButton);

        tableTabButton = new QPushButton(HistoryPage);
        tableTabButton->setObjectName("tableTabButton");
        tableTabButton->setMinimumSize(QSize(0, 26));
        tableTabButton->setCheckable(true);

        filterLayout->addWidget(tableTabButton);

        deviceCombo = new QComboBox(HistoryPage);
        deviceCombo->setObjectName("deviceCombo");
        deviceCombo->setMinimumSize(QSize(150, 26));

        filterLayout->addWidget(deviceCombo);

        periodCombo = new QComboBox(HistoryPage);
        periodCombo->addItem(QString());
        periodCombo->addItem(QString());
        periodCombo->addItem(QString());
        periodCombo->setObjectName("periodCombo");
        periodCombo->setMinimumSize(QSize(72, 26));

        filterLayout->addWidget(periodCombo);

        dateEdit = new QDateEdit(HistoryPage);
        dateEdit->setObjectName("dateEdit");
        dateEdit->setMinimumSize(QSize(95, 26));
        dateEdit->setCalendarPopup(true);

        filterLayout->addWidget(dateEdit);

        searchButton = new QPushButton(HistoryPage);
        searchButton->setObjectName("searchButton");
        searchButton->setMinimumSize(QSize(0, 26));

        filterLayout->addWidget(searchButton);

        filterSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        filterLayout->addItem(filterSpacer);

        recordCountLabel = new QLabel(HistoryPage);
        recordCountLabel->setObjectName("recordCountLabel");

        filterLayout->addWidget(recordCountLabel);


        verticalLayout->addLayout(filterLayout);

        viewStack = new QStackedWidget(HistoryPage);
        viewStack->setObjectName("viewStack");
        chartPage = new QWidget();
        chartPage->setObjectName("chartPage");
        viewStack->addWidget(chartPage);
        tablePage = new QWidget();
        tablePage->setObjectName("tablePage");
        tablePageLayout = new QVBoxLayout(tablePage);
        tablePageLayout->setObjectName("tablePageLayout");
        tablePageLayout->setContentsMargins(0, 0, 0, 0);
        historyTable = new QTableWidget(tablePage);
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

        tablePageLayout->addWidget(historyTable);

        viewStack->addWidget(tablePage);

        verticalLayout->addWidget(viewStack);


        retranslateUi(HistoryPage);

        QMetaObject::connectSlotsByName(HistoryPage);
    } // setupUi

    void retranslateUi(QWidget *HistoryPage)
    {
        chartTabButton->setText(QCoreApplication::translate("HistoryPage", "\360\237\223\212  Bi\341\273\203u \304\221\341\273\223", nullptr));
        tableTabButton->setText(QCoreApplication::translate("HistoryPage", "\360\237\223\213  B\341\272\243ng", nullptr));
        periodCombo->setItemText(0, QCoreApplication::translate("HistoryPage", "Ng\303\240y", nullptr));
        periodCombo->setItemText(1, QCoreApplication::translate("HistoryPage", "Th\303\241ng", nullptr));
        periodCombo->setItemText(2, QCoreApplication::translate("HistoryPage", "N\304\203m", nullptr));

        dateEdit->setDisplayFormat(QCoreApplication::translate("HistoryPage", "dd/MM/yyyy", nullptr));
        searchButton->setText(QCoreApplication::translate("HistoryPage", "T\303\254m", nullptr));
        recordCountLabel->setText(QCoreApplication::translate("HistoryPage", "0 b\341\272\243n ghi", nullptr));
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
