#include "LoginPage.h"
#include "VirtualKeyboard.h"

#include "ui_LoginPage.h"

#include <QAction>
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
    VirtualKeyboardDialog::attachToLineEdit(ui->usernameEdit, tr("Nhập tên tài khoản"));
    VirtualKeyboardDialog::attachToLineEdit(ui->passwordEdit, tr("Nhập mật khẩu"));

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
    connect(ui->usernameEdit, &QLineEdit::returnPressed,
            ui->loginButton, &QPushButton::click);
    connect(ui->passwordEdit, &QLineEdit::returnPressed,
            ui->loginButton, &QPushButton::click);
}

LoginPage::~LoginPage()
{
    delete ui;
}
