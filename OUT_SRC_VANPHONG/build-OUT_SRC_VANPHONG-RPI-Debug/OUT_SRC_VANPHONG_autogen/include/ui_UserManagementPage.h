/********************************************************************************
** Form generated from reading UI file 'UserManagementPage.ui'
**
** Created by: Qt User Interface Compiler version 6.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USERMANAGEMENTPAGE_H
#define UI_USERMANAGEMENTPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_UserManagementPage
{
public:
    QVBoxLayout *mainLayout;
    QHBoxLayout *topBar;
    QLabel *userPageTitle;
    QSpacerItem *userSpacer;
    QPushButton *btnAddUser;
    QTableWidget *userTable;

    void setupUi(QWidget *UserManagementPage)
    {
        if (UserManagementPage->objectName().isEmpty())
            UserManagementPage->setObjectName("UserManagementPage");
        UserManagementPage->resize(800, 480);
        mainLayout = new QVBoxLayout(UserManagementPage);
        mainLayout->setSpacing(8);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(12, 8, 12, 8);
        topBar = new QHBoxLayout();
        topBar->setObjectName("topBar");
        userPageTitle = new QLabel(UserManagementPage);
        userPageTitle->setObjectName("userPageTitle");
        userPageTitle->setStyleSheet(QString::fromUtf8("color: #34d399; font-size: 13px; font-weight: 800;"));

        topBar->addWidget(userPageTitle);

        userSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        topBar->addItem(userSpacer);

        btnAddUser = new QPushButton(UserManagementPage);
        btnAddUser->setObjectName("btnAddUser");
        btnAddUser->setCursor(QCursor(Qt::PointingHandCursor));
        btnAddUser->setStyleSheet(QString::fromUtf8("QPushButton { background: #059669; color: white; border: none; border-radius: 6px; padding: 6px 14px; font-weight: 700; font-size: 11px; } QPushButton:hover { background: #10b981; }"));

        topBar->addWidget(btnAddUser);


        mainLayout->addLayout(topBar);

        userTable = new QTableWidget(UserManagementPage);
        userTable->setObjectName("userTable");
        userTable->setStyleSheet(QString::fromUtf8("QTableWidget { background-color: #0c2317; border: 1px solid #1b4332; border-radius: 8px; color: #f1f5f9; gridline-color: #1b4332; } QHeaderView::section { background-color: #133925; color: #34d399; font-weight: 700; padding: 6px; border: none; }"));

        mainLayout->addWidget(userTable);


        retranslateUi(UserManagementPage);

        QMetaObject::connectSlotsByName(UserManagementPage);
    } // setupUi

    void retranslateUi(QWidget *UserManagementPage)
    {
        userPageTitle->setText(QCoreApplication::translate("UserManagementPage", "\360\237\221\245 QU\341\272\242N TR\341\273\212 T\303\200I KHO\341\272\242N V\341\272\254N H\303\200NH V\306\257\341\273\234N", nullptr));
        btnAddUser->setText(QCoreApplication::translate("UserManagementPage", "\342\236\225 Th\303\252m t\303\240i kho\341\272\243n m\341\273\233i", nullptr));
        (void)UserManagementPage;
    } // retranslateUi

};

namespace Ui {
    class UserManagementPage: public Ui_UserManagementPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERMANAGEMENTPAGE_H
