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
#include <QtGui/QIcon>
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
    QVBoxLayout *mainPageLayout;
    QFrame *loginRootFrame;
    QHBoxLayout *contentLayout;
    QFrame *heroFrame;
    QVBoxLayout *heroLayout;
    QLabel *heroImageLabel;
    QFrame *loginFormCard;
    QVBoxLayout *formLayout;
    QLabel *welcomeTitle;
    QSpacerItem *titleGap;
    QLabel *usernameLabel;
    QLineEdit *usernameEdit;
    QLabel *passwordLabel;
    QLineEdit *passwordEdit;
    QHBoxLayout *forgotLayout;
    QCheckBox *rememberAccountCheckBox;
    QSpacerItem *forgotSpacer;
    QLabel *forgetPasswordLabel;
    QSpacerItem *btnGap;
    QPushButton *loginButton;
    QPushButton *signinButton;
    QHBoxLayout *orLayout;
    QFrame *leftOrLine;
    QLabel *orLabel;
    QFrame *rightOrLine;
    QHBoxLayout *socialLayout;
    QSpacerItem *socialLeftSpacer;
    QPushButton *googleBtn;
    QPushButton *fbBtn;
    QPushButton *xBtn;
    QSpacerItem *socialRightSpacer;

    void setupUi(QWidget *LoginPage)
    {
        if (LoginPage->objectName().isEmpty())
            LoginPage->setObjectName("LoginPage");
        LoginPage->resize(800, 480);
        mainPageLayout = new QVBoxLayout(LoginPage);
        mainPageLayout->setSpacing(0);
        mainPageLayout->setObjectName("mainPageLayout");
        mainPageLayout->setContentsMargins(0, 0, 0, 0);
        loginRootFrame = new QFrame(LoginPage);
        loginRootFrame->setObjectName("loginRootFrame");
        contentLayout = new QHBoxLayout(loginRootFrame);
        contentLayout->setSpacing(16);
        contentLayout->setObjectName("contentLayout");
        contentLayout->setContentsMargins(24, 12, 28, 12);
        heroFrame = new QFrame(loginRootFrame);
        heroFrame->setObjectName("heroFrame");
        heroFrame->setMinimumSize(QSize(320, 0));
        heroLayout = new QVBoxLayout(heroFrame);
        heroLayout->setSpacing(0);
        heroLayout->setObjectName("heroLayout");
        heroLayout->setContentsMargins(0, 0, 0, 0);
        heroImageLabel = new QLabel(heroFrame);
        heroImageLabel->setObjectName("heroImageLabel");
        heroImageLabel->setMinimumSize(QSize(280, 340));
        heroImageLabel->setMaximumSize(QSize(320, 380));
        heroImageLabel->setPixmap(QPixmap(QString::fromUtf8(":/images/login_hero.png")));
        heroImageLabel->setScaledContents(true);
        heroImageLabel->setAlignment(Qt::AlignCenter);

        heroLayout->addWidget(heroImageLabel, 0, Qt::AlignCenter);


        contentLayout->addWidget(heroFrame);

        loginFormCard = new QFrame(loginRootFrame);
        loginFormCard->setObjectName("loginFormCard");
        loginFormCard->setMinimumSize(QSize(340, 0));
        loginFormCard->setMaximumSize(QSize(370, 16777215));
        formLayout = new QVBoxLayout(loginFormCard);
        formLayout->setSpacing(5);
        formLayout->setObjectName("formLayout");
        formLayout->setContentsMargins(12, 4, 12, 4);
        welcomeTitle = new QLabel(loginFormCard);
        welcomeTitle->setObjectName("welcomeTitle");
        welcomeTitle->setAlignment(Qt::AlignCenter);

        formLayout->addWidget(welcomeTitle, 0, Qt::AlignHCenter);

        titleGap = new QSpacerItem(20, 4, QSizePolicy::Minimum, QSizePolicy::Expanding);

        formLayout->addItem(titleGap);

        usernameLabel = new QLabel(loginFormCard);
        usernameLabel->setObjectName("usernameLabel");

        formLayout->addWidget(usernameLabel);

        usernameEdit = new QLineEdit(loginFormCard);
        usernameEdit->setObjectName("usernameEdit");
        usernameEdit->setMinimumSize(QSize(0, 34));
        usernameEdit->setClearButtonEnabled(true);

        formLayout->addWidget(usernameEdit);

        passwordLabel = new QLabel(loginFormCard);
        passwordLabel->setObjectName("passwordLabel");

        formLayout->addWidget(passwordLabel);

        passwordEdit = new QLineEdit(loginFormCard);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setMinimumSize(QSize(0, 34));
        passwordEdit->setEchoMode(QLineEdit::Password);

        formLayout->addWidget(passwordEdit);

        forgotLayout = new QHBoxLayout();
        forgotLayout->setSpacing(0);
        forgotLayout->setObjectName("forgotLayout");
        rememberAccountCheckBox = new QCheckBox(loginFormCard);
        rememberAccountCheckBox->setObjectName("rememberAccountCheckBox");

        forgotLayout->addWidget(rememberAccountCheckBox);

        forgotSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        forgotLayout->addItem(forgotSpacer);

        forgetPasswordLabel = new QLabel(loginFormCard);
        forgetPasswordLabel->setObjectName("forgetPasswordLabel");
        forgetPasswordLabel->setCursor(QCursor(Qt::PointingHandCursor));
        forgetPasswordLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        forgotLayout->addWidget(forgetPasswordLabel);


        formLayout->addLayout(forgotLayout);

        btnGap = new QSpacerItem(20, 4, QSizePolicy::Minimum, QSizePolicy::Expanding);

        formLayout->addItem(btnGap);

        loginButton = new QPushButton(loginFormCard);
        loginButton->setObjectName("loginButton");
        loginButton->setMinimumSize(QSize(0, 36));
        loginButton->setCursor(QCursor(Qt::PointingHandCursor));

        formLayout->addWidget(loginButton);

        signinButton = new QPushButton(loginFormCard);
        signinButton->setObjectName("signinButton");
        signinButton->setMinimumSize(QSize(0, 36));
        signinButton->setCursor(QCursor(Qt::PointingHandCursor));

        formLayout->addWidget(signinButton);

        orLayout = new QHBoxLayout();
        orLayout->setSpacing(6);
        orLayout->setObjectName("orLayout");
        orLayout->setContentsMargins(-1, 4, -1, 2);
        leftOrLine = new QFrame(loginFormCard);
        leftOrLine->setObjectName("leftOrLine");
        leftOrLine->setFrameShape(QFrame::HLine);
        leftOrLine->setFrameShadow(QFrame::Sunken);

        orLayout->addWidget(leftOrLine);

        orLabel = new QLabel(loginFormCard);
        orLabel->setObjectName("orLabel");
        orLabel->setAlignment(Qt::AlignCenter);

        orLayout->addWidget(orLabel);

        rightOrLine = new QFrame(loginFormCard);
        rightOrLine->setObjectName("rightOrLine");
        rightOrLine->setFrameShape(QFrame::HLine);
        rightOrLine->setFrameShadow(QFrame::Sunken);

        orLayout->addWidget(rightOrLine);


        formLayout->addLayout(orLayout);

        socialLayout = new QHBoxLayout();
        socialLayout->setSpacing(12);
        socialLayout->setObjectName("socialLayout");
        socialLeftSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        socialLayout->addItem(socialLeftSpacer);

        googleBtn = new QPushButton(loginFormCard);
        googleBtn->setObjectName("googleBtn");
        googleBtn->setMinimumSize(QSize(32, 32));
        googleBtn->setMaximumSize(QSize(32, 32));
        googleBtn->setCursor(QCursor(Qt::PointingHandCursor));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/google_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        googleBtn->setIcon(icon);
        googleBtn->setIconSize(QSize(26, 26));

        socialLayout->addWidget(googleBtn);

        fbBtn = new QPushButton(loginFormCard);
        fbBtn->setObjectName("fbBtn");
        fbBtn->setMinimumSize(QSize(32, 32));
        fbBtn->setMaximumSize(QSize(32, 32));
        fbBtn->setCursor(QCursor(Qt::PointingHandCursor));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/fb_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        fbBtn->setIcon(icon1);
        fbBtn->setIconSize(QSize(26, 26));

        socialLayout->addWidget(fbBtn);

        xBtn = new QPushButton(loginFormCard);
        xBtn->setObjectName("xBtn");
        xBtn->setMinimumSize(QSize(32, 32));
        xBtn->setMaximumSize(QSize(32, 32));
        xBtn->setCursor(QCursor(Qt::PointingHandCursor));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/x_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        xBtn->setIcon(icon2);
        xBtn->setIconSize(QSize(26, 26));

        socialLayout->addWidget(xBtn);

        socialRightSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        socialLayout->addItem(socialRightSpacer);


        formLayout->addLayout(socialLayout);


        contentLayout->addWidget(loginFormCard);


        mainPageLayout->addWidget(loginRootFrame);


        retranslateUi(LoginPage);

        loginButton->setDefault(true);


        QMetaObject::connectSlotsByName(LoginPage);
    } // setupUi

    void retranslateUi(QWidget *LoginPage)
    {
        welcomeTitle->setText(QCoreApplication::translate("LoginPage", "Welcome !", nullptr));
        usernameLabel->setText(QCoreApplication::translate("LoginPage", "Username", nullptr));
        usernameEdit->setPlaceholderText(QCoreApplication::translate("LoginPage", "Username", nullptr));
        passwordLabel->setText(QCoreApplication::translate("LoginPage", "Password", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("LoginPage", "Password", nullptr));
        rememberAccountCheckBox->setText(QCoreApplication::translate("LoginPage", "Remember", nullptr));
        forgetPasswordLabel->setText(QCoreApplication::translate("LoginPage", "Forgetpassword ?", nullptr));
        loginButton->setText(QCoreApplication::translate("LoginPage", "Login", nullptr));
        signinButton->setText(QCoreApplication::translate("LoginPage", "Signin", nullptr));
        orLabel->setText(QCoreApplication::translate("LoginPage", "OR", nullptr));
        (void)LoginPage;
    } // retranslateUi

};

namespace Ui {
    class LoginPage: public Ui_LoginPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINPAGE_H
