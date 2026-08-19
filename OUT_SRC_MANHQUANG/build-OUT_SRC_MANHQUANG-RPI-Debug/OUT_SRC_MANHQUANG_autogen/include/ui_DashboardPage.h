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
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DashboardPage
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *topRowLayout;
    QFrame *doorVisualizerContainer;
    QVBoxLayout *doorVisLayout;
    QFrame *controlPanelCard;
    QVBoxLayout *ctrlLayout;
    QLabel *ctrlTitle;
    QHBoxLayout *actionRow1;
    QPushButton *btnOpenDoor;
    QPushButton *btnCloseDoor;
    QHBoxLayout *actionRow2;
    QPushButton *btnHoldOpen;
    QPushButton *btnStopEmergency;
    QHBoxLayout *actionRow3;
    QPushButton *btnSimMotion;
    QPushButton *btnSimObstacle;
    QHBoxLayout *bottomRowLayout;
    QFrame *sr602Card;
    QVBoxLayout *sr602Layout;
    QLabel *sr602Title;
    QLabel *sr602StatusLabel;
    QLabel *sr602TriggerCountLabel;
    QFrame *irCard;
    QVBoxLayout *irLayout;
    QLabel *irTitle;
    QLabel *irStatusLabel;
    QLabel *irSubLabel;
    QFrame *motorCard;
    QVBoxLayout *motorLayout;
    QLabel *motorTitle;
    QLabel *motorStepsLabel;
    QLabel *motorSpeedLabel;
    QFrame *trafficCard;
    QVBoxLayout *trafficLayout;
    QLabel *trafficTitle;
    QLabel *passageCountLabel;
    QLabel *trafficSubLabel;
    QFrame *chartCard;
    QVBoxLayout *chartContainerLayout;

    void setupUi(QWidget *DashboardPage)
    {
        if (DashboardPage->objectName().isEmpty())
            DashboardPage->setObjectName("DashboardPage");
        DashboardPage->resize(800, 480);
        mainLayout = new QVBoxLayout(DashboardPage);
        mainLayout->setSpacing(8);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(12, 8, 12, 8);
        topRowLayout = new QHBoxLayout();
        topRowLayout->setSpacing(10);
        topRowLayout->setObjectName("topRowLayout");
        doorVisualizerContainer = new QFrame(DashboardPage);
        doorVisualizerContainer->setObjectName("doorVisualizerContainer");
        doorVisualizerContainer->setMinimumSize(QSize(360, 180));
        doorVisLayout = new QVBoxLayout(doorVisualizerContainer);
        doorVisLayout->setContentsMargins(0, 0, 0, 0);
        doorVisLayout->setObjectName("doorVisLayout");

        topRowLayout->addWidget(doorVisualizerContainer);

        controlPanelCard = new QFrame(DashboardPage);
        controlPanelCard->setObjectName("controlPanelCard");
        controlPanelCard->setMinimumSize(QSize(240, 180));
        controlPanelCard->setStyleSheet(QString::fromUtf8("QFrame#controlPanelCard { background-color: #0f172a; border: 1.5px solid #1e293b; border-radius: 10px; }"));
        ctrlLayout = new QVBoxLayout(controlPanelCard);
        ctrlLayout->setSpacing(6);
        ctrlLayout->setObjectName("ctrlLayout");
        ctrlLayout->setContentsMargins(12, 10, 12, 10);
        ctrlTitle = new QLabel(controlPanelCard);
        ctrlTitle->setObjectName("ctrlTitle");
        ctrlTitle->setStyleSheet(QString::fromUtf8("color: #38bdf8; font-size: 11px; font-weight: 800;"));

        ctrlLayout->addWidget(ctrlTitle);

        actionRow1 = new QHBoxLayout();
        actionRow1->setSpacing(6);
        actionRow1->setObjectName("actionRow1");
        btnOpenDoor = new QPushButton(controlPanelCard);
        btnOpenDoor->setObjectName("btnOpenDoor");
        btnOpenDoor->setMinimumSize(QSize(0, 34));
        btnOpenDoor->setCursor(QCursor(Qt::PointingHandCursor));
        btnOpenDoor->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #059669; color: white; border: none; border-radius: 6px; font-size: 11px; font-weight: 700; } QPushButton:hover { background-color: #10b981; }"));

        actionRow1->addWidget(btnOpenDoor);

        btnCloseDoor = new QPushButton(controlPanelCard);
        btnCloseDoor->setObjectName("btnCloseDoor");
        btnCloseDoor->setMinimumSize(QSize(0, 34));
        btnCloseDoor->setCursor(QCursor(Qt::PointingHandCursor));
        btnCloseDoor->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #2563eb; color: white; border: none; border-radius: 6px; font-size: 11px; font-weight: 700; } QPushButton:hover { background-color: #3b82f6; }"));

        actionRow1->addWidget(btnCloseDoor);


        ctrlLayout->addLayout(actionRow1);

        actionRow2 = new QHBoxLayout();
        actionRow2->setSpacing(6);
        actionRow2->setObjectName("actionRow2");
        btnHoldOpen = new QPushButton(controlPanelCard);
        btnHoldOpen->setObjectName("btnHoldOpen");
        btnHoldOpen->setMinimumSize(QSize(0, 32));
        btnHoldOpen->setCursor(QCursor(Qt::PointingHandCursor));
        btnHoldOpen->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #d97706; color: white; border: none; border-radius: 6px; font-size: 10px; font-weight: 700; } QPushButton:hover { background-color: #f59e0b; }"));

        actionRow2->addWidget(btnHoldOpen);

        btnStopEmergency = new QPushButton(controlPanelCard);
        btnStopEmergency->setObjectName("btnStopEmergency");
        btnStopEmergency->setMinimumSize(QSize(0, 32));
        btnStopEmergency->setCursor(QCursor(Qt::PointingHandCursor));
        btnStopEmergency->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #dc2626; color: white; border: none; border-radius: 6px; font-size: 10px; font-weight: 700; } QPushButton:hover { background-color: #ef4444; }"));

        actionRow2->addWidget(btnStopEmergency);


        ctrlLayout->addLayout(actionRow2);

        actionRow3 = new QHBoxLayout();
        actionRow3->setSpacing(6);
        actionRow3->setObjectName("actionRow3");
        btnSimMotion = new QPushButton(controlPanelCard);
        btnSimMotion->setObjectName("btnSimMotion");
        btnSimMotion->setCursor(QCursor(Qt::PointingHandCursor));
        btnSimMotion->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #1e293b; color: #10b981; border: 1px solid #334155; border-radius: 5px; font-size: 10px; font-weight: 600; padding: 4px; } QPushButton:hover { background-color: #334155; }"));

        actionRow3->addWidget(btnSimMotion);

        btnSimObstacle = new QPushButton(controlPanelCard);
        btnSimObstacle->setObjectName("btnSimObstacle");
        btnSimObstacle->setCursor(QCursor(Qt::PointingHandCursor));
        btnSimObstacle->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #1e293b; color: #f87171; border: 1px solid #334155; border-radius: 5px; font-size: 10px; font-weight: 600; padding: 4px; } QPushButton:hover { background-color: #334155; }"));

        actionRow3->addWidget(btnSimObstacle);


        ctrlLayout->addLayout(actionRow3);


        topRowLayout->addWidget(controlPanelCard);


        mainLayout->addLayout(topRowLayout);

        bottomRowLayout = new QHBoxLayout();
        bottomRowLayout->setSpacing(8);
        bottomRowLayout->setObjectName("bottomRowLayout");
        sr602Card = new QFrame(DashboardPage);
        sr602Card->setObjectName("sr602Card");
        sr602Card->setStyleSheet(QString::fromUtf8("QFrame#sr602Card { background-color: #0f172a; border: 1.5px solid #1e293b; border-radius: 10px; padding: 6px; }"));
        sr602Layout = new QVBoxLayout(sr602Card);
        sr602Layout->setSpacing(4);
        sr602Layout->setContentsMargins(6, 6, 6, 6);
        sr602Layout->setObjectName("sr602Layout");
        sr602Title = new QLabel(sr602Card);
        sr602Title->setObjectName("sr602Title");
        sr602Title->setStyleSheet(QString::fromUtf8("color: #10b981; font-size: 10px; font-weight: 800;"));

        sr602Layout->addWidget(sr602Title);

        sr602StatusLabel = new QLabel(sr602Card);
        sr602StatusLabel->setObjectName("sr602StatusLabel");
        sr602StatusLabel->setStyleSheet(QString::fromUtf8("color: #ffffff; font-size: 14px; font-weight: 800;"));

        sr602Layout->addWidget(sr602StatusLabel);

        sr602TriggerCountLabel = new QLabel(sr602Card);
        sr602TriggerCountLabel->setObjectName("sr602TriggerCountLabel");
        sr602TriggerCountLabel->setStyleSheet(QString::fromUtf8("color: #94a3b8; font-size: 10px;"));

        sr602Layout->addWidget(sr602TriggerCountLabel);


        bottomRowLayout->addWidget(sr602Card);

        irCard = new QFrame(DashboardPage);
        irCard->setObjectName("irCard");
        irCard->setStyleSheet(QString::fromUtf8("QFrame#irCard { background-color: #0f172a; border: 1.5px solid #1e293b; border-radius: 10px; padding: 6px; }"));
        irLayout = new QVBoxLayout(irCard);
        irLayout->setSpacing(4);
        irLayout->setContentsMargins(6, 6, 6, 6);
        irLayout->setObjectName("irLayout");
        irTitle = new QLabel(irCard);
        irTitle->setObjectName("irTitle");
        irTitle->setStyleSheet(QString::fromUtf8("color: #38bdf8; font-size: 10px; font-weight: 800;"));

        irLayout->addWidget(irTitle);

        irStatusLabel = new QLabel(irCard);
        irStatusLabel->setObjectName("irStatusLabel");
        irStatusLabel->setStyleSheet(QString::fromUtf8("color: #10b981; font-size: 14px; font-weight: 800;"));

        irLayout->addWidget(irStatusLabel);

        irSubLabel = new QLabel(irCard);
        irSubLabel->setObjectName("irSubLabel");
        irSubLabel->setStyleSheet(QString::fromUtf8("color: #94a3b8; font-size: 10px;"));

        irLayout->addWidget(irSubLabel);


        bottomRowLayout->addWidget(irCard);

        motorCard = new QFrame(DashboardPage);
        motorCard->setObjectName("motorCard");
        motorCard->setStyleSheet(QString::fromUtf8("QFrame#motorCard { background-color: #0f172a; border: 1.5px solid #1e293b; border-radius: 10px; padding: 6px; }"));
        motorLayout = new QVBoxLayout(motorCard);
        motorLayout->setSpacing(4);
        motorLayout->setContentsMargins(6, 6, 6, 6);
        motorLayout->setObjectName("motorLayout");
        motorTitle = new QLabel(motorCard);
        motorTitle->setObjectName("motorTitle");
        motorTitle->setStyleSheet(QString::fromUtf8("color: #f59e0b; font-size: 10px; font-weight: 800;"));

        motorLayout->addWidget(motorTitle);

        motorStepsLabel = new QLabel(motorCard);
        motorStepsLabel->setObjectName("motorStepsLabel");
        motorStepsLabel->setStyleSheet(QString::fromUtf8("color: #ffffff; font-size: 14px; font-weight: 800;"));

        motorLayout->addWidget(motorStepsLabel);

        motorSpeedLabel = new QLabel(motorCard);
        motorSpeedLabel->setObjectName("motorSpeedLabel");
        motorSpeedLabel->setStyleSheet(QString::fromUtf8("color: #94a3b8; font-size: 10px;"));

        motorLayout->addWidget(motorSpeedLabel);


        bottomRowLayout->addWidget(motorCard);

        trafficCard = new QFrame(DashboardPage);
        trafficCard->setObjectName("trafficCard");
        trafficCard->setStyleSheet(QString::fromUtf8("QFrame#trafficCard { background-color: #0f172a; border: 1.5px solid #1e293b; border-radius: 10px; padding: 6px; }"));
        trafficLayout = new QVBoxLayout(trafficCard);
        trafficLayout->setSpacing(4);
        trafficLayout->setContentsMargins(6, 6, 6, 6);
        trafficLayout->setObjectName("trafficLayout");
        trafficTitle = new QLabel(trafficCard);
        trafficTitle->setObjectName("trafficTitle");
        trafficTitle->setStyleSheet(QString::fromUtf8("color: #a855f7; font-size: 10px; font-weight: 800;"));

        trafficLayout->addWidget(trafficTitle);

        passageCountLabel = new QLabel(trafficCard);
        passageCountLabel->setObjectName("passageCountLabel");
        passageCountLabel->setStyleSheet(QString::fromUtf8("color: #ffffff; font-size: 14px; font-weight: 800;"));

        trafficLayout->addWidget(passageCountLabel);

        trafficSubLabel = new QLabel(trafficCard);
        trafficSubLabel->setObjectName("trafficSubLabel");
        trafficSubLabel->setStyleSheet(QString::fromUtf8("color: #94a3b8; font-size: 10px;"));

        trafficLayout->addWidget(trafficSubLabel);


        bottomRowLayout->addWidget(trafficCard);


        mainLayout->addLayout(bottomRowLayout);

        chartCard = new QFrame(DashboardPage);
        chartCard->setObjectName("chartCard");
        chartCard->setMinimumSize(QSize(0, 120));
        chartCard->setStyleSheet(QString::fromUtf8("QFrame#chartCard { background-color: #0f172a; border: 1.5px solid #1e293b; border-radius: 10px; }"));
        chartContainerLayout = new QVBoxLayout(chartCard);
        chartContainerLayout->setContentsMargins(6, 6, 6, 6);
        chartContainerLayout->setObjectName("chartContainerLayout");

        mainLayout->addWidget(chartCard);


        retranslateUi(DashboardPage);

        QMetaObject::connectSlotsByName(DashboardPage);
    } // setupUi

    void retranslateUi(QWidget *DashboardPage)
    {
        ctrlTitle->setText(QCoreApplication::translate("DashboardPage", "\360\237\225\271\357\270\217 \304\220I\341\273\200U KHI\341\273\202N C\341\273\254A NHANH", nullptr));
        btnOpenDoor->setText(QCoreApplication::translate("DashboardPage", "\360\237\232\252 M\341\273\236 C\341\273\254A", nullptr));
        btnCloseDoor->setText(QCoreApplication::translate("DashboardPage", "\360\237\224\222 \304\220\303\223NG C\341\273\254A", nullptr));
        btnHoldOpen->setText(QCoreApplication::translate("DashboardPage", "\342\217\261 GI\341\273\256 M\341\273\236", nullptr));
        btnStopEmergency->setText(QCoreApplication::translate("DashboardPage", "\360\237\233\221 D\341\273\252NG KH\341\272\250N", nullptr));
        btnSimMotion->setText(QCoreApplication::translate("DashboardPage", "\360\237\232\266 Test SR602", nullptr));
        btnSimObstacle->setText(QCoreApplication::translate("DashboardPage", "\360\237\232\247 Test V\341\272\255t C\341\272\243n IR", nullptr));
        sr602Title->setText(QCoreApplication::translate("DashboardPage", "\342\232\241 C\341\272\242M BI\341\272\276N SR602", nullptr));
        sr602StatusLabel->setText(QCoreApplication::translate("DashboardPage", "Th\303\264ng tho\303\241ng", nullptr));
        sr602TriggerCountLabel->setText(QCoreApplication::translate("DashboardPage", "L\306\260\341\273\243t k\303\255ch ho\341\272\241t: 0", nullptr));
        irTitle->setText(QCoreApplication::translate("DashboardPage", "\360\237\232\250 CH\303\231M TIA IR (CH\341\273\220NG K\341\272\270T)", nullptr));
        irStatusLabel->setText(QCoreApplication::translate("DashboardPage", "Ch\303\271m tia an to\303\240n", nullptr));
        irSubLabel->setText(QCoreApplication::translate("DashboardPage", "T\341\273\261 \304\221\341\272\243o chi\341\273\201u: B\341\272\255t", nullptr));
        motorTitle->setText(QCoreApplication::translate("DashboardPage", "\342\232\231\357\270\217 \304\220\341\273\230NG C\306\240 B\306\257\341\273\232C", nullptr));
        motorStepsLabel->setText(QCoreApplication::translate("DashboardPage", "0 / 3200 steps", nullptr));
        motorSpeedLabel->setText(QCoreApplication::translate("DashboardPage", "T\341\273\221c \304\221\341\273\231: 0 RPM | STOP", nullptr));
        trafficTitle->setText(QCoreApplication::translate("DashboardPage", "\360\237\221\245 L\306\257U L\306\257\341\273\242NG RA V\303\200O", nullptr));
        passageCountLabel->setText(QCoreApplication::translate("DashboardPage", "0 l\306\260\341\273\243t", nullptr));
        trafficSubLabel->setText(QCoreApplication::translate("DashboardPage", "Nhi\341\273\207t \304\221\341\273\231: 28.5 \302\260C", nullptr));
        (void)DashboardPage;
    } // retranslateUi

};

namespace Ui {
    class DashboardPage: public Ui_DashboardPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DASHBOARDPAGE_H
