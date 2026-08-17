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
    QLabel *welcomeLabel;
    QLabel *subtitleLabel;
    QSpacerItem *formGap;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QCheckBox *rememberAccountCheckBox;
    QPushButton *loginButton;
    QSpacerItem *bottomSpacer;

    void setupUi(QWidget *LoginPage)
    {
        if (LoginPage->objectName().isEmpty())
            LoginPage->setObjectName("LoginPage");
        LoginPage->resize(800, 480);
        LoginPage->setStyleSheet(QString::fromUtf8("QWidget#LoginPage {\n"
"  background-color: #06090e;\n"
"  color: #e2e8f0;\n"
"  font-family: 'Segoe UI', sans-serif;\n"
"}"));
        pageLayout = new QGridLayout(LoginPage);
        pageLayout->setSpacing(0);
        pageLayout->setObjectName("pageLayout");
        pageLayout->setContentsMargins(16, 10, 16, 10);
        topSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        pageLayout->addItem(topSpacer, 0, 0, 1, 1);

        loginCard = new QFrame(LoginPage);
        loginCard->setObjectName("loginCard");
        loginCard->setMinimumSize(QSize(340, 0));
        loginCard->setMaximumSize(QSize(360, 16777215));
        loginCard->setStyleSheet(QString::fromUtf8("QFrame#loginCard {\n"
"  background-color: #0c1218;\n"
"  border: 1.5px solid #162430;\n"
"  border-radius: 10px;\n"
"}"));
        loginFormLayout = new QVBoxLayout(loginCard);
        loginFormLayout->setSpacing(8);
        loginFormLayout->setObjectName("loginFormLayout");
        loginFormLayout->setContentsMargins(18, 14, 18, 14);
        welcomeLabel = new QLabel(loginCard);
        welcomeLabel->setObjectName("welcomeLabel");
        welcomeLabel->setStyleSheet(QString::fromUtf8("color: #00f0ff; font-size: 13px; font-weight: 900; letter-spacing: 0.8px;"));
        welcomeLabel->setAlignment(Qt::AlignCenter);

        loginFormLayout->addWidget(welcomeLabel, 0, Qt::AlignHCenter);

        subtitleLabel = new QLabel(loginCard);
        subtitleLabel->setObjectName("subtitleLabel");
        subtitleLabel->setStyleSheet(QString::fromUtf8("color: #64748b; font-size: 8px; font-weight: 800; letter-spacing: 0.5px;"));
        subtitleLabel->setAlignment(Qt::AlignCenter);

        loginFormLayout->addWidget(subtitleLabel, 0, Qt::AlignHCenter);

        formGap = new QSpacerItem(20, 6, QSizePolicy::Minimum, QSizePolicy::Fixed);

        loginFormLayout->addItem(formGap);

        usernameEdit = new QLineEdit(loginCard);
        usernameEdit->setObjectName("usernameEdit");
        usernameEdit->setMinimumSize(QSize(0, 36));
        usernameEdit->setClearButtonEnabled(true);
        usernameEdit->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"  background-color: #080c10;\n"
"  color: #ffffff;\n"
"  border: 1.5px solid #162430;\n"
"  border-radius: 6px;\n"
"  padding: 4px 10px;\n"
"  font-size: 12px;\n"
"}\n"
"QLineEdit:focus {\n"
"  border: 1.5px solid #00f0ff;\n"
"  background-color: #0e1620;\n"
"}"));

        loginFormLayout->addWidget(usernameEdit);

        passwordEdit = new QLineEdit(loginCard);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setMinimumSize(QSize(0, 36));
        passwordEdit->setEchoMode(QLineEdit::Password);
        passwordEdit->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"  background-color: #080c10;\n"
"  color: #ffffff;\n"
"  border: 1.5px solid #162430;\n"
"  border-radius: 6px;\n"
"  padding: 4px 10px;\n"
"  font-size: 12px;\n"
"}\n"
"QLineEdit:focus {\n"
"  border: 1.5px solid #00f0ff;\n"
"  background-color: #0e1620;\n"
"}"));

        loginFormLayout->addWidget(passwordEdit);

        rememberAccountCheckBox = new QCheckBox(loginCard);
        rememberAccountCheckBox->setObjectName("rememberAccountCheckBox");
        rememberAccountCheckBox->setStyleSheet(QString::fromUtf8("color: #94a3b8; font-size: 10px; font-weight: 700; spacing: 6px;"));

        loginFormLayout->addWidget(rememberAccountCheckBox);

        loginButton = new QPushButton(loginCard);
        loginButton->setObjectName("loginButton");
        loginButton->setMinimumSize(QSize(0, 36));
        loginButton->setCursor(QCursor(Qt::PointingHandCursor));
        loginButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0284c7, stop:1 #00f0ff);\n"
"  color: #000000;\n"
"  border: none;\n"
"  border-radius: 6px;\n"
"  font-size: 11px;\n"
"  font-weight: 900;\n"
"  letter-spacing: 0.5px;\n"
"}\n"
"QPushButton:hover {\n"
"  background: #38bdf8;\n"
"}\n"
"QPushButton:pressed {\n"
"  background: #0369a1;\n"
"}"));

        loginFormLayout->addWidget(loginButton);


        pageLayout->addWidget(loginCard, 1, 0, 1, 1, Qt::AlignCenter);

        bottomSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        pageLayout->addItem(bottomSpacer, 2, 0, 1, 1);


        retranslateUi(LoginPage);

        QMetaObject::connectSlotsByName(LoginPage);
    } // setupUi

    void retranslateUi(QWidget *LoginPage)
    {
        welcomeLabel->setText(QCoreApplication::translate("LoginPage", "\342\254\242 SYSTEM ACCESS TERMINAL", nullptr));
        subtitleLabel->setText(QCoreApplication::translate("LoginPage", "TUANANH // SECURE MESH CONSOLE", nullptr));
        usernameEdit->setPlaceholderText(QCoreApplication::translate("LoginPage", "T\303\252n t\303\240i kho\341\272\243n (Operator ID)", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("LoginPage", "M\341\272\255t kh\341\272\251u (Security Passcode)", nullptr));
        rememberAccountCheckBox->setText(QCoreApplication::translate("LoginPage", "Ghi nh\341\273\233 t\303\240i kho\341\272\243n tr\303\252n thi\341\272\277t b\341\273\213 n\303\240y", nullptr));
        loginButton->setText(QCoreApplication::translate("LoginPage", "AUTHENTICATE & ENTER \342\236\224", nullptr));
        (void)LoginPage;
    } // retranslateUi

};

namespace Ui {
    class LoginPage: public Ui_LoginPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINPAGE_H
