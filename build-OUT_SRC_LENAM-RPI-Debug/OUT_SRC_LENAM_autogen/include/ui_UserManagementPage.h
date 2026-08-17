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
    QVBoxLayout *verticalLayout;
    QHBoxLayout *headerLayout;
    QLabel *titleLabel;
    QSpacerItem *spacer;
    QPushButton *addUserButton;
    QTableWidget *usersTable;

    void setupUi(QWidget *UserManagementPage)
    {
        if (UserManagementPage->objectName().isEmpty())
            UserManagementPage->setObjectName("UserManagementPage");
        verticalLayout = new QVBoxLayout(UserManagementPage);
        verticalLayout->setObjectName("verticalLayout");
        headerLayout = new QHBoxLayout();
        headerLayout->setObjectName("headerLayout");
        titleLabel = new QLabel(UserManagementPage);
        titleLabel->setObjectName("titleLabel");

        headerLayout->addWidget(titleLabel);

        spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        headerLayout->addItem(spacer);

        addUserButton = new QPushButton(UserManagementPage);
        addUserButton->setObjectName("addUserButton");

        headerLayout->addWidget(addUserButton);


        verticalLayout->addLayout(headerLayout);

        usersTable = new QTableWidget(UserManagementPage);
        if (usersTable->columnCount() < 4)
            usersTable->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        usersTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        usersTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        usersTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        usersTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        usersTable->setObjectName("usersTable");
        usersTable->setColumnCount(4);
        usersTable->setRowCount(0);

        verticalLayout->addWidget(usersTable);


        retranslateUi(UserManagementPage);

        QMetaObject::connectSlotsByName(UserManagementPage);
    } // setupUi

    void retranslateUi(QWidget *UserManagementPage)
    {
        titleLabel->setText(QCoreApplication::translate("UserManagementPage", "QU\341\272\242N L\303\235 T\303\200I KHO\341\272\242N", nullptr));
        addUserButton->setText(QCoreApplication::translate("UserManagementPage", "+ Th\303\252m t\303\240i kho\341\272\243n", nullptr));
        QTableWidgetItem *___qtablewidgetitem = usersTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("UserManagementPage", "T\303\240i kho\341\272\243n", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = usersTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("UserManagementPage", "Quy\341\273\201n", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = usersTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("UserManagementPage", "Device ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = usersTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("UserManagementPage", "Tr\341\272\241ng th\303\241i", nullptr));
        (void)UserManagementPage;
    } // retranslateUi

};

namespace Ui {
    class UserManagementPage: public Ui_UserManagementPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERMANAGEMENTPAGE_H
