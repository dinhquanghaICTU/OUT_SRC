/********************************************************************************
** Form generated from reading UI file 'LoginPage.ui'
**
** Created by: Qt User Interface Compiler version 6.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINPAGE_H
#define UI_LOGINPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoginPage
{
public:
    QGridLayout *pageLayout;
    QSpacerItem *topSpacer;
    QFrame *loginCard;
    QVBoxLayout *loginFormLayout;
    QLabel *logoLabel;
    QLabel *welcomeLabel;
    QLabel *subtitleLabel;
    QSpacerItem *formGap;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QHBoxLayout *optionsLayout;
    QCheckBox *rememberAccountCheckBox;
    QSpacerItem *optionSpacer;
    QPushButton *loginButton;
    QLabel *footerHintLabel;
    QSpacerItem *bottomSpacer;

    void setupUi(QWidget *LoginPage)
    {
        if (LoginPage->objectName().isEmpty())
            LoginPage->setObjectName("LoginPage");
        pageLayout = new QGridLayout(LoginPage);
        pageLayout->setObjectName("pageLayout");
        pageLayout->setHorizontalSpacing(0);
        pageLayout->setVerticalSpacing(0);
        pageLayout->setContentsMargins(20, 12, 20, 12);
        topSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        pageLayout->addItem(topSpacer, 0, 0, 1, 1);

        loginCard = new QFrame(LoginPage);
        loginCard->setObjectName("loginCard");
        loginCard->setMinimumSize(QSize(0, 0));
        loginCard->setMaximumSize(QSize(430, 16777215));
        loginFormLayout = new QVBoxLayout(loginCard);
        loginFormLayout->setSpacing(8);
        loginFormLayout->setObjectName("loginFormLayout");
        loginFormLayout->setContentsMargins(26, 18, 26, 18);
        logoLabel = new QLabel(loginCard);
        logoLabel->setObjectName("logoLabel");
        logoLabel->setMinimumSize(QSize(76, 76));
        logoLabel->setMaximumSize(QSize(76, 76));
        logoLabel->setPixmap(QPixmap(QString::fromUtf8(":/images/logo_ictu.png")));
        logoLabel->setScaledContents(true);
        logoLabel->setAlignment(Qt::AlignCenter);

        loginFormLayout->addWidget(logoLabel, 0, Qt::AlignHCenter);

        welcomeLabel = new QLabel(loginCard);
        welcomeLabel->setObjectName("welcomeLabel");
        welcomeLabel->setAlignment(Qt::AlignCenter);

        loginFormLayout->addWidget(welcomeLabel, 0, Qt::AlignHCenter);

        subtitleLabel = new QLabel(loginCard);
        subtitleLabel->setObjectName("subtitleLabel");
        subtitleLabel->setAlignment(Qt::AlignCenter);

        loginFormLayout->addWidget(subtitleLabel, 0, Qt::AlignHCenter);

        formGap = new QSpacerItem(20, 8, QSizePolicy::Minimum, QSizePolicy::Expanding);

        loginFormLayout->addItem(formGap);

        usernameEdit = new QLineEdit(loginCard);
        usernameEdit->setObjectName("usernameEdit");
        usernameEdit->setMinimumSize(QSize(0, 40));
        usernameEdit->setClearButtonEnabled(true);

        loginFormLayout->addWidget(usernameEdit);

        passwordEdit = new QLineEdit(loginCard);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setMinimumSize(QSize(0, 40));
        passwordEdit->setEchoMode(QLineEdit::Password);

        loginFormLayout->addWidget(passwordEdit);

        optionsLayout = new QHBoxLayout();
        optionsLayout->setObjectName("optionsLayout");
        optionsLayout->setContentsMargins(2, -1, 2, -1);
        rememberAccountCheckBox = new QCheckBox(loginCard);
        rememberAccountCheckBox->setObjectName("rememberAccountCheckBox");
        rememberAccountCheckBox->setCursor(QCursor(Qt::PointingHandCursor));

        optionsLayout->addWidget(rememberAccountCheckBox);

        optionSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        optionsLayout->addItem(optionSpacer);


        loginFormLayout->addLayout(optionsLayout);

        loginButton = new QPushButton(loginCard);
        loginButton->setObjectName("loginButton");
        loginButton->setMinimumSize(QSize(0, 42));
        loginButton->setCursor(QCursor(Qt::PointingHandCursor));

        loginFormLayout->addWidget(loginButton);

        footerHintLabel = new QLabel(loginCard);
        footerHintLabel->setObjectName("footerHintLabel");
        footerHintLabel->setAlignment(Qt::AlignCenter);

        loginFormLayout->addWidget(footerHintLabel, 0, Qt::AlignHCenter);


        pageLayout->addWidget(loginCard, 1, 0, 1, 1, Qt::AlignCenter);

        bottomSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        pageLayout->addItem(bottomSpacer, 2, 0, 1, 1);


        retranslateUi(LoginPage);

        loginButton->setDefault(true);


        QMetaObject::connectSlotsByName(LoginPage);
    } // setupUi

    void retranslateUi(QWidget *LoginPage)
    {
        welcomeLabel->setText(QCoreApplication::translate("LoginPage", "\304\220\304\203ng nh\341\272\255p h\341\273\207 th\341\273\221ng", nullptr));
        subtitleLabel->setText(QCoreApplication::translate("LoginPage", "ICTU Environmental Monitor", nullptr));
        usernameEdit->setPlaceholderText(QCoreApplication::translate("LoginPage", "T\303\240i kho\341\272\243n", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("LoginPage", "M\341\272\255t kh\341\272\251u", nullptr));
        rememberAccountCheckBox->setText(QCoreApplication::translate("LoginPage", "Ghi nh\341\273\233 t\303\240i kho\341\272\243n", nullptr));
        loginButton->setText(QCoreApplication::translate("LoginPage", "\304\220\304\202NG NH\341\272\254P", nullptr));
        footerHintLabel->setText(QCoreApplication::translate("LoginPage", "Nh\341\272\245n Enter \304\221\341\273\203 \304\221\304\203ng nh\341\272\255p nhanh", nullptr));
        (void)LoginPage;
    } // retranslateUi

};

namespace Ui {
    class LoginPage: public Ui_LoginPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINPAGE_H
