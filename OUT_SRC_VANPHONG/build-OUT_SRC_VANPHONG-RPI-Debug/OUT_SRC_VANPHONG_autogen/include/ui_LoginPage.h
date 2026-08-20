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
    QVBoxLayout *pageVLayout;
    QHBoxLayout *centerHLayout;
    QSpacerItem *leftPodSpacer;
    QFrame *bioDomeCard;
    QVBoxLayout *domeVLayout;
    QLabel *domeEmblem;
    QLabel *domeTitle;
    QLabel *domeSub;
    QHBoxLayout *miniStatusStrip;
    QLabel *pPill1;
    QLabel *pPill2;
    QLabel *pPill3;
    QLineEdit *usernameEdit;
    QHBoxLayout *passInputRow;
    QLineEdit *passwordEdit;
    QPushButton *togglePassBtn;
    QHBoxLayout *chipsRow;
    QPushButton *quickFillAdminBtn;
    QPushButton *quickFillOpBtn;
    QCheckBox *rememberAccountCheckBox;
    QPushButton *loginButton;
    QSpacerItem *rightPodSpacer;
    QHBoxLayout *keyboardContainerLayout;

    void setupUi(QWidget *LoginPage)
    {
        if (LoginPage->objectName().isEmpty())
            LoginPage->setObjectName("LoginPage");
        LoginPage->resize(800, 480);
        pageVLayout = new QVBoxLayout(LoginPage);
        pageVLayout->setSpacing(0);
        pageVLayout->setObjectName("pageVLayout");
        pageVLayout->setContentsMargins(0, 8, 0, 4);
        centerHLayout = new QHBoxLayout();
        centerHLayout->setSpacing(0);
        centerHLayout->setObjectName("centerHLayout");
        leftPodSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        centerHLayout->addItem(leftPodSpacer);

        bioDomeCard = new QFrame(LoginPage);
        bioDomeCard->setObjectName("bioDomeCard");
        bioDomeCard->setMinimumSize(QSize(440, 0));
        bioDomeCard->setMaximumSize(QSize(460, 16777215));
        bioDomeCard->setStyleSheet(QString::fromUtf8("QFrame#bioDomeCard { background: qradialgradient(cx:0.5, cy:0.2, radius:0.8, fx:0.5, fy:0.2, stop:0 #103322, stop:0.5 #091e14, stop:1 #05130b); border: 2px solid #10b981; border-radius: 24px; }"));
        domeVLayout = new QVBoxLayout(bioDomeCard);
        domeVLayout->setSpacing(10);
        domeVLayout->setObjectName("domeVLayout");
        domeVLayout->setContentsMargins(28, 18, 28, 18);
        domeEmblem = new QLabel(bioDomeCard);
        domeEmblem->setObjectName("domeEmblem");
        domeEmblem->setStyleSheet(QString::fromUtf8("font-size: 32px; background: rgba(16, 185, 129, 0.15); border: 1.5px solid #10b981; border-radius: 28px; padding: 6px 14px;"));

        domeVLayout->addWidget(domeEmblem, 0, Qt::AlignCenter);

        domeTitle = new QLabel(bioDomeCard);
        domeTitle->setObjectName("domeTitle");
        domeTitle->setStyleSheet(QString::fromUtf8("color: #34d399; font-size: 16px; font-weight: 900; letter-spacing: 1.5px;"));

        domeVLayout->addWidget(domeTitle, 0, Qt::AlignCenter);

        domeSub = new QLabel(bioDomeCard);
        domeSub->setObjectName("domeSub");
        domeSub->setStyleSheet(QString::fromUtf8("color: #a7f3d0; font-size: 10px; font-weight: 500;"));

        domeVLayout->addWidget(domeSub, 0, Qt::AlignCenter);

        miniStatusStrip = new QHBoxLayout();
        miniStatusStrip->setSpacing(8);
        miniStatusStrip->setObjectName("miniStatusStrip");
        pPill1 = new QLabel(bioDomeCard);
        pPill1->setObjectName("pPill1");
        pPill1->setStyleSheet(QString::fromUtf8("background: rgba(16, 185, 129, 0.15); color: #34d399; border: 1px solid #059669; border-radius: 12px; font-size: 10px; font-weight: 700; padding: 4px 10px;"));
        pPill1->setAlignment(Qt::AlignCenter);

        miniStatusStrip->addWidget(pPill1);

        pPill2 = new QLabel(bioDomeCard);
        pPill2->setObjectName("pPill2");
        pPill2->setStyleSheet(QString::fromUtf8("background: rgba(245, 158, 11, 0.15); color: #f59e0b; border: 1px solid #d97706; border-radius: 12px; font-size: 10px; font-weight: 700; padding: 4px 10px;"));
        pPill2->setAlignment(Qt::AlignCenter);

        miniStatusStrip->addWidget(pPill2);

        pPill3 = new QLabel(bioDomeCard);
        pPill3->setObjectName("pPill3");
        pPill3->setStyleSheet(QString::fromUtf8("background: rgba(56, 189, 248, 0.15); color: #38bdf8; border: 1px solid #0284c7; border-radius: 12px; font-size: 10px; font-weight: 700; padding: 4px 10px;"));
        pPill3->setAlignment(Qt::AlignCenter);

        miniStatusStrip->addWidget(pPill3);


        domeVLayout->addLayout(miniStatusStrip);

        usernameEdit = new QLineEdit(bioDomeCard);
        usernameEdit->setObjectName("usernameEdit");
        usernameEdit->setMinimumSize(QSize(0, 38));
        usernameEdit->setStyleSheet(QString::fromUtf8("QLineEdit { background-color: #041209; border: 1.5px solid #1b4332; border-radius: 19px; color: #ffffff; font-size: 12px; padding: 4px 16px; } QLineEdit:focus { border: 1.5px solid #10b981; background-color: #082012; }"));
        usernameEdit->setClearButtonEnabled(true);

        domeVLayout->addWidget(usernameEdit);

        passInputRow = new QHBoxLayout();
        passInputRow->setSpacing(6);
        passInputRow->setObjectName("passInputRow");
        passwordEdit = new QLineEdit(bioDomeCard);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setMinimumSize(QSize(0, 38));
        passwordEdit->setEchoMode(QLineEdit::Password);
        passwordEdit->setStyleSheet(QString::fromUtf8("QLineEdit { background-color: #041209; border: 1.5px solid #1b4332; border-radius: 19px; color: #ffffff; font-size: 12px; padding: 4px 16px; } QLineEdit:focus { border: 1.5px solid #10b981; background-color: #082012; }"));

        passInputRow->addWidget(passwordEdit);

        togglePassBtn = new QPushButton(bioDomeCard);
        togglePassBtn->setObjectName("togglePassBtn");
        togglePassBtn->setMinimumSize(QSize(38, 38));
        togglePassBtn->setCursor(QCursor(Qt::PointingHandCursor));
        togglePassBtn->setCheckable(true);
        togglePassBtn->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #041209; border: 1.5px solid #1b4332; border-radius: 19px; color: #94a3b8; font-size: 13px; } QPushButton:hover { background-color: #1b4332; }"));

        passInputRow->addWidget(togglePassBtn);


        domeVLayout->addLayout(passInputRow);

        chipsRow = new QHBoxLayout();
        chipsRow->setSpacing(6);
        chipsRow->setObjectName("chipsRow");
        quickFillAdminBtn = new QPushButton(bioDomeCard);
        quickFillAdminBtn->setObjectName("quickFillAdminBtn");
        quickFillAdminBtn->setCursor(QCursor(Qt::PointingHandCursor));
        quickFillAdminBtn->setStyleSheet(QString::fromUtf8("QPushButton { background: #0c281a; border: 1px solid #10b981; border-radius: 12px; color: #34d399; font-size: 10px; font-weight: 700; padding: 5px 10px; } QPushButton:hover { background: #10b981; color: white; }"));

        chipsRow->addWidget(quickFillAdminBtn);

        quickFillOpBtn = new QPushButton(bioDomeCard);
        quickFillOpBtn->setObjectName("quickFillOpBtn");
        quickFillOpBtn->setCursor(QCursor(Qt::PointingHandCursor));
        quickFillOpBtn->setStyleSheet(QString::fromUtf8("QPushButton { background: #0c281a; border: 1px solid #059669; border-radius: 12px; color: #a7f3d0; font-size: 10px; font-weight: 600; padding: 5px 10px; } QPushButton:hover { background: #059669; color: white; }"));

        chipsRow->addWidget(quickFillOpBtn);

        rememberAccountCheckBox = new QCheckBox(bioDomeCard);
        rememberAccountCheckBox->setObjectName("rememberAccountCheckBox");
        rememberAccountCheckBox->setCursor(QCursor(Qt::PointingHandCursor));
        rememberAccountCheckBox->setStyleSheet(QString::fromUtf8("QCheckBox { color: #a7f3d0; font-size: 10px; }"));

        chipsRow->addWidget(rememberAccountCheckBox);


        domeVLayout->addLayout(chipsRow);

        loginButton = new QPushButton(bioDomeCard);
        loginButton->setObjectName("loginButton");
        loginButton->setMinimumSize(QSize(0, 44));
        loginButton->setCursor(QCursor(Qt::PointingHandCursor));
        loginButton->setStyleSheet(QString::fromUtf8("QPushButton#loginButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #10b981, stop:0.5 #059669, stop:1 #047857); color: #ffffff; border: none; border-radius: 22px; font-size: 13px; font-weight: 900; letter-spacing: 1px; } QPushButton#loginButton:hover { background: #10b981; } QPushButton#loginButton:pressed { background: #065f46; }"));

        domeVLayout->addWidget(loginButton);


        centerHLayout->addWidget(bioDomeCard);

        rightPodSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        centerHLayout->addItem(rightPodSpacer);


        pageVLayout->addLayout(centerHLayout);

        keyboardContainerLayout = new QHBoxLayout();
        keyboardContainerLayout->setContentsMargins(0, 0, 0, 0);
        keyboardContainerLayout->setObjectName("keyboardContainerLayout");

        pageVLayout->addLayout(keyboardContainerLayout);


        retranslateUi(LoginPage);

        loginButton->setDefault(true);


        QMetaObject::connectSlotsByName(LoginPage);
    } // setupUi

    void retranslateUi(QWidget *LoginPage)
    {
        domeEmblem->setText(QCoreApplication::translate("LoginPage", "\360\237\214\277", nullptr));
        domeTitle->setText(QCoreApplication::translate("LoginPage", "VAN PHONG AGRI-POD", nullptr));
        domeSub->setText(QCoreApplication::translate("LoginPage", "H\341\273\207 Th\341\273\221ng T\306\260\341\273\233i N\306\260\341\273\233c Th\303\264ng Minh & C\341\272\243m Bi\341\272\277n Vi Kh\303\255 H\341\272\255u DHT11", nullptr));
        pPill1->setText(QCoreApplication::translate("LoginPage", "\360\237\214\276 \304\220\341\273\231 \341\272\251m \304\221\341\272\245t: 55%", nullptr));
        pPill2->setText(QCoreApplication::translate("LoginPage", "\360\237\214\241\357\270\217 DHT11: 27.5\302\260C", nullptr));
        pPill3->setText(QCoreApplication::translate("LoginPage", "\360\237\222\246 B\306\241m: S\341\272\265n s\303\240ng", nullptr));
        usernameEdit->setPlaceholderText(QCoreApplication::translate("LoginPage", "T\303\252n t\303\240i kho\341\272\243n (admin / admin1 / operator)", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("LoginPage", "M\341\272\255t kh\341\272\251u", nullptr));
        togglePassBtn->setText(QCoreApplication::translate("LoginPage", "\360\237\221\201\357\270\217", nullptr));
        quickFillAdminBtn->setText(QCoreApplication::translate("LoginPage", "\360\237\221\221 Qu\341\272\243n Tr\341\273\213 (Admin)", nullptr));
        quickFillOpBtn->setText(QCoreApplication::translate("LoginPage", "\360\237\221\250\342\200\215\360\237\214\276 V\341\272\255n H\303\240nh (Operator)", nullptr));
        rememberAccountCheckBox->setText(QCoreApplication::translate("LoginPage", "Ghi nh\341\273\233", nullptr));
        loginButton->setText(QCoreApplication::translate("LoginPage", "\360\237\214\261 K\303\215CH HO\341\272\240T H\341\273\206 TH\341\273\220NG T\306\257\341\273\232I V\306\257\341\273\234N", nullptr));
        (void)LoginPage;
    } // retranslateUi

};

namespace Ui {
    class LoginPage: public Ui_LoginPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINPAGE_H
