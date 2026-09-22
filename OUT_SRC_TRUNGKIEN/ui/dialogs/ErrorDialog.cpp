#include "ErrorDialog.h"

#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ErrorDialog::ErrorDialog(const QString &title,
                         const QString &message,
                         const QString &details,
                         QWidget *parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("errorDialog"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    setFixedSize(440, 250);

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(16, 16, 16, 16);

    auto *card = new QWidget(this);
    card->setObjectName(QStringLiteral("errorCard"));
    card->setAttribute(Qt::WA_StyledBackground, true);
    card->setStyleSheet(QStringLiteral(
        "QWidget#errorCard { background-color: #0b152d; border: 2px solid #ef4444; border-radius: 12px; }"
        "QLabel#errorIcon { background-color: #dc2626; color: #ffffff; font-size: 20px; font-weight: 900; border-radius: 18px; border: 2px solid #f87171; }"
        "QLabel#errorTitle { color: #ef4444; font-size: 15px; font-weight: 900; }"
        "QLabel#errorMessage { color: #f8fafc; font-size: 13px; font-weight: 700; }"
        "QLabel#errorDetails { color: #fca5a5; background-color: #1a1024; border: 1px solid #7f1d1d; border-radius: 6px; padding: 4px 8px; font-size: 11px; }"
        "QPushButton#dialogDismissButton { background-color: #dc2626; color: #ffffff; border: 1px solid #f87171; border-radius: 6px; font-size: 12px; font-weight: 800; }"
        "QPushButton#dialogDismissButton:hover { background-color: #ef4444; border-color: #ffffff; }"
        "QPushButton#dialogDismissButton:pressed { background-color: #b91c1c; }"
        "QPushButton#dialogCloseButton { background: transparent; color: #94a3b8; border: none; font-size: 20px; font-weight: bold; border-radius: 14px; }"
        "QPushButton#dialogCloseButton:hover { color: #ffffff; background-color: #1e293b; }"
    ));

    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(28);
    shadow->setOffset(0, 7);
    shadow->setColor(QColor(0, 0, 0, 180));
    card->setGraphicsEffect(shadow);
    outerLayout->addWidget(card);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(22, 16, 22, 18);
    cardLayout->setSpacing(10);

    auto *topLayout = new QHBoxLayout;
    auto *iconLabel = new QLabel(QStringLiteral("!"), card);
    iconLabel->setObjectName(QStringLiteral("errorIcon"));
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(36, 36);

    auto *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName(QStringLiteral("errorTitle"));

    auto *closeButton = new QPushButton(QStringLiteral("×"), card);
    closeButton->setObjectName(QStringLiteral("dialogCloseButton"));
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setFixedSize(28, 28);

    topLayout->addWidget(iconLabel);
    topLayout->addSpacing(10);
    topLayout->addWidget(titleLabel, 1);
    topLayout->addWidget(closeButton);
    cardLayout->addLayout(topLayout);

    auto *messageLabel = new QLabel(message, card);
    messageLabel->setObjectName(QStringLiteral("errorMessage"));
    messageLabel->setWordWrap(true);
    messageLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(messageLabel);

    if (!details.isEmpty() && details != message) {
        auto *detailsLabel = new QLabel(details, card);
        detailsLabel->setObjectName(QStringLiteral("errorDetails"));
        detailsLabel->setWordWrap(true);
        detailsLabel->setAlignment(Qt::AlignCenter);
        cardLayout->addWidget(detailsLabel);
    }

    cardLayout->addStretch();

    auto *buttonLayout = new QHBoxLayout;
    auto *dismissButton = new QPushButton(tr("ĐÃ HIỂU"), card);
    dismissButton->setObjectName(QStringLiteral("dialogDismissButton"));
    dismissButton->setCursor(Qt::PointingHandCursor);
    dismissButton->setFixedSize(140, 36);
    dismissButton->setDefault(true);
    buttonLayout->addStretch();
    buttonLayout->addWidget(dismissButton);
    buttonLayout->addStretch();
    cardLayout->addLayout(buttonLayout);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(dismissButton, &QPushButton::clicked, this, &QDialog::accept);
}

void ErrorDialog::showLoginError(QWidget *parent, const QString &technicalMessage)
{
    QString title = tr("Đăng nhập thất bại");
    QString userMessage;

    if (technicalMessage.contains(tr("mật khẩu"), Qt::CaseInsensitive) ||
        technicalMessage.contains(tr("tài khoản"), Qt::CaseInsensitive) ||
        technicalMessage.contains(QStringLiteral("password"), Qt::CaseInsensitive) ||
        technicalMessage.contains(QStringLiteral("credentials"), Qt::CaseInsensitive) ||
        technicalMessage.contains(QStringLiteral("unauthorized"), Qt::CaseInsensitive) ||
        technicalMessage.contains(QStringLiteral("401"), Qt::CaseInsensitive) ||
        technicalMessage.contains(QStringLiteral("không đúng"), Qt::CaseInsensitive)) {
        userMessage = tr("Tài khoản hoặc mật khẩu không chính xác!\nVui lòng kiểm tra lại thông tin đăng nhập.");
    } else if (technicalMessage.trimmed().isEmpty()) {
        userMessage = tr("Tài khoản hoặc mật khẩu không chính xác!\nVui lòng kiểm tra lại thông tin đăng nhập.");
    } else {
        userMessage = tr("Không thể kết nối tới máy chủ Raspberry Pi.\nVui lòng kiểm tra mạng và dịch vụ API.");
    }

    ErrorDialog dialog(title, userMessage, technicalMessage, parent);
    dialog.exec();
}
