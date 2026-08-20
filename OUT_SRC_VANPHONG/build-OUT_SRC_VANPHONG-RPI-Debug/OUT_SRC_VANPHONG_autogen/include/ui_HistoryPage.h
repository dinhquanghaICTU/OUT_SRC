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
#include <QtWidgets/QFrame>
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
    QHBoxLayout *filterRow;
    QLabel *pageTitle;
    QSpacerItem *fSpacer;
    QComboBox *deviceCombo;
    QComboBox *periodCombo;
    QPushButton *btnRefreshHistory;
    QHBoxLayout *statsRow;
    QFrame *statCard1;
    QVBoxLayout *sc1;
    QLabel *st1;
    QLabel *valWaterCount;
    QFrame *statCard2;
    QVBoxLayout *sc2;
    QLabel *st2;
    QLabel *valWaterVolume;
    QFrame *statCard3;
    QVBoxLayout *sc3;
    QLabel *st3;
    QLabel *valAvgSoil;
    QFrame *statCard4;
    QVBoxLayout *sc4;
    QLabel *st4;
    QLabel *valMaxTemp;
    QTableWidget *historyTable;

    void setupUi(QWidget *HistoryPage)
    {
        if (HistoryPage->objectName().isEmpty())
            HistoryPage->setObjectName("HistoryPage");
        HistoryPage->resize(800, 480);
        mainLayout = new QVBoxLayout(HistoryPage);
        mainLayout->setSpacing(8);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(12, 8, 12, 8);
        filterRow = new QHBoxLayout();
        filterRow->setSpacing(8);
        filterRow->setObjectName("filterRow");
        pageTitle = new QLabel(HistoryPage);
        pageTitle->setObjectName("pageTitle");
        pageTitle->setStyleSheet(QString::fromUtf8("color: #34d399; font-size: 13px; font-weight: 800;"));

        filterRow->addWidget(pageTitle);

        fSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        filterRow->addItem(fSpacer);

        deviceCombo = new QComboBox(HistoryPage);
        deviceCombo->setObjectName("deviceCombo");
        deviceCombo->setMinimumSize(QSize(160, 30));
        deviceCombo->setStyleSheet(QString::fromUtf8("QComboBox { background-color: #0c2317; border: 1px solid #1b4332; border-radius: 6px; color: #f1f5f9; padding: 4px 10px; font-size: 11px; }"));

        filterRow->addWidget(deviceCombo);

        periodCombo = new QComboBox(HistoryPage);
        periodCombo->addItem(QString());
        periodCombo->addItem(QString());
        periodCombo->addItem(QString());
        periodCombo->setObjectName("periodCombo");
        periodCombo->setMinimumSize(QSize(110, 30));
        periodCombo->setStyleSheet(QString::fromUtf8("QComboBox { background-color: #0c2317; border: 1px solid #1b4332; border-radius: 6px; color: #f1f5f9; padding: 4px 10px; font-size: 11px; }"));

        filterRow->addWidget(periodCombo);

        btnRefreshHistory = new QPushButton(HistoryPage);
        btnRefreshHistory->setObjectName("btnRefreshHistory");
        btnRefreshHistory->setMinimumSize(QSize(0, 30));
        btnRefreshHistory->setCursor(QCursor(Qt::PointingHandCursor));
        btnRefreshHistory->setStyleSheet(QString::fromUtf8("QPushButton { background: #059669; color: white; border: none; border-radius: 6px; padding: 4px 12px; font-weight: 700; font-size: 11px; } QPushButton:hover { background: #10b981; }"));

        filterRow->addWidget(btnRefreshHistory);


        mainLayout->addLayout(filterRow);

        statsRow = new QHBoxLayout();
        statsRow->setSpacing(8);
        statsRow->setObjectName("statsRow");
        statCard1 = new QFrame(HistoryPage);
        statCard1->setObjectName("statCard1");
        statCard1->setStyleSheet(QString::fromUtf8("QFrame#statCard1 { background: #0c2317; border: 1px solid #1b4332; border-radius: 8px; padding: 6px; }"));
        sc1 = new QVBoxLayout(statCard1);
        sc1->setContentsMargins(4, 4, 4, 4);
        sc1->setObjectName("sc1");
        st1 = new QLabel(statCard1);
        st1->setObjectName("st1");
        st1->setStyleSheet(QString::fromUtf8("color: #a7f3d0; font-size: 9px; font-weight: 700;"));

        sc1->addWidget(st1);

        valWaterCount = new QLabel(statCard1);
        valWaterCount->setObjectName("valWaterCount");
        valWaterCount->setStyleSheet(QString::fromUtf8("color: #34d399; font-size: 14px; font-weight: 900;"));

        sc1->addWidget(valWaterCount);


        statsRow->addWidget(statCard1);

        statCard2 = new QFrame(HistoryPage);
        statCard2->setObjectName("statCard2");
        statCard2->setStyleSheet(QString::fromUtf8("QFrame#statCard2 { background: #0c2317; border: 1px solid #1b4332; border-radius: 8px; padding: 6px; }"));
        sc2 = new QVBoxLayout(statCard2);
        sc2->setContentsMargins(4, 4, 4, 4);
        sc2->setObjectName("sc2");
        st2 = new QLabel(statCard2);
        st2->setObjectName("st2");
        st2->setStyleSheet(QString::fromUtf8("color: #a7f3d0; font-size: 9px; font-weight: 700;"));

        sc2->addWidget(st2);

        valWaterVolume = new QLabel(statCard2);
        valWaterVolume->setObjectName("valWaterVolume");
        valWaterVolume->setStyleSheet(QString::fromUtf8("color: #38bdf8; font-size: 14px; font-weight: 900;"));

        sc2->addWidget(valWaterVolume);


        statsRow->addWidget(statCard2);

        statCard3 = new QFrame(HistoryPage);
        statCard3->setObjectName("statCard3");
        statCard3->setStyleSheet(QString::fromUtf8("QFrame#statCard3 { background: #0c2317; border: 1px solid #1b4332; border-radius: 8px; padding: 6px; }"));
        sc3 = new QVBoxLayout(statCard3);
        sc3->setContentsMargins(4, 4, 4, 4);
        sc3->setObjectName("sc3");
        st3 = new QLabel(statCard3);
        st3->setObjectName("st3");
        st3->setStyleSheet(QString::fromUtf8("color: #a7f3d0; font-size: 9px; font-weight: 700;"));

        sc3->addWidget(st3);

        valAvgSoil = new QLabel(statCard3);
        valAvgSoil->setObjectName("valAvgSoil");
        valAvgSoil->setStyleSheet(QString::fromUtf8("color: #10b981; font-size: 14px; font-weight: 900;"));

        sc3->addWidget(valAvgSoil);


        statsRow->addWidget(statCard3);

        statCard4 = new QFrame(HistoryPage);
        statCard4->setObjectName("statCard4");
        statCard4->setStyleSheet(QString::fromUtf8("QFrame#statCard4 { background: #0c2317; border: 1px solid #1b4332; border-radius: 8px; padding: 6px; }"));
        sc4 = new QVBoxLayout(statCard4);
        sc4->setContentsMargins(4, 4, 4, 4);
        sc4->setObjectName("sc4");
        st4 = new QLabel(statCard4);
        st4->setObjectName("st4");
        st4->setStyleSheet(QString::fromUtf8("color: #a7f3d0; font-size: 9px; font-weight: 700;"));

        sc4->addWidget(st4);

        valMaxTemp = new QLabel(statCard4);
        valMaxTemp->setObjectName("valMaxTemp");
        valMaxTemp->setStyleSheet(QString::fromUtf8("color: #f59e0b; font-size: 14px; font-weight: 900;"));

        sc4->addWidget(valMaxTemp);


        statsRow->addWidget(statCard4);


        mainLayout->addLayout(statsRow);

        historyTable = new QTableWidget(HistoryPage);
        historyTable->setObjectName("historyTable");
        historyTable->setStyleSheet(QString::fromUtf8("QTableWidget { background-color: #0c2317; border: 1px solid #1b4332; border-radius: 8px; color: #f1f5f9; gridline-color: #1b4332; } QHeaderView::section { background-color: #133925; color: #34d399; font-weight: 700; padding: 6px; border: none; }"));

        mainLayout->addWidget(historyTable);


        retranslateUi(HistoryPage);

        QMetaObject::connectSlotsByName(HistoryPage);
    } // setupUi

    void retranslateUi(QWidget *HistoryPage)
    {
        pageTitle->setText(QCoreApplication::translate("HistoryPage", "\360\237\223\234 L\341\273\212CH S\341\273\254 T\306\257\341\273\232I C\303\202Y & \304\220\341\273\230 \341\272\250M \304\220\341\272\244T", nullptr));
        periodCombo->setItemText(0, QCoreApplication::translate("HistoryPage", "H\303\264m nay", nullptr));
        periodCombo->setItemText(1, QCoreApplication::translate("HistoryPage", "7 ng\303\240y qua", nullptr));
        periodCombo->setItemText(2, QCoreApplication::translate("HistoryPage", "30 ng\303\240y qua", nullptr));

        btnRefreshHistory->setText(QCoreApplication::translate("HistoryPage", "\360\237\224\204 T\341\272\243i d\341\273\257 li\341\273\207u", nullptr));
        st1->setText(QCoreApplication::translate("HistoryPage", "L\306\257\341\273\242T T\306\257\341\273\232I H\303\224M NAY", nullptr));
        valWaterCount->setText(QCoreApplication::translate("HistoryPage", "4 l\341\272\247n", nullptr));
        st2->setText(QCoreApplication::translate("HistoryPage", "L\306\257\341\273\242NG N\306\257\341\273\232C TI\303\212U TH\341\273\244", nullptr));
        valWaterVolume->setText(QCoreApplication::translate("HistoryPage", "14.5 L\303\255t", nullptr));
        st3->setText(QCoreApplication::translate("HistoryPage", "\304\220\341\273\230 \341\272\250M \304\220\341\272\244T TRUNG B\303\214NH", nullptr));
        valAvgSoil->setText(QCoreApplication::translate("HistoryPage", "58.4 %", nullptr));
        st4->setText(QCoreApplication::translate("HistoryPage", "NHI\341\273\206T \304\220\341\273\230 KH\303\215 CAO NH\341\272\244T", nullptr));
        valMaxTemp->setText(QCoreApplication::translate("HistoryPage", "31.2 \302\260C", nullptr));
        (void)HistoryPage;
    } // retranslateUi

};

namespace Ui {
    class HistoryPage: public Ui_HistoryPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HISTORYPAGE_H
