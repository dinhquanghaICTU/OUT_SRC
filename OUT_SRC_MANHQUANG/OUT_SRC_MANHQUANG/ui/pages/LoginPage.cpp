#include "LoginPage.h"

#include "VirtualKeyboard.h"
#include "ui_LoginPage.h"

#include <QApplication>
#include <QSettings>

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::LoginPage)
{
    ui->setupUi(this);

    QSettings settings;
    const bool rememberAccount = settings.value(
        QStringLiteral("login/rememberAccount"), true).toBool();
    ui->rememberAccountCheckBox->setChecked(rememberAccount);
    if (rememberAccount) {
        ui->usernameEdit->setText(
            settings.value(QStringLiteral("login/username"), QStringLiteral("admin")).toString());
        ui->passwordEdit->setText(QStringLiteral("admin123"));
        ui->passwordEdit->setFocus();
    } else {
        ui->usernameEdit->setFocus();
    }

    connect(ui->togglePassBtn, &QPushButton::toggled, this, [this](bool checked) {
        ui->passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        ui->togglePassBtn->setText(checked ? QStringLiteral("🙈") : QStringLiteral("👁️"));
    });

    connect(ui->quickFillAdminBtn, &QPushButton::clicked, this, [this] {
        ui->usernameEdit->setText(QStringLiteral("admin"));
        ui->passwordEdit->setText(QStringLiteral("admin123"));
        ui->loginButton->click();
    });

    connect(ui->loginButton, &QPushButton::clicked, this, [this] {
        QSettings settings;
        if (ui->rememberAccountCheckBox->isChecked()) {
            settings.setValue(QStringLiteral("login/rememberAccount"), true);
            settings.setValue(QStringLiteral("login/username"), ui->usernameEdit->text().trimmed());
        } else {
            settings.remove(QStringLiteral("login/rememberAccount"));
            settings.remove(QStringLiteral("login/username"));
        }

        emit loginRequested(ui->usernameEdit->text(), ui->passwordEdit->text());
    });

    connect(ui->usernameEdit, &QLineEdit::returnPressed, ui->loginButton, &QPushButton::click);
    connect(ui->passwordEdit, &QLineEdit::returnPressed, ui->loginButton, &QPushButton::click);

    // On-screen Touch Keyboard
    m_keyboard = new VirtualKeyboard(this);
    ui->keyboardContainerLayout->addWidget(m_keyboard);
    m_keyboard->hide();

    auto showKb = [this](QLineEdit *target) {
        m_keyboard->attachTo(target);
        m_keyboard->show();
    };

    struct InputClickFilter : public QObject {
        std::function<void()> onClick;
        InputClickFilter(QObject *parent, std::function<void()> cb) : QObject(parent), onClick(cb) {}
        bool eventFilter(QObject *watched, QEvent *event) override {
            if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::FocusIn) {
                if (onClick) onClick();
            }
            return QObject::eventFilter(watched, event);
        }
    };

    ui->usernameEdit->installEventFilter(new InputClickFilter(ui->usernameEdit, [showKb, this] { showKb(ui->usernameEdit); }));
    ui->passwordEdit->installEventFilter(new InputClickFilter(ui->passwordEdit, [showKb, this] { showKb(ui->passwordEdit); }));

    connect(m_keyboard, &VirtualKeyboard::enterPressed, ui->loginButton, &QPushButton::click);
    connect(m_keyboard, &VirtualKeyboard::hideRequested, this, [this] { m_keyboard->hide(); });

    connect(qApp, &QApplication::focusChanged, this, [this](QWidget *, QWidget *current) {
        if (!isVisible()) {
            m_keyboard->hide();
            return;
        }
        auto *target = qobject_cast<QLineEdit *>(current);
        if (target == ui->usernameEdit || target == ui->passwordEdit) {
            m_keyboard->attachTo(target);
            m_keyboard->show();
        }
    });
}

void LoginPage::hideEvent(QHideEvent *event)
{
    if (m_keyboard)
        m_keyboard->hide();
    QWidget::hideEvent(event);
}

LoginPage::~LoginPage()
{
    delete ui;
}
