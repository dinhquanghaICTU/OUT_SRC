/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *rootLayout;
    QFrame *topHeaderRibbon;
    QHBoxLayout *headerHLayout;
    QLabel *appBrandLabel;
    QLabel *doorStatusPill;
    QLabel *modePill;
    QSpacerItem *headerSpacer;
    QLabel *clockLabel;
    QLabel *currentUserBadge;
    QPushButton *logoutButton;
    QHBoxLayout *bodyHLayout;
    QFrame *leftNavRail;
    QVBoxLayout *navRailLayout;
    QPushButton *btnNavDashboard;
    QPushButton *btnNavDevices;
    QPushButton *btnNavHistory;
    QPushButton *btnNavAlerts;
    QPushButton *btnNavUsers;
    QSpacerItem *railSpacer;
    QStackedWidget *pages;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 480);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        rootLayout = new QVBoxLayout(centralWidget);
        rootLayout->setSpacing(0);
        rootLayout->setObjectName("rootLayout");
        rootLayout->setContentsMargins(0, 0, 0, 0);
        topHeaderRibbon = new QFrame(centralWidget);
        topHeaderRibbon->setObjectName("topHeaderRibbon");
        topHeaderRibbon->setMinimumSize(QSize(0, 44));
        topHeaderRibbon->setMaximumSize(QSize(16777215, 44));
        topHeaderRibbon->setStyleSheet(QString::fromUtf8("QFrame#topHeaderRibbon { background-color: #0b0f19; border-bottom: 1.5px solid #1e293b; }"));
        headerHLayout = new QHBoxLayout(topHeaderRibbon);
        headerHLayout->setSpacing(10);
        headerHLayout->setObjectName("headerHLayout");
        headerHLayout->setContentsMargins(12, 4, 12, 4);
        appBrandLabel = new QLabel(topHeaderRibbon);
        appBrandLabel->setObjectName("appBrandLabel");
        appBrandLabel->setStyleSheet(QString::fromUtf8("color: #38bdf8; font-size: 13px; font-weight: 900; letter-spacing: 0.5px;"));

        headerHLayout->addWidget(appBrandLabel);

        doorStatusPill = new QLabel(topHeaderRibbon);
        doorStatusPill->setObjectName("doorStatusPill");
        doorStatusPill->setStyleSheet(QString::fromUtf8("color: #94a3b8; background: rgba(148, 163, 184, 0.12); border: 1px solid #334155; border-radius: 6px; font-size: 10px; font-weight: 800; padding: 3px 8px;"));

        headerHLayout->addWidget(doorStatusPill);

        modePill = new QLabel(topHeaderRibbon);
        modePill->setObjectName("modePill");
        modePill->setStyleSheet(QString::fromUtf8("color: #10b981; background: rgba(16, 185, 129, 0.12); border: 1px solid #059669; border-radius: 6px; font-size: 10px; font-weight: 800; padding: 3px 8px;"));

        headerHLayout->addWidget(modePill);

        headerSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        headerHLayout->addItem(headerSpacer);

        clockLabel = new QLabel(topHeaderRibbon);
        clockLabel->setObjectName("clockLabel");
        clockLabel->setStyleSheet(QString::fromUtf8("color: #cbd5e1; font-size: 11px; font-weight: 700;"));

        headerHLayout->addWidget(clockLabel);

        currentUserBadge = new QLabel(topHeaderRibbon);
        currentUserBadge->setObjectName("currentUserBadge");
        currentUserBadge->setStyleSheet(QString::fromUtf8("color: #f59e0b; background: rgba(245, 158, 11, 0.12); border: 1px solid #d97706; border-radius: 6px; font-size: 10px; font-weight: 800; padding: 3px 8px;"));

        headerHLayout->addWidget(currentUserBadge);

        logoutButton = new QPushButton(topHeaderRibbon);
        logoutButton->setObjectName("logoutButton");
        logoutButton->setCursor(QCursor(Qt::PointingHandCursor));
        logoutButton->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #3b1424; color: #f87171; border: 1px solid #7f1d1d; border-radius: 6px; font-size: 10px; font-weight: 800; padding: 4px 10px; } QPushButton:hover { background-color: #dc2626; color: #ffffff; }"));

        headerHLayout->addWidget(logoutButton);


        rootLayout->addWidget(topHeaderRibbon);

        bodyHLayout = new QHBoxLayout();
        bodyHLayout->setSpacing(0);
        bodyHLayout->setObjectName("bodyHLayout");
        leftNavRail = new QFrame(centralWidget);
        leftNavRail->setObjectName("leftNavRail");
        leftNavRail->setMinimumSize(QSize(130, 0));
        leftNavRail->setMaximumSize(QSize(140, 16777215));
        leftNavRail->setStyleSheet(QString::fromUtf8("QFrame#leftNavRail { background-color: #0b0f19; border-right: 1.5px solid #1e293b; }"));
        navRailLayout = new QVBoxLayout(leftNavRail);
        navRailLayout->setSpacing(6);
        navRailLayout->setObjectName("navRailLayout");
        navRailLayout->setContentsMargins(6, 8, 6, 8);
        btnNavDashboard = new QPushButton(leftNavRail);
        btnNavDashboard->setObjectName("btnNavDashboard");
        btnNavDashboard->setMinimumSize(QSize(0, 38));
        btnNavDashboard->setCursor(QCursor(Qt::PointingHandCursor));
        btnNavDashboard->setCheckable(true);
        btnNavDashboard->setChecked(true);
        btnNavDashboard->setAutoExclusive(true);
        btnNavDashboard->setStyleSheet(QString::fromUtf8("QPushButton { background: transparent; color: #94a3b8; border: none; border-radius: 8px; font-size: 11px; font-weight: 700; text-align: left; padding-left: 12px; } QPushButton:hover { background: #1e293b; color: white; } QPushButton:checked { background: #0284c7; color: white; }"));

        navRailLayout->addWidget(btnNavDashboard);

        btnNavDevices = new QPushButton(leftNavRail);
        btnNavDevices->setObjectName("btnNavDevices");
        btnNavDevices->setMinimumSize(QSize(0, 38));
        btnNavDevices->setCursor(QCursor(Qt::PointingHandCursor));
        btnNavDevices->setCheckable(true);
        btnNavDevices->setAutoExclusive(true);
        btnNavDevices->setStyleSheet(QString::fromUtf8("QPushButton { background: transparent; color: #94a3b8; border: none; border-radius: 8px; font-size: 11px; font-weight: 700; text-align: left; padding-left: 12px; } QPushButton:hover { background: #1e293b; color: white; } QPushButton:checked { background: #0284c7; color: white; }"));

        navRailLayout->addWidget(btnNavDevices);

        btnNavHistory = new QPushButton(leftNavRail);
        btnNavHistory->setObjectName("btnNavHistory");
        btnNavHistory->setMinimumSize(QSize(0, 38));
        btnNavHistory->setCursor(QCursor(Qt::PointingHandCursor));
        btnNavHistory->setCheckable(true);
        btnNavHistory->setAutoExclusive(true);
        btnNavHistory->setStyleSheet(QString::fromUtf8("QPushButton { background: transparent; color: #94a3b8; border: none; border-radius: 8px; font-size: 11px; font-weight: 700; text-align: left; padding-left: 12px; } QPushButton:hover { background: #1e293b; color: white; } QPushButton:checked { background: #0284c7; color: white; }"));

        navRailLayout->addWidget(btnNavHistory);

        btnNavAlerts = new QPushButton(leftNavRail);
        btnNavAlerts->setObjectName("btnNavAlerts");
        btnNavAlerts->setMinimumSize(QSize(0, 38));
        btnNavAlerts->setCursor(QCursor(Qt::PointingHandCursor));
        btnNavAlerts->setCheckable(true);
        btnNavAlerts->setAutoExclusive(true);
        btnNavAlerts->setStyleSheet(QString::fromUtf8("QPushButton { background: transparent; color: #94a3b8; border: none; border-radius: 8px; font-size: 11px; font-weight: 700; text-align: left; padding-left: 12px; } QPushButton:hover { background: #1e293b; color: white; } QPushButton:checked { background: #0284c7; color: white; }"));

        navRailLayout->addWidget(btnNavAlerts);

        btnNavUsers = new QPushButton(leftNavRail);
        btnNavUsers->setObjectName("btnNavUsers");
        btnNavUsers->setMinimumSize(QSize(0, 38));
        btnNavUsers->setCursor(QCursor(Qt::PointingHandCursor));
        btnNavUsers->setCheckable(true);
        btnNavUsers->setAutoExclusive(true);
        btnNavUsers->setStyleSheet(QString::fromUtf8("QPushButton { background: transparent; color: #94a3b8; border: none; border-radius: 8px; font-size: 11px; font-weight: 700; text-align: left; padding-left: 12px; } QPushButton:hover { background: #1e293b; color: white; } QPushButton:checked { background: #0284c7; color: white; }"));

        navRailLayout->addWidget(btnNavUsers);

        railSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        navRailLayout->addItem(railSpacer);


        bodyHLayout->addWidget(leftNavRail);

        pages = new QStackedWidget(centralWidget);
        pages->setObjectName("pages");

        bodyHLayout->addWidget(pages);


        rootLayout->addLayout(bodyHLayout);

        MainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName("statusBar");
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Manh Quang Smart Door Control Hub", nullptr));
        appBrandLabel->setText(QCoreApplication::translate("MainWindow", "\360\237\232\252 MANH QUANG SMART DOOR", nullptr));
        doorStatusPill->setText(QCoreApplication::translate("MainWindow", "\342\232\252 C\341\273\254A \304\220\303\223NG", nullptr));
        modePill->setText(QCoreApplication::translate("MainWindow", "\342\232\241 AUTO PIR", nullptr));
        clockLabel->setText(QCoreApplication::translate("MainWindow", "--:--:--", nullptr));
        currentUserBadge->setText(QCoreApplication::translate("MainWindow", "\360\237\221\244 Admin", nullptr));
        logoutButton->setText(QCoreApplication::translate("MainWindow", "\304\220\304\203ng xu\341\272\245t \342\206\252", nullptr));
        btnNavDashboard->setText(QCoreApplication::translate("MainWindow", "\360\237\223\212 B\341\272\243ng \304\220K", nullptr));
        btnNavDevices->setText(QCoreApplication::translate("MainWindow", "\360\237\223\241 Thi\341\272\277t B\341\273\213", nullptr));
        btnNavHistory->setText(QCoreApplication::translate("MainWindow", "\360\237\223\234 L\341\273\213ch S\341\273\255", nullptr));
        btnNavAlerts->setText(QCoreApplication::translate("MainWindow", "\342\232\240\357\270\217 C\341\272\243nh B\303\241o", nullptr));
        btnNavUsers->setText(QCoreApplication::translate("MainWindow", "\360\237\221\245 T\303\240i Kho\341\272\243n", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
