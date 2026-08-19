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
    QFrame *topIslandHeader;
    QHBoxLayout *headerHLayout;
    QLabel *appBrandLabel;
    QSpacerItem *headerLeftSpacer;
    QFrame *segmentedNavCapsule;
    QHBoxLayout *navSegLayout;
    QPushButton *btnNavDashboard;
    QPushButton *btnNavDevices;
    QPushButton *btnNavHistory;
    QPushButton *btnNavAlerts;
    QPushButton *btnNavUsers;
    QSpacerItem *headerRightSpacer;
    QLabel *clockLabel;
    QLabel *currentUserBadge;
    QPushButton *logoutButton;
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
        topIslandHeader = new QFrame(centralWidget);
        topIslandHeader->setObjectName("topIslandHeader");
        topIslandHeader->setMinimumSize(QSize(0, 50));
        topIslandHeader->setMaximumSize(QSize(16777215, 50));
        topIslandHeader->setStyleSheet(QString::fromUtf8("QFrame#topIslandHeader { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #05140b, stop:0.5 #0c2617, stop:1 #05140b); border-bottom: 2px solid #10b981; }"));
        headerHLayout = new QHBoxLayout(topIslandHeader);
        headerHLayout->setSpacing(8);
        headerHLayout->setObjectName("headerHLayout");
        headerHLayout->setContentsMargins(12, 4, 12, 4);
        appBrandLabel = new QLabel(topIslandHeader);
        appBrandLabel->setObjectName("appBrandLabel");
        appBrandLabel->setStyleSheet(QString::fromUtf8("color: #34d399; font-size: 13px; font-weight: 900; letter-spacing: 0.8px;"));

        headerHLayout->addWidget(appBrandLabel);

        headerLeftSpacer = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        headerHLayout->addItem(headerLeftSpacer);

        segmentedNavCapsule = new QFrame(topIslandHeader);
        segmentedNavCapsule->setObjectName("segmentedNavCapsule");
        segmentedNavCapsule->setStyleSheet(QString::fromUtf8("QFrame#segmentedNavCapsule { background-color: #05120a; border: 1.5px solid #1b4332; border-radius: 18px; }"));
        navSegLayout = new QHBoxLayout(segmentedNavCapsule);
        navSegLayout->setSpacing(4);
        navSegLayout->setObjectName("navSegLayout");
        navSegLayout->setContentsMargins(4, 2, 4, 2);
        btnNavDashboard = new QPushButton(segmentedNavCapsule);
        btnNavDashboard->setObjectName("btnNavDashboard");
        btnNavDashboard->setMinimumSize(QSize(95, 32));
        btnNavDashboard->setCursor(QCursor(Qt::PointingHandCursor));
        btnNavDashboard->setCheckable(true);
        btnNavDashboard->setChecked(true);
        btnNavDashboard->setAutoExclusive(true);
        btnNavDashboard->setStyleSheet(QString::fromUtf8("QPushButton { background: transparent; color: #a7f3d0; border: none; border-radius: 14px; font-size: 11px; font-weight: 700; padding: 4px 8px; } QPushButton:hover { background: #0c281a; color: white; } QPushButton:checked { background: #10b981; color: white; }"));

        navSegLayout->addWidget(btnNavDashboard);

        btnNavDevices = new QPushButton(segmentedNavCapsule);
        btnNavDevices->setObjectName("btnNavDevices");
        btnNavDevices->setMinimumSize(QSize(105, 32));
        btnNavDevices->setCursor(QCursor(Qt::PointingHandCursor));
        btnNavDevices->setCheckable(true);
        btnNavDevices->setAutoExclusive(true);
        btnNavDevices->setStyleSheet(QString::fromUtf8("QPushButton { background: transparent; color: #a7f3d0; border: none; border-radius: 14px; font-size: 11px; font-weight: 700; padding: 4px 8px; } QPushButton:hover { background: #0c281a; color: white; } QPushButton:checked { background: #10b981; color: white; }"));

        navSegLayout->addWidget(btnNavDevices);

        btnNavHistory = new QPushButton(segmentedNavCapsule);
        btnNavHistory->setObjectName("btnNavHistory");
        btnNavHistory->setMinimumSize(QSize(95, 32));
        btnNavHistory->setCursor(QCursor(Qt::PointingHandCursor));
        btnNavHistory->setCheckable(true);
        btnNavHistory->setAutoExclusive(true);
        btnNavHistory->setStyleSheet(QString::fromUtf8("QPushButton { background: transparent; color: #a7f3d0; border: none; border-radius: 14px; font-size: 11px; font-weight: 700; padding: 4px 8px; } QPushButton:hover { background: #0c281a; color: white; } QPushButton:checked { background: #10b981; color: white; }"));

        navSegLayout->addWidget(btnNavHistory);

        btnNavAlerts = new QPushButton(segmentedNavCapsule);
        btnNavAlerts->setObjectName("btnNavAlerts");
        btnNavAlerts->setMinimumSize(QSize(90, 32));
        btnNavAlerts->setCursor(QCursor(Qt::PointingHandCursor));
        btnNavAlerts->setCheckable(true);
        btnNavAlerts->setAutoExclusive(true);
        btnNavAlerts->setStyleSheet(QString::fromUtf8("QPushButton { background: transparent; color: #a7f3d0; border: none; border-radius: 14px; font-size: 11px; font-weight: 700; padding: 4px 8px; } QPushButton:hover { background: #0c281a; color: white; } QPushButton:checked { background: #10b981; color: white; }"));

        navSegLayout->addWidget(btnNavAlerts);

        btnNavUsers = new QPushButton(segmentedNavCapsule);
        btnNavUsers->setObjectName("btnNavUsers");
        btnNavUsers->setMinimumSize(QSize(90, 32));
        btnNavUsers->setCursor(QCursor(Qt::PointingHandCursor));
        btnNavUsers->setCheckable(true);
        btnNavUsers->setAutoExclusive(true);
        btnNavUsers->setStyleSheet(QString::fromUtf8("QPushButton { background: transparent; color: #a7f3d0; border: none; border-radius: 14px; font-size: 11px; font-weight: 700; padding: 4px 8px; } QPushButton:hover { background: #0c281a; color: white; } QPushButton:checked { background: #10b981; color: white; }"));

        navSegLayout->addWidget(btnNavUsers);


        headerHLayout->addWidget(segmentedNavCapsule);

        headerRightSpacer = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        headerHLayout->addItem(headerRightSpacer);

        clockLabel = new QLabel(topIslandHeader);
        clockLabel->setObjectName("clockLabel");
        clockLabel->setStyleSheet(QString::fromUtf8("color: #6ee7b7; font-size: 11px; font-weight: 700;"));

        headerHLayout->addWidget(clockLabel);

        currentUserBadge = new QLabel(topIslandHeader);
        currentUserBadge->setObjectName("currentUserBadge");
        currentUserBadge->setStyleSheet(QString::fromUtf8("color: #f59e0b; background: rgba(245, 158, 11, 0.15); border: 1px solid #d97706; border-radius: 10px; font-size: 10px; font-weight: 800; padding: 3px 8px;"));

        headerHLayout->addWidget(currentUserBadge);

        logoutButton = new QPushButton(topIslandHeader);
        logoutButton->setObjectName("logoutButton");
        logoutButton->setCursor(QCursor(Qt::PointingHandCursor));
        logoutButton->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #37151e; color: #f87171; border: 1px solid #7f1d1d; border-radius: 10px; font-size: 10px; font-weight: 800; padding: 4px 8px; } QPushButton:hover { background-color: #dc2626; color: #ffffff; }"));

        headerHLayout->addWidget(logoutButton);


        rootLayout->addWidget(topIslandHeader);

        pages = new QStackedWidget(centralWidget);
        pages->setObjectName("pages");

        rootLayout->addWidget(pages);

        MainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName("statusBar");
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Van Phong Agro-Pod Smart Controller", nullptr));
        appBrandLabel->setText(QCoreApplication::translate("MainWindow", "\360\237\214\277 VAN PHONG AGRI-POD", nullptr));
        btnNavDashboard->setText(QCoreApplication::translate("MainWindow", "\360\237\214\277 V\306\260\341\273\235n \306\257\306\241m", nullptr));
        btnNavDevices->setText(QCoreApplication::translate("MainWindow", "\360\237\232\260 B\306\241m & Ng\306\260\341\273\241ng", nullptr));
        btnNavHistory->setText(QCoreApplication::translate("MainWindow", "\360\237\223\210 L\341\273\213ch S\341\273\255", nullptr));
        btnNavAlerts->setText(QCoreApplication::translate("MainWindow", "\342\232\240\357\270\217 C\341\272\243nh B\303\241o", nullptr));
        btnNavUsers->setText(QCoreApplication::translate("MainWindow", "\360\237\221\245 T\303\240i Kho\341\272\243n", nullptr));
        clockLabel->setText(QCoreApplication::translate("MainWindow", "--:--:--", nullptr));
        currentUserBadge->setText(QCoreApplication::translate("MainWindow", "\360\237\221\221 Admin", nullptr));
        logoutButton->setText(QCoreApplication::translate("MainWindow", "Tho\303\241t \342\206\252", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
