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
    QWidget *topNavBar;
    QHBoxLayout *topNavLayout;
    QLabel *navLogo;
    QPushButton *dashboardButton;
    QPushButton *devicesButton;
    QPushButton *historyButton;
    QPushButton *usersButton;
    QSpacerItem *navSpacer;
    QPushButton *logoutButton;
    QStackedWidget *pages;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 480);
        MainWindow->setMinimumSize(QSize(480, 320));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setSpacing(0);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(0, 0, 0, 0);
        topNavBar = new QWidget(centralWidget);
        topNavBar->setObjectName("topNavBar");
        topNavBar->setMinimumSize(QSize(0, 48));
        topNavBar->setMaximumSize(QSize(16777215, 48));
        topNavLayout = new QHBoxLayout(topNavBar);
        topNavLayout->setSpacing(0);
        topNavLayout->setObjectName("topNavLayout");
        topNavLayout->setContentsMargins(0, 0, 0, 0);
        navLogo = new QLabel(topNavBar);
        navLogo->setObjectName("navLogo");
        navLogo->setMinimumSize(QSize(140, 48));

        topNavLayout->addWidget(navLogo);

        dashboardButton = new QPushButton(topNavBar);
        dashboardButton->setObjectName("dashboardButton");
        dashboardButton->setCheckable(true);
        dashboardButton->setAutoExclusive(true);

        topNavLayout->addWidget(dashboardButton);

        devicesButton = new QPushButton(topNavBar);
        devicesButton->setObjectName("devicesButton");
        devicesButton->setCheckable(true);
        devicesButton->setAutoExclusive(true);

        topNavLayout->addWidget(devicesButton);

        historyButton = new QPushButton(topNavBar);
        historyButton->setObjectName("historyButton");
        historyButton->setCheckable(true);
        historyButton->setAutoExclusive(true);

        topNavLayout->addWidget(historyButton);

        usersButton = new QPushButton(topNavBar);
        usersButton->setObjectName("usersButton");
        usersButton->setCheckable(true);
        usersButton->setAutoExclusive(true);

        topNavLayout->addWidget(usersButton);

        navSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        topNavLayout->addItem(navSpacer);

        logoutButton = new QPushButton(topNavBar);
        logoutButton->setObjectName("logoutButton");

        topNavLayout->addWidget(logoutButton);


        mainLayout->addWidget(topNavBar);

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
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "SON Environmental Monitor", nullptr));
        navLogo->setText(QCoreApplication::translate("MainWindow", "\342\254\241 SON MONITOR", nullptr));
        dashboardButton->setObjectName(QCoreApplication::translate("MainWindow", "navBtn", nullptr));
        dashboardButton->setText(QCoreApplication::translate("MainWindow", "\342\214\202  Trang ch\341\273\247", nullptr));
#if QT_CONFIG(tooltip)
        dashboardButton->setToolTip(QCoreApplication::translate("MainWindow", "Trang ch\341\273\247", nullptr));
#endif // QT_CONFIG(tooltip)
        devicesButton->setObjectName(QCoreApplication::translate("MainWindow", "navBtn", nullptr));
        devicesButton->setText(QCoreApplication::translate("MainWindow", "\342\227\206  Thi\341\272\277t b\341\273\213", nullptr));
#if QT_CONFIG(tooltip)
        devicesButton->setToolTip(QCoreApplication::translate("MainWindow", "Qu\341\272\243n l\303\275 thi\341\272\277t b\341\273\213", nullptr));
#endif // QT_CONFIG(tooltip)
        historyButton->setObjectName(QCoreApplication::translate("MainWindow", "navBtn", nullptr));
        historyButton->setText(QCoreApplication::translate("MainWindow", "\342\226\244  L\341\273\213ch s\341\273\255", nullptr));
#if QT_CONFIG(tooltip)
        historyButton->setToolTip(QCoreApplication::translate("MainWindow", "L\341\273\213ch s\341\273\255 d\341\273\257 li\341\273\207u", nullptr));
#endif // QT_CONFIG(tooltip)
        usersButton->setObjectName(QCoreApplication::translate("MainWindow", "navBtn", nullptr));
        usersButton->setText(QCoreApplication::translate("MainWindow", "\342\231\237  T\303\240i kho\341\272\243n", nullptr));
#if QT_CONFIG(tooltip)
        usersButton->setToolTip(QCoreApplication::translate("MainWindow", "Qu\341\272\243n l\303\275 t\303\240i kho\341\272\243n", nullptr));
#endif // QT_CONFIG(tooltip)
        logoutButton->setObjectName(QCoreApplication::translate("MainWindow", "navLogoutBtn", nullptr));
        logoutButton->setText(QCoreApplication::translate("MainWindow", "\342\206\252  \304\220\304\203ng xu\341\272\245t", nullptr));
#if QT_CONFIG(tooltip)
        logoutButton->setToolTip(QCoreApplication::translate("MainWindow", "\304\220\304\203ng xu\341\272\245t", nullptr));
#endif // QT_CONFIG(tooltip)
        pages->setObjectName(QCoreApplication::translate("MainWindow", "pageContainer", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
