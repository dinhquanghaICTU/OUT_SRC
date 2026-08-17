#include "LoginPage.h"

#include "VirtualKeyboard.h"
#include "ui_LoginPage.h"

#include <QAction>
#include <QApplication>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QSettings>

namespace {
QIcon makePasswordVisibilityIcon(bool passwordVisible)
{
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(QStringLiteral("#66788A")), 1.8,
                        Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    QPainterPath eye;
    eye.moveTo(3, 12);
    eye.cubicTo(7, 6, 17, 6, 21, 12);
    eye.cubicTo(17, 18, 7, 18, 3, 12);
    painter.drawPath(eye);
    painter.drawEllipse(QPointF(12, 12), 2.7, 2.7);

    if (!passwordVisible)
        painter.drawLine(QPointF(4, 4), QPointF(20, 20));

    return QIcon(pixmap);
}
}

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::LoginPage)
{
    ui->setupUi(this);

    QSettings settings;
    const bool rememberAccount = settings.value(
        QStringLiteral("login/rememberAccount"), false).toBool();
    ui->rememberAccountCheckBox->setChecked(rememberAccount);
    if (rememberAccount) {
        ui->usernameEdit->setText(
            settings.value(QStringLiteral("login/username")).toString());
        ui->passwordEdit->setFocus();
    } else {
        ui->usernameEdit->setFocus();
    }

    auto *passwordVisibilityAction = new QAction(makePasswordVisibilityIcon(false),
                                                  tr("Hiện mật khẩu"),
                                                  ui->passwordEdit);
    passwordVisibilityAction->setCheckable(true);
    ui->passwordEdit->addAction(passwordVisibilityAction, QLineEdit::TrailingPosition);
    connect(passwordVisibilityAction, &QAction::toggled, this,
            [this, passwordVisibilityAction](bool visible) {
                ui->passwordEdit->setEchoMode(visible
                                                  ? QLineEdit::Normal
                                                  : QLineEdit::Password);
                passwordVisibilityAction->setIcon(makePasswordVisibilityIcon(visible));
                passwordVisibilityAction->setToolTip(visible
                                                          ? tr("Ẩn mật khẩu")
                                                          : tr("Hiện mật khẩu"));
            });

    connect(ui->loginButton, &QPushButton::clicked, this, [this] {
        QSettings settings;
        if (ui->rememberAccountCheckBox->isChecked()) {
            settings.setValue(QStringLiteral("login/rememberAccount"), true);
            settings.setValue(QStringLiteral("login/username"),
                              ui->usernameEdit->text().trimmed());
        } else {
            settings.remove(QStringLiteral("login/rememberAccount"));
            settings.remove(QStringLiteral("login/username"));
        }

        emit loginRequested(ui->usernameEdit->text(), ui->passwordEdit->text());
    });
    connect(ui->signinButton, &QPushButton::clicked,
            ui->loginButton, &QPushButton::click);
    connect(ui->usernameEdit, &QLineEdit::returnPressed,
            ui->loginButton, &QPushButton::click);
    connect(ui->passwordEdit, &QLineEdit::returnPressed,
            ui->loginButton, &QPushButton::click);

    m_keyboard = new VirtualKeyboard(this);
    auto *keyboardLayout = new QHBoxLayout;
    keyboardLayout->setContentsMargins(12, 0, 12, 8);
    keyboardLayout->addWidget(m_keyboard);
    ui->mainPageLayout->addLayout(keyboardLayout);
    m_keyboard->hide();

    auto showKb = [this](QLineEdit *target) {
        m_keyboard->attachTo(target);
        m_keyboard->show();
    };

    struct InputClickFilter : public QObject {
        std::function<void()> onClick;
        InputClickFilter(QObject *parent, std::function<void()> cb) : QObject(parent), onClick(cb) {}
        bool eventFilter(QObject *watched, QEvent *event) override {
            if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::FocusIn) {
                if (onClick) onClick();
            }
            return QObject::eventFilter(watched, event);
        }
    };

    ui->usernameEdit->installEventFilter(new InputClickFilter(ui->usernameEdit, [showKb, this] { showKb(ui->usernameEdit); }));
    ui->passwordEdit->installEventFilter(new InputClickFilter(ui->passwordEdit, [showKb, this] { showKb(ui->passwordEdit); }));

    connect(m_keyboard, &VirtualKeyboard::enterPressed,
            ui->loginButton, &QPushButton::click);
    connect(qApp, &QApplication::focusChanged, this,
            [this](QWidget *, QWidget *current) {
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
