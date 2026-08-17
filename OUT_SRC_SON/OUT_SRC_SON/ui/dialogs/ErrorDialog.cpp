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
    setFixedSize(420, 250);

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(18, 18, 18, 18);

    auto *card = new QWidget(this);
    card->setObjectName(QStringLiteral("errorCard"));
    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(28);
    shadow->setOffset(0, 7);
    shadow->setColor(QColor(0, 35, 55, 75));
    card->setGraphicsEffect(shadow);
    outerLayout->addWidget(card);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 20, 28, 22);
    cardLayout->setSpacing(10);

    auto *topLayout = new QHBoxLayout;
    auto *iconLabel = new QLabel(QStringLiteral("!"), card);
    iconLabel->setObjectName(QStringLiteral("errorIcon"));
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(42, 42);

    auto *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName(QStringLiteral("errorTitle"));

    auto *closeButton = new QPushButton(QStringLiteral("×"), card);
    closeButton->setObjectName(QStringLiteral("dialogCloseButton"));
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setFixedSize(30, 30);

    topLayout->addWidget(iconLabel);
    topLayout->addSpacing(12);
    topLayout->addWidget(titleLabel, 1);
    topLayout->addWidget(closeButton);
    cardLayout->addLayout(topLayout);

    auto *messageLabel = new QLabel(message, card);
    messageLabel->setObjectName(QStringLiteral("errorMessage"));
    messageLabel->setWordWrap(true);
    messageLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(messageLabel);

    if (!details.isEmpty()) {
        auto *detailsLabel = new QLabel(details, card);
        detailsLabel->setObjectName(QStringLiteral("errorDetails"));
        detailsLabel->setWordWrap(true);
        detailsLabel->setAlignment(Qt::AlignCenter);
        cardLayout->addWidget(detailsLabel);
    }

    cardLayout->addStretch();

    auto *buttonLayout = new QHBoxLayout;
    auto *dismissButton = new QPushButton(tr("ĐÓNG"), card);
    dismissButton->setObjectName(QStringLiteral("dialogDismissButton"));
    dismissButton->setCursor(Qt::PointingHandCursor);
    dismissButton->setFixedSize(140, 40);
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
    ErrorDialog dialog(
        tr("Không thể đăng nhập"),
        tr("Không thể kết nối tới máy chủ Raspberry Pi.\n"
           "Vui lòng kiểm tra mạng và dịch vụ API."),
        technicalMessage,
        parent);
    dialog.exec();
}
