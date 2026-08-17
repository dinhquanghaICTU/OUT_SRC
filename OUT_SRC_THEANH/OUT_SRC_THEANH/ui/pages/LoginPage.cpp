#include "LoginPage.h"

#include "VirtualKeyboard.h"
#include "ui_LoginPage.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPolygonF>
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
        ui->passwordEdit->setFocus();
    } else {
        ui->usernameEdit->setFocus();
    }

    // Toggle password visibility via pill switch
    connect(ui->toggleVisibilityBtn, &QPushButton::toggled, this, [this](bool visible) {
        ui->passwordEdit->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
    });

    // Forgot password hint
    struct LabelClickFilter : public QObject {
        std::function<void()> onClick;
        LabelClickFilter(QObject *parent, std::function<void()> cb) : QObject(parent), onClick(cb) {}
        bool eventFilter(QObject *watched, QEvent *event) override {
            if (event->type() == QEvent::MouseButtonRelease) {
                if (onClick) onClick();
                return true;
            }
            return QObject::eventFilter(watched, event);
        }
    };
    ui->forgotPasswordLabel->installEventFilter(new LabelClickFilter(ui->forgotPasswordLabel, [this] {
        QMessageBox::information(this, tr("Quên mật khẩu"),
            tr("Tài khoản quản trị mặc định: admin / 1\nVui lòng liên hệ Quản trị viên để được cấp lại mật khẩu."));
    }));

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

        emit loginRequested(ui->usernameEdit->text().trimmed(), ui->passwordEdit->text());
    });
    connect(ui->usernameEdit, &QLineEdit::returnPressed,
            ui->loginButton, &QPushButton::click);
    connect(ui->passwordEdit, &QLineEdit::returnPressed,
            ui->loginButton, &QPushButton::click);

    // Virtual Keyboard integration (docked at the very bottom of the screen)
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

LoginPage::~LoginPage()
{
    delete ui;
}

void LoginPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();

    // 1. Base Deep Navy Background (Top & Center)
    painter.fillRect(rect(), QColor(QStringLiteral("#203546")));

    // 2. Mid Darker Navy Polygonal Layer (Top Left)
    QPainterPath topNavy;
    topNavy.moveTo(0, 0);
    topNavy.lineTo(w * 0.45, 0);
    topNavy.lineTo(0, h * 0.42);
    topNavy.closeSubpath();
    painter.fillPath(topNavy, QColor(QStringLiteral("#182936")));

    // 3. Right Teal / Cyan Ribbon (Top Right to Mid)
    QPainterPath tealRibbon;
    tealRibbon.moveTo(w * 0.78, 0);
    tealRibbon.lineTo(w, 0);
    tealRibbon.lineTo(w, h * 0.45);
    tealRibbon.lineTo(w * 0.72, h * 0.32);
    tealRibbon.closeSubpath();
    painter.fillPath(tealRibbon, QColor(QStringLiteral("#277d8c")));

    // 4. Bright Turquoise / Cyan Layer (Right to Center)
    QPainterPath cyanLayer;
    cyanLayer.moveTo(w * 0.72, h * 0.32);
    cyanLayer.lineTo(w, h * 0.45);
    cyanLayer.lineTo(w, h * 0.62);
    cyanLayer.lineTo(w * 0.35, h * 0.78);
    cyanLayer.closeSubpath();
    painter.fillPath(cyanLayer, QColor(QStringLiteral("#3eb7c3")));

    // 5. Left Teal Wedge
    QPainterPath leftTeal;
    leftTeal.moveTo(0, h * 0.38);
    leftTeal.lineTo(w * 0.35, h * 0.78);
    leftTeal.lineTo(0, h * 0.72);
    leftTeal.closeSubpath();
    painter.fillPath(leftTeal, QColor(QStringLiteral("#2597a7")));

    // 6. Warm Orange Layer (Bottom-Right diagonal band)
    QPainterPath orangeLayer;
    orangeLayer.moveTo(w, h * 0.58);
    orangeLayer.lineTo(w, h);
    orangeLayer.lineTo(w * 0.48, h);
    orangeLayer.closeSubpath();
    painter.fillPath(orangeLayer, QColor(QStringLiteral("#f05a24")));

    // 7. Darker Orange Shade Shadow
    QPainterPath darkOrange;
    darkOrange.moveTo(w * 0.48, h);
    darkOrange.lineTo(w * 0.32, h);
    darkOrange.lineTo(0, h * 0.68);
    darkOrange.lineTo(0, h * 0.72);
    darkOrange.closeSubpath();
    painter.fillPath(darkOrange, QColor(QStringLiteral("#c94212")));

    // 8. Soft Mint / Turquoise (Bottom-Left)
    QPainterPath mintBottom;
    mintBottom.moveTo(0, h * 0.72);
    mintBottom.lineTo(w * 0.48, h);
    mintBottom.lineTo(0, h);
    mintBottom.closeSubpath();
    painter.fillPath(mintBottom, QColor(QStringLiteral("#a2ded8")));
}

void LoginPage::hideEvent(QHideEvent *event)
{
    if (m_keyboard)
        m_keyboard->hide();
    QWidget::hideEvent(event);
}
