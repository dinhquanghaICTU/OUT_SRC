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
    QHBoxLayout *mainLayout;
    QWidget *sideBar;
    QVBoxLayout *sideBarLayout;
    QLabel *sideLogo;
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
        MainWindow->resize(1080, 680);
        MainWindow->setMinimumSize(QSize(640, 400));
        MainWindow->setMaximumSize(QSize(16777215, 16777215));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        mainLayout = new QHBoxLayout(centralWidget);
        mainLayout->setSpacing(0);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(0, 0, 0, 0);
        sideBar = new QWidget(centralWidget);
        sideBar->setObjectName("sideBar");
        sideBar->setMinimumSize(QSize(66, 0));
        sideBar->setMaximumSize(QSize(66, 16777215));
        sideBarLayout = new QVBoxLayout(sideBar);
        sideBarLayout->setSpacing(8);
        sideBarLayout->setObjectName("sideBarLayout");
        sideBarLayout->setContentsMargins(8, 12, 8, 12);
        sideLogo = new QLabel(sideBar);
        sideLogo->setObjectName("sideLogo");
        sideLogo->setMinimumSize(QSize(46, 46));
        sideLogo->setMaximumSize(QSize(46, 46));
        sideLogo->setPixmap(QPixmap(QString::fromUtf8(":/images/logo_ictu.png")));
        sideLogo->setScaledContents(true);

        sideBarLayout->addWidget(sideLogo);

        dashboardButton = new QPushButton(sideBar);
        dashboardButton->setObjectName("dashboardButton");
        dashboardButton->setCheckable(true);
        dashboardButton->setAutoExclusive(true);

        sideBarLayout->addWidget(dashboardButton);

        devicesButton = new QPushButton(sideBar);
        devicesButton->setObjectName("devicesButton");
        devicesButton->setCheckable(true);
        devicesButton->setAutoExclusive(true);

        sideBarLayout->addWidget(devicesButton);

        historyButton = new QPushButton(sideBar);
        historyButton->setObjectName("historyButton");
        historyButton->setCheckable(true);
        historyButton->setAutoExclusive(true);

        sideBarLayout->addWidget(historyButton);

        usersButton = new QPushButton(sideBar);
        usersButton->setObjectName("usersButton");
        usersButton->setCheckable(true);
        usersButton->setAutoExclusive(true);

        sideBarLayout->addWidget(usersButton);

        navSpacer = new QSpacerItem(20, 80, QSizePolicy::Minimum, QSizePolicy::Expanding);

        sideBarLayout->addItem(navSpacer);

        logoutButton = new QPushButton(sideBar);
        logoutButton->setObjectName("logoutButton");

        sideBarLayout->addWidget(logoutButton);


        mainLayout->addWidget(sideBar);

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
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "ICTU Environmental Monitor", nullptr));
#if QT_CONFIG(tooltip)
        dashboardButton->setToolTip(QCoreApplication::translate("MainWindow", "Trang ch\341\273\247", nullptr));
#endif // QT_CONFIG(tooltip)
        dashboardButton->setText(QCoreApplication::translate("MainWindow", "\342\214\202", nullptr));
#if QT_CONFIG(tooltip)
        devicesButton->setToolTip(QCoreApplication::translate("MainWindow", "Qu\341\272\243n l\303\275 thi\341\272\277t b\341\273\213", nullptr));
#endif // QT_CONFIG(tooltip)
        devicesButton->setText(QCoreApplication::translate("MainWindow", "\342\227\206", nullptr));
#if QT_CONFIG(tooltip)
        historyButton->setToolTip(QCoreApplication::translate("MainWindow", "L\341\273\213ch s\341\273\255 d\341\273\257 li\341\273\207u", nullptr));
#endif // QT_CONFIG(tooltip)
        historyButton->setText(QCoreApplication::translate("MainWindow", "\342\226\244", nullptr));
#if QT_CONFIG(tooltip)
        usersButton->setToolTip(QCoreApplication::translate("MainWindow", "Qu\341\272\243n l\303\275 t\303\240i kho\341\272\243n", nullptr));
#endif // QT_CONFIG(tooltip)
        usersButton->setText(QCoreApplication::translate("MainWindow", "\342\231\237", nullptr));
#if QT_CONFIG(tooltip)
        logoutButton->setToolTip(QCoreApplication::translate("MainWindow", "\304\220\304\203ng xu\341\272\245t", nullptr));
#endif // QT_CONFIG(tooltip)
        logoutButton->setText(QCoreApplication::translate("MainWindow", "\342\206\252", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
