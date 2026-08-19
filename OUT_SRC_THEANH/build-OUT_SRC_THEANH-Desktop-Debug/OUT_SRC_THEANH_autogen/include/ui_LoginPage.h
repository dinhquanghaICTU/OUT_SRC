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
    QVBoxLayout *pageLayout;
    QSpacerItem *topSpacer;
    QLabel *myAccountLabel;
    QWidget *cardWrapper;
    QVBoxLayout *wrapperLayout;
    QLabel *avatarBadge;
    QFrame *loginCard;
    QVBoxLayout *loginFormLayout;
    QHBoxLayout *userRow;
    QLabel *userIcon;
    QLineEdit *usernameEdit;
    QHBoxLayout *passRow;
    QLabel *passIcon;
    QLineEdit *passwordEdit;
    QPushButton *toggleVisibilityBtn;
    QLabel *forgotPasswordLabel;
    QCheckBox *rememberAccountCheckBox;
    QPushButton *loginButton;
    QSpacerItem *bottomSpacer;
    QHBoxLayout *keyboardContainerLayout;

    void setupUi(QWidget *LoginPage)
    {
        if (LoginPage->objectName().isEmpty())
            LoginPage->setObjectName("LoginPage");
        LoginPage->resize(480, 800);
        pageLayout = new QVBoxLayout(LoginPage);
        pageLayout->setSpacing(0);
        pageLayout->setObjectName("pageLayout");
        pageLayout->setContentsMargins(0, 20, 0, 0);
        topSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        pageLayout->addItem(topSpacer);

        myAccountLabel = new QLabel(LoginPage);
        myAccountLabel->setObjectName("myAccountLabel");
        myAccountLabel->setAlignment(Qt::AlignCenter);
        myAccountLabel->setStyleSheet(QString::fromUtf8("color: #ffffff; font-size: 22px; font-weight: 600; letter-spacing: 0.5px; background: transparent; padding-bottom: 12px;"));

        pageLayout->addWidget(myAccountLabel, 0, Qt::AlignHCenter);

        cardWrapper = new QWidget(LoginPage);
        cardWrapper->setObjectName("cardWrapper");
        cardWrapper->setMinimumSize(QSize(310, 340));
        cardWrapper->setMaximumSize(QSize(330, 360));
        wrapperLayout = new QVBoxLayout(cardWrapper);
        wrapperLayout->setSpacing(0);
        wrapperLayout->setObjectName("wrapperLayout");
        wrapperLayout->setContentsMargins(0, 0, 0, 0);
        avatarBadge = new QLabel(cardWrapper);
        avatarBadge->setObjectName("avatarBadge");
        avatarBadge->setMinimumSize(QSize(64, 64));
        avatarBadge->setMaximumSize(QSize(64, 64));
        avatarBadge->setStyleSheet(QString::fromUtf8("background-color: #1e3243; border: 4px solid #ffffff; border-radius: 32px; color: #ffffff; font-size: 26px;"));
        avatarBadge->setAlignment(Qt::AlignCenter);

        wrapperLayout->addWidget(avatarBadge, 0, Qt::AlignHCenter);

        loginCard = new QFrame(cardWrapper);
        loginCard->setObjectName("loginCard");
        loginCard->setStyleSheet(QString::fromUtf8("QFrame#loginCard { background-color: #ffffff; border-radius: 12px; margin-top: -32px; padding-top: 30px; }"));
        loginFormLayout = new QVBoxLayout(loginCard);
        loginFormLayout->setSpacing(14);
        loginFormLayout->setObjectName("loginFormLayout");
        loginFormLayout->setContentsMargins(20, 32, 20, 24);
        userRow = new QHBoxLayout();
        userRow->setSpacing(8);
        userRow->setObjectName("userRow");
        userIcon = new QLabel(loginCard);
        userIcon->setObjectName("userIcon");
        userIcon->setStyleSheet(QString::fromUtf8("color: #94a3b8; font-size: 15px; background: transparent; padding-bottom: 2px;"));

        userRow->addWidget(userIcon);

        usernameEdit = new QLineEdit(loginCard);
        usernameEdit->setObjectName("usernameEdit");
        usernameEdit->setMinimumSize(QSize(0, 34));
        usernameEdit->setStyleSheet(QString::fromUtf8("QLineEdit { background: transparent; border: none; border-bottom: 1.5px solid #cbd5e1; color: #1e293b; font-size: 14px; font-weight: 500; padding: 2px 4px; } QLineEdit:focus { border-bottom: 2px solid #f15a24; }"));

        userRow->addWidget(usernameEdit);


        loginFormLayout->addLayout(userRow);

        passRow = new QHBoxLayout();
        passRow->setSpacing(8);
        passRow->setObjectName("passRow");
        passIcon = new QLabel(loginCard);
        passIcon->setObjectName("passIcon");
        passIcon->setStyleSheet(QString::fromUtf8("color: #94a3b8; font-size: 15px; background: transparent; padding-bottom: 2px;"));

        passRow->addWidget(passIcon);

        passwordEdit = new QLineEdit(loginCard);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setMinimumSize(QSize(0, 34));
        passwordEdit->setEchoMode(QLineEdit::Password);
        passwordEdit->setStyleSheet(QString::fromUtf8("QLineEdit { background: transparent; border: none; border-bottom: 1.5px solid #cbd5e1; color: #1e293b; font-size: 14px; font-weight: 500; padding: 2px 4px; } QLineEdit:focus { border-bottom: 2px solid #f15a24; }"));

        passRow->addWidget(passwordEdit);

        toggleVisibilityBtn = new QPushButton(loginCard);
        toggleVisibilityBtn->setObjectName("toggleVisibilityBtn");
        toggleVisibilityBtn->setMinimumSize(QSize(34, 18));
        toggleVisibilityBtn->setMaximumSize(QSize(34, 18));
        toggleVisibilityBtn->setCursor(QCursor(Qt::PointingHandCursor));
        toggleVisibilityBtn->setCheckable(true);
        toggleVisibilityBtn->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #cbd5e1; border: none; border-radius: 9px; } QPushButton:checked { background-color: #2e4354; }"));

        passRow->addWidget(toggleVisibilityBtn);


        loginFormLayout->addLayout(passRow);

        forgotPasswordLabel = new QLabel(loginCard);
        forgotPasswordLabel->setObjectName("forgotPasswordLabel");
        forgotPasswordLabel->setCursor(QCursor(Qt::PointingHandCursor));
        forgotPasswordLabel->setStyleSheet(QString::fromUtf8("color: #8898aa; font-size: 11px; background: transparent;"));

        loginFormLayout->addWidget(forgotPasswordLabel, 0, Qt::AlignRight);

        rememberAccountCheckBox = new QCheckBox(loginCard);
        rememberAccountCheckBox->setObjectName("rememberAccountCheckBox");
        rememberAccountCheckBox->setVisible(false);
        rememberAccountCheckBox->setChecked(true);

        loginFormLayout->addWidget(rememberAccountCheckBox);

        loginButton = new QPushButton(loginCard);
        loginButton->setObjectName("loginButton");
        loginButton->setMinimumSize(QSize(0, 42));
        loginButton->setCursor(QCursor(Qt::PointingHandCursor));
        loginButton->setStyleSheet(QString::fromUtf8("QPushButton#loginButton { background-color: #f15a24; color: #ffffff; border: none; border-radius: 6px; font-size: 15px; font-weight: 600; } QPushButton#loginButton:hover { background-color: #d94814; } QPushButton#loginButton:pressed { background-color: #bf3e12; }"));

        loginFormLayout->addWidget(loginButton);


        wrapperLayout->addWidget(loginCard);


        pageLayout->addWidget(cardWrapper, 0, Qt::AlignHCenter);

        bottomSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        pageLayout->addItem(bottomSpacer);

        keyboardContainerLayout = new QHBoxLayout();
        keyboardContainerLayout->setObjectName("keyboardContainerLayout");
        keyboardContainerLayout->setContentsMargins(0, 0, 0, 0);

        pageLayout->addLayout(keyboardContainerLayout);


        retranslateUi(LoginPage);

        loginButton->setDefault(true);


        QMetaObject::connectSlotsByName(LoginPage);
    } // setupUi

    void retranslateUi(QWidget *LoginPage)
    {
        myAccountLabel->setText(QCoreApplication::translate("LoginPage", "My Account", nullptr));
        avatarBadge->setText(QCoreApplication::translate("LoginPage", "\360\237\221\244", nullptr));
        userIcon->setText(QCoreApplication::translate("LoginPage", "\360\237\221\244", nullptr));
        usernameEdit->setPlaceholderText(QCoreApplication::translate("LoginPage", "Login", nullptr));
        passIcon->setText(QCoreApplication::translate("LoginPage", "\360\237\224\222", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("LoginPage", "Password", nullptr));
        toggleVisibilityBtn->setText(QString());
        forgotPasswordLabel->setText(QCoreApplication::translate("LoginPage", "Forgot password ?", nullptr));
        rememberAccountCheckBox->setText(QCoreApplication::translate("LoginPage", "Remember", nullptr));
        loginButton->setText(QCoreApplication::translate("LoginPage", "Sign in", nullptr));
        (void)LoginPage;
    } // retranslateUi

};

namespace Ui {
    class LoginPage: public Ui_LoginPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINPAGE_H
