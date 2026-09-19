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
        centralWidget->setObjectName(