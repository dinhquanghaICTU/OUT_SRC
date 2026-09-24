#include "ui/LoginDialog.h"
#include "ui/CustomMessageBox.h"
#include "auth/HwidHelper.h"

#include <QWidget>
#include <QPaintEvent>
#include <QPixmap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QLayout>
#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <QClipboard>
#include <QGuiApplication>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    m_hwid = HwidHelper::getHwid();
    setupUi();
}

void LoginDialog::setupUi()
{
    setWindowTitle("Welcome Back to TUNNGUYEN");
    resize(960, 640);
    setMinimumSize(850, 560);

    // Main background: soft gray/blue tint matching the photo aesthetic
    setStyleSheet("QDialog { background-color: #e9eef2; font-family: 'Google Sans', 'Product Sans', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // Central Card
    auto *cardWidget = new QWidget(this);
    cardWidget->setObjectName("cardWidget");
    cardWidget->setStyleSheet(
        "#cardWidget {"
        "   background-color: #ffffff;"
        "   border-radius: 24px;"
        "}"
    );

    // Add subtle drop shadow
    auto *shadow = new QGraphicsDropShadowEffect(cardWidget);
    shadow->setBlurRadius(35);
    shadow->setColor(QColor(0, 0, 0, 25));
    shadow->setOffset(0, 8);
    cardWidget->setGraphicsEffect(shadow);

    auto *cardLayout = new QHBoxLayout(cardWidget);
    cardLayout->setContentsMargins(40, 35, 35, 35);
    cardLayout->setSpacing(35);

    // Left Form
    QWidget *leftForm = createLeftForm();
    cardLayout->addWidget(leftForm, 1);

    // Right Banner
    QWidget *rightBanner = createRightBanner();
    cardLayout->addWidget(rightBanner, 1);

    mainLayout->addWidget(cardWidget);
}

QWidget* LoginDialog::createLeftForm()
{
    auto *container = new QWidget(this);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(14);

    layout->addStretch(1);

    // Title
    auto *titleLabel = new QLabel("Welcome back TUNNGUYEN", container);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: 800; color: #111827; letter-spacing: -0.5px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // Subtitle
    auto *subLabel = new QLabel("Login to continue your automation journey", container);
    subLabel->setStyleSheet("font-size: 13px; color: #6b7280; font-weight: 400;");
    subLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(subLabel);

    layout->addSpacing(8);

    // "OR" Divider
    auto *dividerRow = new QHBoxLayout();
    dividerRow->setSpacing(12);
    auto *line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setStyleSheet("color: #e5e7eb;");
    auto *orLabel = new QLabel("OR");
    orLabel->setStyleSheet("color: #9ca3af; font-size: 11px; font-weight: 600;");
    auto *line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setStyleSheet("color: #e5e7eb;");
    dividerRow->addWidget(line1, 1);
    dividerRow->addWidget(orLabel);
    dividerRow->addWidget(line2, 1);
    layout->addLayout(dividerRow);

    // Email / License Key input
    auto *emailLabel = new QLabel("License Key", container);
    emailLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #1f2937;");
    layout->addWidget(emailLabel);

    m_emailEdit = new QLineEdit(container);
    m_emailEdit->setPlaceholderText("KEY-XXXX");
    m_emailEdit->setStyleSheet(
        "QLineEdit {"
        "   background-color: #ffffff;"
        "   border: 1px solid #d1d5db;"
        "   border-radius: 10px;"
        "   padding: 11px 14px;"
        "   font-size: 13px;"
        "   color: #111827;"
        "}"
        "QLineEdit:focus {"
        "   border: 1.5px solid #3d7b53;"
        "}"
    );
    layout->addWidget(m_emailEdit);

    layout->addSpacing(8);

    // Login Button (Rich Green #3d7b53 matching reference)
    m_loginBtn = new QPushButton("Login", container);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #3d7b53;"
        "   color: #ffffff;"
        "   border: none;"
        "   border-radius: 10px;"
        "   padding: 13px;"
        "   font-size: 14px;"
        "   font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #346947; }"
        "QPushButton:pressed { background-color: #2b563b; }"
    );
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    layout->addWidget(m_loginBtn);

    // Footer link
    auto *footerLabel = new QLabel("Chưa acctive key? <a href='#signup'Hệ style='color:#3d7b53; font-weight:700; text-decoration:none;'>Liên Hệ</a>", container);
    footerLabel->setStyleSheet("font-size: 12px; color: #4b5563;");
    footerLabel->setAlignment(Qt::AlignCenter);
    footerLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    layout->addWidget(footerLabel);



    layout->addStretch(1);

    return container;
}

QWidget* LoginDialog::createRightBanner()
{
    QLabel *banner = new QLabel(this);
    banner->setObjectName("bannerLabel");
    banner->setMinimumWidth(320);
    banner->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    banner->setScaledContents(true);

    QString imagePath = ":/assets/banner.jpg";
    QPixmap srcPixmap(imagePath);
    if (srcPixmap.isNull()) {
        srcPixmap.load("assets/banner.jpg");
    }

    if (!srcPixmap.isNull()) {
        QPixmap rounded(srcPixmap.size());
        rounded.fill(Qt::transparent);
        QPainter painter(&rounded);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        QPainterPath path;
        path.addRoundedRect(rounded.rect(), 30, 30);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, srcPixmap);

        banner->setPixmap(rounded);
    } else {
        banner->setStyleSheet("background-color: #e5e7eb; border-radius: 20px;");
    }

    return banner;
}

void LoginDialog::onTogglePasswordVisibility()
{
    m_passwordVisible = !m_passwordVisible;
    if (m_passwordVisible) {
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
        m_togglePasswordAction->setText("🙈");
    } else {
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_togglePasswordAction->setText("👁");
    }
}

void LoginDialog::onLoginClicked()
{
    QString key = m_emailEdit->text().trimmed();
//    QString pass = m_passwordEdit->text();

    if (key.isEmpty()) {
        CustomMessageBox::warning(this, "Thông báo", "Vui lòng nhập License Key kích hoạt!");
        m_emailEdit->setFocus();
        return;
    }
    if (key == "1"){
        accept();
    }
    emit loginSubmitted(key);
    accept();
}

void LoginDialog::onCopyHwidClicked()
{
    QGuiApplication::clipboard()->setText(m_hwid);
    CustomMessageBox::information(this, "Đã sao chép", "Đã lưu mã HWID vào bộ nhớ tạm!\nBạn hãy gửi mã này cho Admin để kích hoạt.");
}

QString LoginDialog::getEmail() const
{
    return m_emailEdit ? m_emailEdit->text().trimmed() : "";
}

QString LoginDialog::getPassword() const
{
    return m_passwordEdit ? m_passwordEdit->text() : "";
}
