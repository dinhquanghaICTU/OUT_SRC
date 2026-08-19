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
    QVBoxLayout *mainLayout;
    QFrame *topHeaderBar;
    QHBoxLayout *headerLayout;
    QLabel *appLogoText;
    QSpacerItem *headerLeftSpacer;
    QPushButton *dashboardButton;
    QPushButton *devicesButton;
    QPushButton *historyButton;
    QPushButton *usersButton;
    QSpacerItem *headerRightSpacer;
    QLabel *currentUserBadge;
    QPushButton *logoutButton;
    QStackedWidget *pages;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 480);
        MainWindow->setMinimumSize(QSize(320, 240));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setSpacing(0);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(0, 0, 0, 0);
        topHeaderBar = new QFrame(centralWidget);
        topHeaderBar->setObjectName("topHeaderBar");
        topHeaderBar->setMinimumSize(QSize(0, 44));
        topHeaderBar->setMaximumSize(QSize(16777215, 44));
        topHeaderBar->setStyleSheet(QString::fromUtf8("QFrame#topHeaderBar { background-color: #171333; border-bottom: 1.5px solid #2b235c; } QLabel#appLogoText { color: #38bdf8; font-size: 13px; font-weight: 900; } QPushButton#topNavBtn { background: transparent; color: #94a3b8; border: none; font-size: 11px; font-weight: 700; padding: 4px 10px; border-radius: 6px; } QPushButton#topNavBtn:hover { color: #ffffff; background: #221c4b; } QPushButton#topNavBtn:checked { color: #ffffff; background: #2a225e; border-bottom: 2px solid #38bdf8; } QPushButton#topLogoutBtn { background: #3b1424; color: #f87171; border: 1px solid #7f1d1d; border-radius: 5px; font-size: 10px; font-weight: 800; padding: 3px 8px; } QPushButton#topLogoutBtn:hover { background: #dc2626; color: #ffffff; }"));
        headerLayout = new QHBoxLayout(topHeaderBar);
        headerLayout->setSpacing(8);
        headerLayout->setObjectName("headerLayout");
        headerLayout->setContentsMargins(12, 4, 12, 4);
        appLogoText = new QLabel(topHeaderBar);
        appLogoText->setObjectName("appLogoText");

        headerLayout->addWidget(appLogoText);

        headerLeftSpacer = new QSpacerItem(16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        headerLayout->addItem(headerLeftSpacer);

        dashboardButton = new QPushButton(topHeaderBar);
        dashboardButton->setObjectName("dashboardButton");
        dashboardButton->setCheckable(true);
        dashboardButton->setChecked(true);
        dashboardButton->setAutoExclusive(true);
        dashboardButton->setCursor(QCursor(Qt::PointingHandCursor));

        headerLayout->addWidget(dashboardButton);

        devicesButton = new QPushButton(topHeaderBar);
        devicesButton->setObjectName("devicesButton");
        devicesButton->setCheckable(true);
        devicesButton->setAutoExclusive(true);
        devicesButton->setCursor(QCursor(Qt::PointingHandCursor));

        headerLayout->addWidget(devicesButton);

        historyButton = new QPushButton(topHeaderBar);
        historyButton->setObjectName("historyButton");
        historyButton->setCheckable(true);
        historyButton->setAutoExclusive(true);
        historyButton->setCursor(QCursor(Qt::PointingHandCursor));

        headerLayout->addWidget(historyButton);

        usersButton = new QPushButton(topHeaderBar);
        usersButton->setObjectName("usersButton");
        usersButton->setCheckable(true);
        usersButton->setAutoExclusive(true);
        usersButton->setCursor(QCursor(Qt::PointingHandCursor));

        headerLayout->addWidget(usersButton);

        headerRightSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        headerLayout->addItem(headerRightSpacer);

        currentUserBadge = new QLabel(topHeaderBar);
        currentUserBadge->setObjectName("currentUserBadge");
        currentUserBadge->setStyleSheet(QString::fromUtf8("color: #fbbf24; font-size: 10px; font-weight: 800; background: rgba(251, 191, 36, 0.12); border-radius: 4px; padding: 2px 6px;"));

        headerLayout->addWidget(currentUserBadge);

        logoutButton = new QPushButton(topHeaderBar);
        logoutButton->setObjectName("logoutButton");
        logoutButton->setCursor(QCursor(Qt::PointingHandCursor));

        headerLayout->addWidget(logoutButton);


        mainLayout->addWidget(topHeaderBar);

        pages = new QStackedWidget(centralWidget);
        pages->setObjectName("pages");

        mainLayout->addWidget(pages);

        MainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName("statusBar");
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "THEANH IoT Hub - Power & Environment Monitor", nullptr));
        appLogoText->setText(QCoreApplication::translate("MainWindow", "\342\232\241 THEANH IOT", nullptr));
        dashboardButton->setObjectName(QCoreApplication::translate("MainWindow", "topNavBtn", nullptr));
        dashboardButton->setText(QCoreApplication::translate("MainWindow", "\342\214\202 Trang ch\341\273\247", nullptr));
        devicesButton->setObjectName(QCoreApplication::translate("MainWindow", "topNavBtn", nullptr));
        devicesButton->setText(QCoreApplication::translate("MainWindow", "\360\237\223\241 Thi\341\272\277t b\341\273\213", nullptr));
        historyButton->setObjectName(QCoreApplication::translate("MainWindow", "topNavBtn", nullptr));
        historyButton->setText(QCoreApplication::translate("MainWindow", "\360\237\223\210 L\341\273\213ch s\341\273\255", nullptr));
        usersButton->setObjectName(QCoreApplication::translate("MainWindow", "topNavBtn", nullptr));
        usersButton->setText(QCoreApplication::translate("MainWindow", "\342\231\237 T\303\240i kho\341\272\243n", nullptr));
        currentUserBadge->setText(QCoreApplication::translate("MainWindow", "\360\237\221\221 Admin", nullptr));
        logoutButton->setObjectName(QCoreApplication::translate("MainWindow", "topLogoutBtn", nullptr));
        logoutButton->setText(QCoreApplication::translate("MainWindow", "\304\220\304\203ng xu\341\272\245t \342\206\252", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
