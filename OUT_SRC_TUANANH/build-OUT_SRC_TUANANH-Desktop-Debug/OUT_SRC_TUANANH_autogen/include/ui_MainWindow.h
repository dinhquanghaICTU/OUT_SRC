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
    QLabel *systemTitleLabel;
    QSpacerItem *headerLeftSpacer;
    QPushButton *dashboardButton;
    QPushButton *usersButton;
    QSpacerItem *headerSpacer;
    QLabel *roleBadgeLabel;
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
        topHeaderBar->setStyleSheet(QString::fromUtf8("QFrame#topHeaderBar {\n"
"  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0a1016, stop:0.5 #0c141c, stop:1 #06090e);\n"
"  border-bottom: 1.5px solid #162432;\n"
"}"));
        headerLayout = new QHBoxLayout(topHeaderBar);
        headerLayout->setSpacing(10);
        headerLayout->setObjectName("headerLayout");
        headerLayout->setContentsMargins(12, 4, 12, 4);
        systemTitleLabel = new QLabel(topHeaderBar);
        systemTitleLabel->setObjectName("systemTitleLabel");
        systemTitleLabel->setStyleSheet(QString::fromUtf8("color: #00f0ff; font-size: 11px; font-weight: 900; letter-spacing: 0.8px;"));

        headerLayout->addWidget(systemTitleLabel);

        headerLeftSpacer = new QSpacerItem(16, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        headerLayout->addItem(headerLeftSpacer);

        dashboardButton = new QPushButton(topHeaderBar);
        dashboardButton->setObjectName("dashboardButton");
        dashboardButton->setCursor(QCursor(Qt::PointingHandCursor));
        dashboardButton->setCheckable(true);
        dashboardButton->setChecked(true);
        dashboardButton->setAutoExclusive(true);
        dashboardButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background: transparent;\n"
"  color: #64748b;\n"
"  border: 1px solid transparent;\n"
"  border-radius: 5px;\n"
"  padding: 4px 12px;\n"
"  font-size: 10px;\n"
"  font-weight: 800;\n"
"}\n"
"QPushButton:hover {\n"
"  background: #111c26;\n"
"  color: #ffffff;\n"
"}\n"
"QPushButton:checked {\n"
"  background: #0284c7;\n"
"  color: #ffffff;\n"
"  border: 1px solid #38bdf8;\n"
"  font-weight: 900;\n"
"}"));

        headerLayout->addWidget(dashboardButton);

        usersButton = new QPushButton(topHeaderBar);
        usersButton->setObjectName("usersButton");
        usersButton->setCursor(QCursor(Qt::PointingHandCursor));
        usersButton->setCheckable(true);
        usersButton->setAutoExclusive(true);
        usersButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background: transparent;\n"
"  color: #64748b;\n"
"  border: 1px solid transparent;\n"
"  border-radius: 5px;\n"
"  padding: 4px 12px;\n"
"  font-size: 10px;\n"
"  font-weight: 800;\n"
"}\n"
"QPushButton:hover {\n"
"  background: #111c26;\n"
"  color: #ffffff;\n"
"}\n"
"QPushButton:checked {\n"
"  background: #0284c7;\n"
"  color: #ffffff;\n"
"  border: 1px solid #38bdf8;\n"
"  font-weight: 900;\n"
"}"));

        headerLayout->addWidget(usersButton);

        headerSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        headerLayout->addItem(headerSpacer);

        roleBadgeLabel = new QLabel(topHeaderBar);
        roleBadgeLabel->setObjectName("roleBadgeLabel");
        roleBadgeLabel->setStyleSheet(QString::fromUtf8("background: rgba(2, 132, 199, 0.18);\n"
"color: #38bdf8;\n"
"border: 1px solid #0284c7;\n"
"border-radius: 4px;\n"
"padding: 2px 7px;\n"
"font-size: 9px;\n"
"font-weight: 800;"));

        headerLayout->addWidget(roleBadgeLabel);

        logoutButton = new QPushButton(topHeaderBar);
        logoutButton->setObjectName("logoutButton");
        logoutButton->setCursor(QCursor(Qt::PointingHandCursor));
        logoutButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background: #2b1118;\n"
"  color: #f87171;\n"
"  border: 1px solid #7f1d1d;\n"
"  border-radius: 5px;\n"
"  padding: 3px 8px;\n"
"  font-size: 9px;\n"
"  font-weight: 800;\n"
"}\n"
"QPushButton:hover {\n"
"  background: #dc2626;\n"
"  color: #ffffff;\n"
"}"));

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
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "TUANANH - Tactical Cyber-Light & Security Mesh", nullptr));
        systemTitleLabel->setText(QCoreApplication::translate("MainWindow", "\342\254\242 TUANANH // CYBER-LIGHT & PIR MESH", nullptr));
        dashboardButton->setText(QCoreApplication::translate("MainWindow", "\342\254\241 TACTICAL HUD", nullptr));
        usersButton->setText(QCoreApplication::translate("MainWindow", "\342\254\241 ACCESS CONTROL", nullptr));
        roleBadgeLabel->setText(QCoreApplication::translate("MainWindow", "\360\237\221\221 ROOT ADMIN", nullptr));
        logoutButton->setText(QCoreApplication::translate("MainWindow", "LOGOUT \342\206\252", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
