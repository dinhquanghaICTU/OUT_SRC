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
    QHBoxLayout *headerLayout;
    QLabel *titleLabel;
    QSpacerItem *headerSpacer;
    QPushButton *addUserButton;
    QTableWidget *usersTable;

    void setupUi(QWidget *UserManagementPage)
    {
        if (UserManagementPage->objectName().isEmpty())
            UserManagementPage->setObjectName("UserManagementPage");
        UserManagementPage->resize(800, 480);
        mainLayout = new QVBoxLayout(UserManagementPage);
        mainLayout->setSpacing(8);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(12, 10, 12, 10);
        headerLayout = new QHBoxLayout();
        headerLayout->setObjectName("headerLayout");
        titleLabel = new QLabel(UserManagementPage);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setStyleSheet(QString::fromUtf8("color: #38bdf8; font-size: 14px; font-weight: 800;"));

        headerLayout->addWidget(titleLabel);

        headerSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        headerLayout->addItem(headerSpacer);

        addUserButton = new QPushButton(UserManagementPage);
        addUserButton->setObjectName("addUserButton");
        addUserButton->setCursor(QCursor(Qt::PointingHandCursor));
        addUserButton->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #10b981; color: white; border: none; border-radius: 6px; padding: 6px 14px; font-weight: 700; font-size: 11px; } QPushButton:hover { background-color: #059669; }"));

        headerLayout->addWidget(addUserButton);


        mainLayout->addLayout(headerLayout);

        usersTable = new QTableWidget(UserManagementPage);
        if (usersTable->columnCount() < 5)
            usersTable->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        usersTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        usersTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        usersTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        usersTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        usersTable->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        usersTable->setObjectName("usersTable");
        usersTable->setColumnCount(5);
        usersTable->setStyleSheet(QString::fromUtf8("QTableWidget { background-color: #0f172a; border: 1.5px solid #1e293b; color: #f1f5f9; border-radius: 10px; gridline-color: #1e293b; } QHeaderView::section { background-color: #1e293b; color: #38bdf8; font-weight: 700; padding: 6px; border: none; }"));

        mainLayout->addWidget(usersTable);


        retranslateUi(UserManagementPage);

        QMetaObject::connectSlotsByName(UserManagementPage);
    } // setupUi

    void retranslateUi(QWidget *UserManagementPage)
    {
        titleLabel->setText(QCoreApplication::translate("UserManagementPage", "\360\237\221\245 QU\341\272\242N L\303\235 T\303\200I KHO\341\272\242N V\341\272\254N H\303\200NH & PH\303\202N QUY\341\273\200N", nullptr));
        addUserButton->setText(QCoreApplication::translate("UserManagementPage", "+ Th\303\252m t\303\240i kho\341\272\243n m\341\273\233i", nullptr));
        QTableWidgetItem *___qtablewidgetitem = usersTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("UserManagementPage", "T\303\252n t\303\240i kho\341\272\243n", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = usersTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("UserManagementPage", "Vai tr\303\262 / Quy\341\273\201n h\341\272\241n", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = usersTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("UserManagementPage", "Thi\341\272\277t b\341\273\213 g\303\241n", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = usersTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("UserManagementPage", "Tr\341\272\241ng th\303\241i", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = usersTable->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("UserManagementPage", "Thao t\303\241c", nullptr));
        (void)UserManagementPage;
    } // retranslateUi

};

namespace Ui {
    class UserManagementPage: public Ui_UserManagementPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERMANAGEMENTPAGE_H
