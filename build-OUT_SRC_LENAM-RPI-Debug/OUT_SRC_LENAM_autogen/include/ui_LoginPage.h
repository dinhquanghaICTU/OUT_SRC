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
    QFrame *splitLoginCard;
    QHBoxLayout *splitLayout;
    QFrame *loginArtPanel;
    QVBoxLayout *artLayout;
    QLabel *artTopDots;
    QLabel *illustrationLabel;
    QLabel *artTitleLabel;
    QLabel *artHintLabel;
    QFrame *loginFormPanel;
    QVBoxLayout *loginFormLayout;
    QLabel *welcomeLabel;
    QLabel *subtitleLabel;
    QSpacerItem *formTopGap;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QCheckBox *rememberAccountCheckBox;
    QHBoxLayout *buttonLayout;
    QPushButton *demoButton;
    QPushButton *loginButton;
    QLabel *footerHintLabel;
    QSpacerItem *bottomStretch;

    void setupUi(QWidget *LoginPage)
    {
        if (LoginPage->objectName().isEmpty())
            LoginPage->setObjectName("LoginPage");
        pageLayout = new QGridLayout(LoginPage);
        pageLayout->setObjectName("pageLayout");
        pageLayout->setContentsMargins(16, 12, 16, 12);
        splitLoginCard = new QFrame(LoginPage);
        splitLoginCard->setObjectName("splitLoginCard");
        splitLoginCard->setMinimumSize(QSize(0, 0));
        splitLoginCard->setMaximumSize(QSize(920, 520));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(splitLoginCard->sizePolicy().hasHeightForWidth());
        splitLoginCard->setSizePolicy(sizePolicy);
        splitLayout = new QHBoxLayout(splitLoginCard);
        splitLayout->setSpacing(0);
        splitLayout->setObjectName("splitLayout");
        splitLayout->setContentsMargins(0, 0, 0, 0);
        loginArtPanel = new QFrame(splitLoginCard);
        loginArtPanel->setObjectName("loginArtPanel");
        artLayout = new QVBoxLayout(loginArtPanel);
        artLayout->setObjectName("artLayout");
        artLayout->setContentsMargins(24, 22, 24, 22);
        artTopDots = new QLabel(loginArtPanel);
        artTopDots->setObjectName("artTopDots");
        artTopDots->setAlignment(Qt::AlignCenter);

        artLayout->addWidget(artTopDots);

        illustrationLabel = new QLabel(loginArtPanel);
        illustrationLabel->setObjectName("illustrationLabel");
        illustrationLabel->setMinimumSize(QSize(0, 120));
        illustrationLabel->setAlignment(Qt::AlignCenter);

        artLayout->addWidget(illustrationLabel);

        artTitleLabel = new QLabel(loginArtPanel);
        artTitleLabel->setObjectName("artTitleLabel");
        artTitleLabel->setAlignment(Qt::AlignCenter);

        artLayout->addWidget(artTitleLabel);

        artHintLabel = new QLabel(loginArtPanel);
        artHintLabel->setObjectName("artHintLabel");
        artHintLabel->setWordWrap(true);
        artHintLabel->setAlignment(Qt::AlignCenter);

        artLayout->addWidget(artHintLabel);


        splitLayout->addWidget(loginArtPanel);

        loginFormPanel = new QFrame(splitLoginCard);
        loginFormPanel->setObjectName("loginFormPanel");
        loginFormLayout = new QVBoxLayout(loginFormPanel);
        loginFormLayout->setSpacing(10);
        loginFormLayout->setObjectName("loginFormLayout");
        loginFormLayout->setContentsMargins(36, 28, 36, 24);
        welcomeLabel = new QLabel(loginFormPanel);
        welcomeLabel->setObjectName("welcomeLabel");
        welcomeLabel->setAlignment(Qt::AlignCenter);

        loginFormLayout->addWidget(welcomeLabel);

        subtitleLabel = new QLabel(loginFormPanel);
        subtitleLabel->setObjectName("subtitleLabel");
        subtitleLabel->setAlignment(Qt::AlignCenter);

        loginFormLayout->addWidget(subtitleLabel);

        formTopGap = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

        loginFormLayout->addItem(formTopGap);

        usernameEdit = new QLineEdit(loginFormPanel);
        usernameEdit->setObjectName("usernameEdit");
        usernameEdit->setMinimumSize(QSize(0, 42));
        usernameEdit->setClearButtonEnabled(true);

        loginFormLayout->addWidget(usernameEdit);

        passwordEdit = new QLineEdit(loginFormPanel);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setMinimumSize(QSize(0, 42));
        passwordEdit->setEchoMode(QLineEdit::Password);

        loginFormLayout->addWidget(passwordEdit);

        rememberAccountCheckBox = new QCheckBox(loginFormPanel);
        rememberAccountCheckBox->setObjectName("rememberAccountCheckBox");
        rememberAccountCheckBox->setCursor(QCursor(Qt::PointingHandCursor));

        loginFormLayout->addWidget(rememberAccountCheckBox);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(14);
        buttonLayout->setObjectName("buttonLayout");
        demoButton = new QPushButton(loginFormPanel);
        demoButton->setObjectName("demoButton");
        demoButton->setMinimumSize(QSize(0, 42));

        buttonLayout->addWidget(demoButton);

        loginButton = new QPushButton(loginFormPanel);
        loginButton->setObjectName("loginButton");
        loginButton->setMinimumSize(QSize(0, 42));
        loginButton->setCursor(QCursor(Qt::PointingHandCursor));

        buttonLayout->addWidget(loginButton);


        loginFormLayout->addLayout(buttonLayout);

        footerHintLabel = new QLabel(loginFormPanel);
        footerHintLabel->setObjectName("footerHintLabel");
        footerHintLabel->setAlignment(Qt::AlignCenter);

        loginFormLayout->addWidget(footerHintLabel);

        bottomStretch = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        loginFormLayout->addItem(bottomStretch);


        splitLayout->addWidget(loginFormPanel);


        pageLayout->addWidget(splitLoginCard, 0, 0, 1, 1, Qt::AlignCenter);


        retranslateUi(LoginPage);

        loginButton->setDefault(true);


        QMetaObject::connectSlotsByName(LoginPage);
    } // setupUi

    void retranslateUi(QWidget *LoginPage)
    {
        artTopDots->setText(QCoreApplication::translate("LoginPage", "\342\227\217 \342\227\217 \342\227\217 \342\227\217 \342\227\217 \342\227\217", nullptr));
        illustrationLabel->setText(QCoreApplication::translate("LoginPage", "\342\232\231", nullptr));
        artTitleLabel->setText(QCoreApplication::translate("LoginPage", "LeNam IoT Workspace", nullptr));
        artHintLabel->setText(QCoreApplication::translate("LoginPage", "Gi\303\241m s\303\241t c\341\272\243m bi\341\272\277n, \304\221i\341\273\201u khi\341\273\203n relay v\303\240 theo d\303\265i d\341\273\257 li\341\273\207u realtime qua server Raspberry Pi.", nullptr));
        welcomeLabel->setText(QCoreApplication::translate("LoginPage", "Welcome!", nullptr));
        subtitleLabel->setText(QCoreApplication::translate("LoginPage", "Sign in to LeNam Control Center", nullptr));
        usernameEdit->setPlaceholderText(QCoreApplication::translate("LoginPage", "\360\237\221\244  Username", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("LoginPage", "\360\237\224\222  Password", nullptr));
        rememberAccountCheckBox->setText(QCoreApplication::translate("LoginPage", "Remember account", nullptr));
        demoButton->setText(QCoreApplication::translate("LoginPage", "Demo", nullptr));
        loginButton->setText(QCoreApplication::translate("LoginPage", "Sign in", nullptr));
        footerHintLabel->setText(QCoreApplication::translate("LoginPage", "Press Enter to continue", nullptr));
        (void)LoginPage;
    } // retranslateUi

};

namespace Ui {
    class LoginPage: public Ui_LoginPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINPAGE_H
