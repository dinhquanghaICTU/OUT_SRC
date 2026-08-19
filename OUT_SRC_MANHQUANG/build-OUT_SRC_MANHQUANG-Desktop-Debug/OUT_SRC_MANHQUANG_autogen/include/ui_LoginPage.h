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
    QHBoxLayout *contentRowLayout;
    QFrame *leftHeroPanel;
    QVBoxLayout *heroLayout;
    QLabel *heroBadge;
    QLabel *heroTitle;
    QLabel *heroSubtitle;
    QSpacerItem *heroGap;
    QFrame *featureCard1;
    QHBoxLayout *f1Layout;
    QLabel *f1Icon;
    QLabel *f1Text;
    QFrame *featureCard2;
    QHBoxLayout *f2Layout;
    QLabel *f2Icon;
    QLabel *f2Text;
    QFrame *featureCard3;
    QHBoxLayout *f3Layout;
    QLabel *f3Icon;
    QLabel *f3Text;
    QSpacerItem *heroBottomSpacer;
    QLabel *heroFooterStatus;
    QFrame *loginCard;
    QVBoxLayout *loginFormLayout;
    QLabel *loginTitle;
    QLabel *loginSubtitle;
    QHBoxLayout *userInputRow;
    QLabel *userIconLabel;
    QLineEdit *usernameEdit;
    QHBoxLayout *passInputRow;
    QLabel *passIconLabel;
    QLineEdit *passwordEdit;
    QPushButton *togglePassBtn;
    QHBoxLayout *optionsRow;
    QCheckBox *rememberAccountCheckBox;
    QSpacerItem *optSpacer;
    QPushButton *quickFillAdminBtn;
    QPushButton *loginButton;
    QLabel *keyboardToggleHint;
    QHBoxLayout *keyboardContainerLayout;

    void setupUi(QWidget *LoginPage)
    {
        if (LoginPage->objectName().isEmpty())
            LoginPage->setObjectName("LoginPage");
        LoginPage->resize(800, 480);
        pageLayout = new QVBoxLayout(LoginPage);
        pageLayout->setSpacing(0);
        pageLayout->setObjectName("pageLayout");
        pageLayout->setContentsMargins(16, 12, 16, 8);
        contentRowLayout = new QHBoxLayout();
        contentRowLayout->setSpacing(20);
        contentRowLayout->setObjectName("contentRowLayout");
        leftHeroPanel = new QFrame(LoginPage);
        leftHeroPanel->setObjectName("leftHeroPanel");
        leftHeroPanel->setMinimumSize(QSize(320, 0));
        leftHeroPanel->setStyleSheet(QString::fromUtf8("QFrame#leftHeroPanel { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #0f172a, stop:1 #1e1b4b); border: 1.5px solid #334155; border-radius: 14px; }"));
        heroLayout = new QVBoxLayout(leftHeroPanel);
        heroLayout->setSpacing(10);
        heroLayout->setObjectName("heroLayout");
        heroLayout->setContentsMargins(20, 20, 20, 20);
        heroBadge = new QLabel(leftHeroPanel);
        heroBadge->setObjectName("heroBadge");
        heroBadge->setStyleSheet(QString::fromUtf8("color: #10b981; font-size: 10px; font-weight: 800; letter-spacing: 1px; background: rgba(16, 185, 129, 0.12); border-radius: 4px; padding: 3px 8px;"));

        heroLayout->addWidget(heroBadge);

        heroTitle = new QLabel(leftHeroPanel);
        heroTitle->setObjectName("heroTitle");
        heroTitle->setStyleSheet(QString::fromUtf8("color: #ffffff; font-size: 16px; font-weight: 900; line-height: 1.3;"));

        heroLayout->addWidget(heroTitle);

        heroSubtitle = new QLabel(leftHeroPanel);
        heroSubtitle->setObjectName("heroSubtitle");
        heroSubtitle->setStyleSheet(QString::fromUtf8("color: #94a3b8; font-size: 11px;"));

        heroLayout->addWidget(heroSubtitle);

        heroGap = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

        heroLayout->addItem(heroGap);

        featureCard1 = new QFrame(leftHeroPanel);
        featureCard1->setObjectName("featureCard1");
        featureCard1->setStyleSheet(QString::fromUtf8("background: rgba(30, 41, 59, 0.6); border: 1px solid #334155; border-radius: 8px; padding: 6px 10px;"));
        f1Layout = new QHBoxLayout(featureCard1);
        f1Layout->setSpacing(8);
        f1Layout->setContentsMargins(0, 0, 0, 0);
        f1Layout->setObjectName("f1Layout");
        f1Icon = new QLabel(featureCard1);
        f1Icon->setObjectName("f1Icon");
        f1Icon->setStyleSheet(QString::fromUtf8("font-size: 16px; color: #10b981;"));

        f1Layout->addWidget(f1Icon);

        f1Text = new QLabel(featureCard1);
        f1Text->setObjectName("f1Text");
        f1Text->setStyleSheet(QString::fromUtf8("color: #e2e8f0; font-size: 11px; font-weight: 600;"));

        f1Layout->addWidget(f1Text);


        heroLayout->addWidget(featureCard1);

        featureCard2 = new QFrame(leftHeroPanel);
        featureCard2->setObjectName("featureCard2");
        featureCard2->setStyleSheet(QString::fromUtf8("background: rgba(30, 41, 59, 0.6); border: 1px solid #334155; border-radius: 8px; padding: 6px 10px;"));
        f2Layout = new QHBoxLayout(featureCard2);
        f2Layout->setSpacing(8);
        f2Layout->setContentsMargins(0, 0, 0, 0);
        f2Layout->setObjectName("f2Layout");
        f2Icon = new QLabel(featureCard2);
        f2Icon->setObjectName("f2Icon");
        f2Icon->setStyleSheet(QString::fromUtf8("font-size: 16px; color: #ef4444;"));

        f2Layout->addWidget(f2Icon);

        f2Text = new QLabel(featureCard2);
        f2Text->setObjectName("f2Text");
        f2Text->setStyleSheet(QString::fromUtf8("color: #e2e8f0; font-size: 11px; font-weight: 600;"));

        f2Layout->addWidget(f2Text);


        heroLayout->addWidget(featureCard2);

        featureCard3 = new QFrame(leftHeroPanel);
        featureCard3->setObjectName("featureCard3");
        featureCard3->setStyleSheet(QString::fromUtf8("background: rgba(30, 41, 59, 0.6); border: 1px solid #334155; border-radius: 8px; padding: 6px 10px;"));
        f3Layout = new QHBoxLayout(featureCard3);
        f3Layout->setSpacing(8);
        f3Layout->setContentsMargins(0, 0, 0, 0);
        f3Layout->setObjectName("f3Layout");
        f3Icon = new QLabel(featureCard3);
        f3Icon->setObjectName("f3Icon");
        f3Icon->setStyleSheet(QString::fromUtf8("font-size: 16px; color: #06b6d4;"));

        f3Layout->addWidget(f3Icon);

        f3Text = new QLabel(featureCard3);
        f3Text->setObjectName("f3Text");
        f3Text->setStyleSheet(QString::fromUtf8("color: #e2e8f0; font-size: 11px; font-weight: 600;"));

        f3Layout->addWidget(f3Text);


        heroLayout->addWidget(featureCard3);

        heroBottomSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

        heroLayout->addItem(heroBottomSpacer);

        heroFooterStatus = new QLabel(leftHeroPanel);
        heroFooterStatus->setObjectName("heroFooterStatus");
        heroFooterStatus->setStyleSheet(QString::fromUtf8("color: #64748b; font-size: 9px;"));

        heroLayout->addWidget(heroFooterStatus);


        contentRowLayout->addWidget(leftHeroPanel);

        loginCard = new QFrame(LoginPage);
        loginCard->setObjectName("loginCard");
        loginCard->setStyleSheet(QString::fromUtf8("QFrame#loginCard { background-color: #111a2e; border: 1.5px solid #1e293b; border-radius: 14px; }"));
        loginFormLayout = new QVBoxLayout(loginCard);
        loginFormLayout->setSpacing(12);
        loginFormLayout->setObjectName("loginFormLayout");
        loginFormLayout->setContentsMargins(24, 20, 24, 20);
        loginTitle = new QLabel(loginCard);
        loginTitle->setObjectName("loginTitle");
        loginTitle->setStyleSheet(QString::fromUtf8("color: #ffffff; font-size: 16px; font-weight: 800;"));

        loginFormLayout->addWidget(loginTitle, 0, Qt::AlignLeft);

        loginSubtitle = new QLabel(loginCard);
        loginSubtitle->setObjectName("loginSubtitle");
        loginSubtitle->setStyleSheet(QString::fromUtf8("color: #64748b; font-size: 11px;"));

        loginFormLayout->addWidget(loginSubtitle, 0, Qt::AlignLeft);

        userInputRow = new QHBoxLayout();
        userInputRow->setSpacing(8);
        userInputRow->setObjectName("userInputRow");
        userIconLabel = new QLabel(loginCard);
        userIconLabel->setObjectName("userIconLabel");
        userIconLabel->setStyleSheet(QString::fromUtf8("font-size: 14px; color: #94a3b8;"));

        userInputRow->addWidget(userIconLabel);

        usernameEdit = new QLineEdit(loginCard);
        usernameEdit->setObjectName("usernameEdit");
        usernameEdit->setMinimumSize(QSize(0, 38));
        usernameEdit->setStyleSheet(QString::fromUtf8("QLineEdit { background-color: #0d1322; border: 1.5px solid #1e293b; border-radius: 8px; color: #ffffff; font-size: 12px; padding: 4px 10px; } QLineEdit:focus { border: 1.5px solid #10b981; }"));
        usernameEdit->setClearButtonEnabled(true);

        userInputRow->addWidget(usernameEdit);


        loginFormLayout->addLayout(userInputRow);

        passInputRow = new QHBoxLayout();
        passInputRow->setSpacing(8);
        passInputRow->setObjectName("passInputRow");
        passIconLabel = new QLabel(loginCard);
        passIconLabel->setObjectName("passIconLabel");
        passIconLabel->setStyleSheet(QString::fromUtf8("font-size: 14px; color: #94a3b8;"));

        passInputRow->addWidget(passIconLabel);

        passwordEdit = new QLineEdit(loginCard);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setMinimumSize(QSize(0, 38));
        passwordEdit->setEchoMode(QLineEdit::Password);
        passwordEdit->setStyleSheet(QString::fromUtf8("QLineEdit { background-color: #0d1322; border: 1.5px solid #1e293b; border-radius: 8px; color: #ffffff; font-size: 12px; padding: 4px 10px; } QLineEdit:focus { border: 1.5px solid #10b981; }"));

        passInputRow->addWidget(passwordEdit);

        togglePassBtn = new QPushButton(loginCard);
        togglePassBtn->setObjectName("togglePassBtn");
        togglePassBtn->setMinimumSize(QSize(36, 36));
        togglePassBtn->setCursor(QCursor(Qt::PointingHandCursor));
        togglePassBtn->setCheckable(true);
        togglePassBtn->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #1e293b; border: none; border-radius: 6px; color: #94a3b8; font-size: 13px; } QPushButton:hover { background-color: #334155; }"));

        passInputRow->addWidget(togglePassBtn);


        loginFormLayout->addLayout(passInputRow);

        optionsRow = new QHBoxLayout();
        optionsRow->setObjectName("optionsRow");
        rememberAccountCheckBox = new QCheckBox(loginCard);
        rememberAccountCheckBox->setObjectName("rememberAccountCheckBox");
        rememberAccountCheckBox->setCursor(QCursor(Qt::PointingHandCursor));
        rememberAccountCheckBox->setStyleSheet(QString::fromUtf8("QCheckBox { color: #94a3b8; font-size: 11px; } QCheckBox::indicator { width: 14px; height: 14px; }"));

        optionsRow->addWidget(rememberAccountCheckBox);

        optSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        optionsRow->addItem(optSpacer);

        quickFillAdminBtn = new QPushButton(loginCard);
        quickFillAdminBtn->setObjectName("quickFillAdminBtn");
        quickFillAdminBtn->setCursor(QCursor(Qt::PointingHandCursor));
        quickFillAdminBtn->setStyleSheet(QString::fromUtf8("QPushButton { background: transparent; border: none; color: #38bdf8; font-size: 10px; font-weight: 700; text-decoration: underline; }"));

        optionsRow->addWidget(quickFillAdminBtn);


        loginFormLayout->addLayout(optionsRow);

        loginButton = new QPushButton(loginCard);
        loginButton->setObjectName("loginButton");
        loginButton->setMinimumSize(QSize(0, 42));
        loginButton->setCursor(QCursor(Qt::PointingHandCursor));
        loginButton->setStyleSheet(QString::fromUtf8("QPushButton#loginButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #10b981, stop:1 #059669); color: #ffffff; border: none; border-radius: 8px; font-size: 13px; font-weight: 800; letter-spacing: 0.5px; } QPushButton#loginButton:hover { background: #10b981; } QPushButton#loginButton:pressed { background: #047857; }"));

        loginFormLayout->addWidget(loginButton);

        keyboardToggleHint = new QLabel(loginCard);
        keyboardToggleHint->setObjectName("keyboardToggleHint");
        keyboardToggleHint->setStyleSheet(QString::fromUtf8("color: #475569; font-size: 10px;"));

        loginFormLayout->addWidget(keyboardToggleHint, 0, Qt::AlignCenter);


        contentRowLayout->addWidget(loginCard);


        pageLayout->addLayout(contentRowLayout);

        keyboardContainerLayout = new QHBoxLayout();
        keyboardContainerLayout->setObjectName("keyboardContainerLayout");
        keyboardContainerLayout->setContentsMargins(0, 4, 0, 0);

        pageLayout->addLayout(keyboardContainerLayout);


        retranslateUi(LoginPage);

        loginButton->setDefault(true);


        QMetaObject::connectSlotsByName(LoginPage);
    } // setupUi

    void retranslateUi(QWidget *LoginPage)
    {
        heroBadge->setText(QCoreApplication::translate("LoginPage", "\360\237\233\241\357\270\217 ACCESS SECURITY TERMINAL", nullptr));
        heroTitle->setText(QCoreApplication::translate("LoginPage", "H\341\273\206 TH\341\273\220NG C\341\273\254A T\341\273\260 \304\220\341\273\230NG\n"
"MANH QUANG SMART DOOR", nullptr));
        heroSubtitle->setText(QCoreApplication::translate("LoginPage", "\304\220i\341\273\201u khi\341\273\203n \304\221\341\273\213nh v\341\273\213 th\303\264ng minh & Gi\303\241m s\303\241t an to\303\240n \304\221a l\341\273\233p", nullptr));
        f1Icon->setText(QCoreApplication::translate("LoginPage", "\342\232\241", nullptr));
        f1Text->setText(QCoreApplication::translate("LoginPage", "C\341\272\243m bi\341\272\277n radar chuy\341\273\203n \304\221\341\273\231ng SR602", nullptr));
        f2Icon->setText(QCoreApplication::translate("LoginPage", "\360\237\232\250", nullptr));
        f2Text->setText(QCoreApplication::translate("LoginPage", "C\341\272\243m bi\341\272\277n IR ch\303\271m tia ch\341\273\221ng k\341\272\271t an to\303\240n", nullptr));
        f3Icon->setText(QCoreApplication::translate("LoginPage", "\342\232\231\357\270\217", nullptr));
        f3Text->setText(QCoreApplication::translate("LoginPage", "\304\220\341\273\231ng c\306\241 b\306\260\341\273\233c \304\221\341\273\213nh v\341\273\213 vi b\306\260\341\273\233c m\306\260\341\273\243t m\303\240", nullptr));
        heroFooterStatus->setText(QCoreApplication::translate("LoginPage", "\360\237\237\242 H\341\273\207 th\341\273\221ng s\341\272\265n s\303\240ng k\341\272\277t n\341\273\221i MQTT / IoT v1", nullptr));
        loginTitle->setText(QCoreApplication::translate("LoginPage", "X\303\241c th\341\273\261c quy\341\273\201n truy c\341\272\255p", nullptr));
        loginSubtitle->setText(QCoreApplication::translate("LoginPage", "Nh\341\272\255p t\303\240i kho\341\272\243n qu\341\272\243n tr\341\273\213 ho\341\272\267c ng\306\260\341\273\235i v\341\272\255n h\303\240nh", nullptr));
        userIconLabel->setText(QCoreApplication::translate("LoginPage", "\360\237\221\244", nullptr));
        usernameEdit->setPlaceholderText(QCoreApplication::translate("LoginPage", "T\303\240i kho\341\272\243n (admin / admin1 / operator)", nullptr));
        passIconLabel->setText(QCoreApplication::translate("LoginPage", "\360\237\224\222", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("LoginPage", "M\341\272\255t kh\341\272\251u", nullptr));
        togglePassBtn->setText(QCoreApplication::translate("LoginPage", "\360\237\221\201\357\270\217", nullptr));
        rememberAccountCheckBox->setText(QCoreApplication::translate("LoginPage", "Ghi nh\341\273\233 t\303\240i kho\341\272\243n", nullptr));
        quickFillAdminBtn->setText(QCoreApplication::translate("LoginPage", "\360\237\224\221 Admin 1-Click", nullptr));
        loginButton->setText(QCoreApplication::translate("LoginPage", "\360\237\232\252 \304\220\304\202NG NH\341\272\254P V\341\272\254N H\303\200NH", nullptr));
        keyboardToggleHint->setText(QCoreApplication::translate("LoginPage", "Ch\341\272\241m v\303\240o \303\264 nh\341\272\255p \304\221\341\273\203 m\341\273\237 b\303\240n ph\303\255m \341\272\243o", nullptr));
        (void)LoginPage;
    } // retranslateUi

};

namespace Ui {
    class LoginPage: public Ui_LoginPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINPAGE_H
