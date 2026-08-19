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
    QWidget *sideBar;
    QHBoxLayout *sideBarLayout;
    QLabel *sideLogo;
    QLabel *appTitleLabel;
    QPushButton *dashboardButton;
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
        MainWindow->setMinimumSize(QSize(320, 240));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setSpacing(0);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(0, 0, 0, 0);
        sideBar = new QWidget(centralWidget);
        sideBar->setObjectName("sideBar");
        sideBar->setMinimumSize(QSize(0, 44));
        sideBar->setMaximumSize(QSize(16777215, 44));
        sideBarLayout = new QHBoxLayout(sideBar);
        sideBarLayout->setSpacing(8);
        sideBarLayout->setObjectName("sideBarLayout");
        sideBarLayout->setContentsMargins(12, 4, 12, 4);
        sideLogo = new QLabel(sideBar);
        sideLogo->setObjectName("sideLogo");
        sideLogo->setMinimumSize(QSize(30, 30));
        sideLogo->setMaximumSize(QSize(30, 30));
        sideLogo->setPixmap(QPixmap(QString::fromUtf8(":/images/logo_ictu.png")));
        sideLogo->setScaledContents(true);

        sideBarLayout->addWidget(sideLogo);

        appTitleLabel = new QLabel(sideBar);
        appTitleLabel->setObjectName("appTitleLabel");
        appTitleLabel->setStyleSheet(QString::fromUtf8("color: #38bdf8; font-size: 13px; font-weight: 900; letter-spacing: 0.5px; margin-right: 8px;"));

        sideBarLayout->addWidget(appTitleLabel);

        dashboardButton = new QPushButton(sideBar);
        dashboardButton->setObjectName("dashboardButton");
        dashboardButton->setCheckable(true);
        dashboardButton->setChecked(true);
        dashboardButton->setAutoExclusive(true);

        sideBarLayout->addWidget(dashboardButton);

        usersButton = new QPushButton(sideBar);
        usersButton->setObjectName("usersButton");
        usersButton->setCheckable(true);
        usersButton->setAutoExclusive(true);

        sideBarLayout->addWidget(usersButton);

        navSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

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
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "ICTU Environmental & Climate Control Center", nullptr));
        appTitleLabel->setText(QCoreApplication::translate("MainWindow", "SON IOT CENTER", nullptr));
#if QT_CONFIG(tooltip)
        dashboardButton->setToolTip(QCoreApplication::translate("MainWindow", "Trang ch\341\273\247", nullptr));
#endif // QT_CONFIG(tooltip)
        dashboardButton->setText(QCoreApplication::translate("MainWindow", "\342\214\202 Trang ch\341\273\247", nullptr));
#if QT_CONFIG(tooltip)
        usersButton->setToolTip(QCoreApplication::translate("MainWindow", "Qu\341\272\243n l\303\275 t\303\240i kho\341\272\243n", nullptr));
#endif // QT_CONFIG(tooltip)
        usersButton->setText(QCoreApplication::translate("MainWindow", "\342\231\237 T\303\240i kho\341\272\243n", nullptr));
#if QT_CONFIG(tooltip)
        logoutButton->setToolTip(QCoreApplication::translate("MainWindow", "\304\220\304\203ng xu\341\272\245t", nullptr));
#endif // QT_CONFIG(tooltip)
        logoutButton->setText(QCoreApplication::translate("MainWindow", "\342\206\252 \304\220\304\203ng xu\341\272\245t", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
