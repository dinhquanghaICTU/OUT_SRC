/********************************************************************************
** Form generated from reading UI file 'DashboardPage.ui'
**
** Created by: Qt User Interface Compiler version 6.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DASHBOARDPAGE_H
#define UI_DASHBOARDPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DashboardPage
{
public:
    QHBoxLayout *dashboardHLayout;
    QFrame *zone1Frame;
    QVBoxLayout *z1Layout;
    QLabel *z1Title;
    QVBoxLayout *plantCanvasContainer;
    QHBoxLayout *z1SimRow;
    QPushButton *btnSimDrySoil;
    QPushButton *btnSimMoistSoil;
    QVBoxLayout *z2Layout;
    QHBoxLayout *z2MetricsRow;
    QFrame *cardSoil;
    QVBoxLayout *csLayout;
    QLabel *csTitle;
    QLabel *soilMoistureValueLabel;
    QLabel *soilMoistureSubLabel;
    QFrame *cardTemp;
    QVBoxLayout *ctLayout;
    QLabel *ctTitle;
    QLabel *tempValueLabel;
    QLabel *tempSubLabel;
    QFrame *cardHum;
    QVBoxLayout *chLayout;
    QLabel *chTitle;
    QLabel *humValueLabel;
    QLabel *humSubLabel;
    QFrame *chartCard;
    QVBoxLayout *chartContainerLayout;
    QFrame *zone3Frame;
    QVBoxLayout *z3Layout;
    QLabel *z3Title;
    QPushButton *btnTogglePumpMaster;
    QHBoxLayout *quickTimerRow;
    QPushButton *btnTimer30s;
    QPushButton *btnTimer1m;
    QPushButton *btnTimer3m;
    QPushButton *btnAutoModeCapsule;
    QSpacerItem *z3Spacer;
    QFrame *tankStatusBox;
    QVBoxLayout *tsbLayout;
    QLabel *tankLevelLabel;
    QLabel *tankSubLabel;

    void setupUi(QWidget *DashboardPage)
    {
        if (DashboardPage->objectName().isEmpty())
            DashboardPage->setObjectName("DashboardPage");
        DashboardPage->resize(800, 420);
        dashboardHLayout = new QHBoxLayout(DashboardPage);
        dashboardHLayout->setSpacing(10);
        dashboardHLayout->setObjectName("dashboardHLayout");
        dashboardHLayout->setContentsMargins(10, 8, 10, 8);
        zone1Frame = new QFrame(DashboardPage);
        zone1Frame->setObjectName("zone1Frame");
        zone1Frame->setMinimumSize(QSize(250, 0));
        zone1Frame->setMaximumSize(QSize(270, 16777215));
        zone1Frame->setStyleSheet(QString::fromUtf8("QFrame#zone1Frame { background-color: #06150c; border: 1.5px solid #1b4332; border-radius: 14px; }"));
        z1Layout = new QVBoxLayout(zone1Frame);
        z1Layout->setSpacing(6);
        z1Layout->setObjectName("z1Layout");
        z1Layout->setContentsMargins(8, 8, 8, 8);
        z1Title = new QLabel(zone1Frame);
        z1Title->setObjectName("z1Title");
        z1Title->setStyleSheet(QString::fromUtf8("color: #34d399; font-size: 11px; font-weight: 800;"));

        z1Layout->addWidget(z1Title);

        plantCanvasContainer = new QVBoxLayout();
        plantCanvasContainer->setObjectName("plantCanvasContainer");

        z1Layout->addLayout(plantCanvasContainer);

        z1SimRow = new QHBoxLayout();
        z1SimRow->setSpacing(4);
        z1SimRow->setObjectName("z1SimRow");
        btnSimDrySoil = new QPushButton(zone1Frame);
        btnSimDrySoil->setObjectName("btnSimDrySoil");
        btnSimDrySoil->setCursor(QCursor(Qt::PointingHandCursor));
        btnSimDrySoil->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #1a1608; color: #f59e0b; border: 1px solid #78350f; border-radius: 6px; font-size: 10px; font-weight: 700; padding: 4px; } QPushButton:hover { background-color: #78350f; color: white; }"));

        z1SimRow->addWidget(btnSimDrySoil);

        btnSimMoistSoil = new QPushButton(zone1Frame);
        btnSimMoistSoil->setObjectName("btnSimMoistSoil");
        btnSimMoistSoil->setCursor(QCursor(Qt::PointingHandCursor));
        btnSimMoistSoil->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #061f12; color: #34d399; border: 1px solid #15803d; border-radius: 6px; font-size: 10px; font-weight: 700; padding: 4px; } QPushButton:hover { background-color: #15803d; color: white; }"));

        z1SimRow->addWidget(btnSimMoistSoil);


        z1Layout->addLayout(z1SimRow);


        dashboardHLayout->addWidget(zone1Frame);

        z2Layout = new QVBoxLayout();
        z2Layout->setSpacing(8);
        z2Layout->setObjectName("z2Layout");
        z2MetricsRow = new QHBoxLayout();
        z2MetricsRow->setSpacing(8);
        z2MetricsRow->setObjectName("z2MetricsRow");
        cardSoil = new QFrame(DashboardPage);
        cardSoil->setObjectName("cardSoil");
        cardSoil->setStyleSheet(QString::fromUtf8("QFrame#cardSoil { background-color: #07190f; border: 1.5px solid #10b981; border-radius: 10px; padding: 6px; }"));
        csLayout = new QVBoxLayout(cardSoil);
        csLayout->setContentsMargins(4, 4, 4, 4);
        csLayout->setObjectName("csLayout");
        csTitle = new QLabel(cardSoil);
        csTitle->setObjectName("csTitle");
        csTitle->setStyleSheet(QString::fromUtf8("color: #34d399; font-size: 9px; font-weight: 800;"));

        csLayout->addWidget(csTitle);

        soilMoistureValueLabel = new QLabel(cardSoil);
        soilMoistureValueLabel->setObjectName("soilMoistureValueLabel");
        soilMoistureValueLabel->setStyleSheet(QString::fromUtf8("color: #ffffff; font-size: 17px; font-weight: 900;"));

        csLayout->addWidget(soilMoistureValueLabel);

        soilMoistureSubLabel = new QLabel(cardSoil);
        soilMoistureSubLabel->setObjectName("soilMoistureSubLabel");
        soilMoistureSubLabel->setStyleSheet(QString::fromUtf8("color: #a7f3d0; font-size: 9px; font-weight: 600;"));

        csLayout->addWidget(soilMoistureSubLabel);


        z2MetricsRow->addWidget(cardSoil);

        cardTemp = new QFrame(DashboardPage);
        cardTemp->setObjectName("cardTemp");
        cardTemp->setStyleSheet(QString::fromUtf8("QFrame#cardTemp { background-color: #07190f; border: 1.5px solid #1b4332; border-radius: 10px; padding: 6px; }"));
        ctLayout = new QVBoxLayout(cardTemp);
        ctLayout->setContentsMargins(4, 4, 4, 4);
        ctLayout->setObjectName("ctLayout");
        ctTitle = new QLabel(cardTemp);
        ctTitle->setObjectName("ctTitle");
        ctTitle->setStyleSheet(QString::fromUtf8("color: #f59e0b; font-size: 9px; font-weight: 800;"));

        ctLayout->addWidget(ctTitle);

        tempValueLabel = new QLabel(cardTemp);
        tempValueLabel->setObjectName("tempValueLabel");
        tempValueLabel->setStyleSheet(QString::fromUtf8("color: #ffffff; font-size: 17px; font-weight: 900;"));

        ctLayout->addWidget(tempValueLabel);

        tempSubLabel = new QLabel(cardTemp);
        tempSubLabel->setObjectName("tempSubLabel");
        tempSubLabel->setStyleSheet(QString::fromUtf8("color: #a7f3d0; font-size: 9px; font-weight: 600;"));

        ctLayout->addWidget(tempSubLabel);


        z2MetricsRow->addWidget(cardTemp);

        cardHum = new QFrame(DashboardPage);
        cardHum->setObjectName("cardHum");
        cardHum->setStyleSheet(QString::fromUtf8("QFrame#cardHum { background-color: #07190f; border: 1.5px solid #1b4332; border-radius: 10px; padding: 6px; }"));
        chLayout = new QVBoxLayout(cardHum);
        chLayout->setContentsMargins(4, 4, 4, 4);
        chLayout->setObjectName("chLayout");
        chTitle = new QLabel(cardHum);
        chTitle->setObjectName("chTitle");
        chTitle->setStyleSheet(QString::fromUtf8("color: #38bdf8; font-size: 9px; font-weight: 800;"));

        chLayout->addWidget(chTitle);

        humValueLabel = new QLabel(cardHum);
        humValueLabel->setObjectName("humValueLabel");
        humValueLabel->setStyleSheet(QString::fromUtf8("color: #ffffff; font-size: 17px; font-weight: 900;"));

        chLayout->addWidget(humValueLabel);

        humSubLabel = new QLabel(cardHum);
        humSubLabel->setObjectName("humSubLabel");
        humSubLabel->setStyleSheet(QString::fromUtf8("color: #a7f3d0; font-size: 9px; font-weight: 600;"));

        chLayout->addWidget(humSubLabel);


        z2MetricsRow->addWidget(cardHum);


        z2Layout->addLayout(z2MetricsRow);

        chartCard = new QFrame(DashboardPage);
        chartCard->setObjectName("chartCard");
        chartCard->setStyleSheet(QString::fromUtf8("QFrame#chartCard { background-color: #06150c; border: 1.5px solid #1b4332; border-radius: 12px; }"));
        chartContainerLayout = new QVBoxLayout(chartCard);
        chartContainerLayout->setObjectName("chartContainerLayout");
        chartContainerLayout->setContentsMargins(6, 4, 6, 4);

        z2Layout->addWidget(chartCard);


        dashboardHLayout->addLayout(z2Layout);

        zone3Frame = new QFrame(DashboardPage);
        zone3Frame->setObjectName("zone3Frame");
        zone3Frame->setMinimumSize(QSize(210, 0));
        zone3Frame->setMaximumSize(QSize(220, 16777215));
        zone3Frame->setStyleSheet(QString::fromUtf8("QFrame#zone3Frame { background-color: #06150c; border: 1.5px solid #1b4332; border-radius: 14px; }"));
        z3Layout = new QVBoxLayout(zone3Frame);
        z3Layout->setSpacing(8);
        z3Layout->setObjectName("z3Layout");
        z3Layout->setContentsMargins(10, 10, 10, 10);
        z3Title = new QLabel(zone3Frame);
        z3Title->setObjectName("z3Title");
        z3Title->setStyleSheet(QString::fromUtf8("color: #38bdf8; font-size: 11px; font-weight: 800;"));

        z3Layout->addWidget(z3Title);

        btnTogglePumpMaster = new QPushButton(zone3Frame);
        btnTogglePumpMaster->setObjectName("btnTogglePumpMaster");
        btnTogglePumpMaster->setMinimumSize(QSize(0, 52));
        btnTogglePumpMaster->setCursor(QCursor(Qt::PointingHandCursor));
        btnTogglePumpMaster->setCheckable(true);
        btnTogglePumpMaster->setStyleSheet(QString::fromUtf8("QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #059669, stop:1 #10b981); color: white; border: none; border-radius: 12px; font-size: 12px; font-weight: 900; } QPushButton:hover { background: #10b981; } QPushButton:checked { background: #ef4444; }"));

        z3Layout->addWidget(btnTogglePumpMaster);

        quickTimerRow = new QHBoxLayout();
        quickTimerRow->setSpacing(4);
        quickTimerRow->setObjectName("quickTimerRow");
        btnTimer30s = new QPushButton(zone3Frame);
        btnTimer30s->setObjectName("btnTimer30s");
        btnTimer30s->setCursor(QCursor(Qt::PointingHandCursor));
        btnTimer30s->setStyleSheet(QString::fromUtf8("QPushButton { background: #0c2618; color: #a7f3d0; border: 1px solid #1b4332; border-radius: 6px; font-size: 10px; font-weight: 700; padding: 5px; } QPushButton:hover { background: #1b4332; color: white; }"));

        quickTimerRow->addWidget(btnTimer30s);

        btnTimer1m = new QPushButton(zone3Frame);
        btnTimer1m->setObjectName("btnTimer1m");
        btnTimer1m->setCursor(QCursor(Qt::PointingHandCursor));
        btnTimer1m->setStyleSheet(QString::fromUtf8("QPushButton { background: #0c2618; color: #a7f3d0; border: 1px solid #1b4332; border-radius: 6px; font-size: 10px; font-weight: 700; padding: 5px; } QPushButton:hover { background: #1b4332; color: white; }"));

        quickTimerRow->addWidget(btnTimer1m);

        btnTimer3m = new QPushButton(zone3Frame);
        btnTimer3m->setObjectName("btnTimer3m");
        btnTimer3m->setCursor(QCursor(Qt::PointingHandCursor));
        btnTimer3m->setStyleSheet(QString::fromUtf8("QPushButton { background: #0c2618; color: #a7f3d0; border: 1px solid #1b4332; border-radius: 6px; font-size: 10px; font-weight: 700; padding: 5px; } QPushButton:hover { background: #1b4332; color: white; }"));

        quickTimerRow->addWidget(btnTimer3m);


        z3Layout->addLayout(quickTimerRow);

        btnAutoModeCapsule = new QPushButton(zone3Frame);
        btnAutoModeCapsule->setObjectName("btnAutoModeCapsule");
        btnAutoModeCapsule->setMinimumSize(QSize(0, 34));
        btnAutoModeCapsule->setCursor(QCursor(Qt::PointingHandCursor));
        btnAutoModeCapsule->setCheckable(true);
        btnAutoModeCapsule->setChecked(true);
        btnAutoModeCapsule->setStyleSheet(QString::fromUtf8("QPushButton { background: #064e3b; color: #34d399; border: 1px solid #059669; border-radius: 8px; font-size: 10px; font-weight: 800; } QPushButton:checked { background: #064e3b; color: #34d399; } QPushButton:!checked { background: #1e293b; color: #94a3b8; border-color: #334155; }"));

        z3Layout->addWidget(btnAutoModeCapsule);

        z3Spacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

        z3Layout->addItem(z3Spacer);

        tankStatusBox = new QFrame(zone3Frame);
        tankStatusBox->setObjectName("tankStatusBox");
        tankStatusBox->setStyleSheet(QString::fromUtf8("background: #07190f; border: 1px solid #1b4332; border-radius: 8px; padding: 6px;"));
        tsbLayout = new QVBoxLayout(tankStatusBox);
        tsbLayout->setContentsMargins(2, 2, 2, 2);
        tsbLayout->setObjectName("tsbLayout");
        tankLevelLabel = new QLabel(tankStatusBox);
        tankLevelLabel->setObjectName("tankLevelLabel");
        tankLevelLabel->setStyleSheet(QString::fromUtf8("color: #38bdf8; font-size: 11px; font-weight: 800;"));

        tsbLayout->addWidget(tankLevelLabel);

        tankSubLabel = new QLabel(tankStatusBox);
        tankSubLabel->setObjectName("tankSubLabel");
        tankSubLabel->setStyleSheet(QString::fromUtf8("color: #a7f3d0; font-size: 9px;"));

        tsbLayout->addWidget(tankSubLabel);


        z3Layout->addWidget(tankStatusBox);


        dashboardHLayout->addWidget(zone3Frame);


        retranslateUi(DashboardPage);

        QMetaObject::connectSlotsByName(DashboardPage);
    } // setupUi

    void retranslateUi(QWidget *DashboardPage)
    {
        z1Title->setText(QCoreApplication::translate("DashboardPage", "\360\237\214\261 M\303\224 H\303\214NH V\306\257\341\273\234N \306\257\306\240M", nullptr));
        btnSimDrySoil->setText(QCoreApplication::translate("DashboardPage", "\360\237\215\202 Test \304\220\341\272\245t Kh\303\264", nullptr));
        btnSimMoistSoil->setText(QCoreApplication::translate("DashboardPage", "\360\237\214\261 Test \304\220\341\272\245t \341\272\250m", nullptr));
        csTitle->setText(QCoreApplication::translate("DashboardPage", "\360\237\214\276 \304\220\341\273\230 \341\272\250M \304\220\341\272\244T", nullptr));
        soilMoistureValueLabel->setText(QCoreApplication::translate("DashboardPage", "55.0%", nullptr));
        soilMoistureSubLabel->setText(QCoreApplication::translate("DashboardPage", "\360\237\214\261 \304\220\341\272\241t chu\341\272\251n", nullptr));
        ctTitle->setText(QCoreApplication::translate("DashboardPage", "\360\237\214\241\357\270\217 NHI\341\273\206T \304\220\341\273\230 KH\303\215", nullptr));
        tempValueLabel->setText(QCoreApplication::translate("DashboardPage", "27.5 \302\260C", nullptr));
        tempSubLabel->setText(QCoreApplication::translate("DashboardPage", "DHT11 M\303\241t m\341\272\273", nullptr));
        chTitle->setText(QCoreApplication::translate("DashboardPage", "\360\237\222\247 \304\220\341\273\230 \341\272\250M KH\303\215", nullptr));
        humValueLabel->setText(QCoreApplication::translate("DashboardPage", "65.0 %RH", nullptr));
        humSubLabel->setText(QCoreApplication::translate("DashboardPage", "\304\220\341\273\231 \341\272\251m t\303\241n: T\341\273\221t", nullptr));
        z3Title->setText(QCoreApplication::translate("DashboardPage", "\360\237\232\260 TR\341\272\240M B\306\240M T\306\257\341\273\232I", nullptr));
        btnTogglePumpMaster->setText(QCoreApplication::translate("DashboardPage", "\360\237\222\246 B\341\272\254T B\306\240M T\306\257\341\273\232I", nullptr));
        btnTimer30s->setText(QCoreApplication::translate("DashboardPage", "\342\217\261 30s", nullptr));
        btnTimer1m->setText(QCoreApplication::translate("DashboardPage", "\342\217\261 1 ph\303\272t", nullptr));
        btnTimer3m->setText(QCoreApplication::translate("DashboardPage", "\342\217\261 3 ph\303\272t", nullptr));
        btnAutoModeCapsule->setText(QCoreApplication::translate("DashboardPage", "\342\232\241 T\341\273\260 \304\220\341\273\230NG THEO \304\220\341\273\230 \341\272\250M: B\341\272\254T", nullptr));
        tankLevelLabel->setText(QCoreApplication::translate("DashboardPage", "\360\237\232\260 B\341\273\223n n\306\260\341\273\233c: 85%", nullptr));
        tankSubLabel->setText(QCoreApplication::translate("DashboardPage", "\304\220\303\243 t\306\260\341\273\233i: 3 l\341\272\247n (12.5 L)", nullptr));
        (void)DashboardPage;
    } // retranslateUi

};

namespace Ui {
    class DashboardPage: public Ui_DashboardPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DASHBOARDPAGE_H
