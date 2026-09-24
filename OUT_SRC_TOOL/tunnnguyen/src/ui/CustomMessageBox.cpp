#include "ui/CustomMessageBox.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>

CustomMessageBox::CustomMessageBox(IconType type, const QString &title, const QString &message, QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    setupUi(type, title, message);
}

void CustomMessageBox::setupUi(IconType type, const QString &title, const QString &message)
{
    resize(380, 240);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(16, 16, 16, 16);

    // Card Container
    auto *card = new QWidget(this);
    card->setObjectName("alertCard");
    card->setStyleSheet(
        "#alertCard {"
        "   background-color: #ffffff;"
        "   border-radius: 20px;"
        "   border: 1px solid #e5e7eb;"
        "}"
    );

    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(28);
    shadow->setColor(QColor(0, 0, 0, 35));
    shadow->setOffset(0, 8);
    card->setGraphicsEffect(shadow);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 26, 24, 22);
    cardLayout->setSpacing(12);

    // Icon Badge Container (Circular)
    auto *iconBadge = new QLabel(card);
    iconBadge->setFixedSize(54, 54);
    iconBadge->setAlignment(Qt::AlignCenter);

    QString badgeStyle;
    QString iconText;

    switch (type) {
    case Warning:
        iconText = "⚠️";
        badgeStyle = 
            "QLabel {"
            "   background-color: #fffbeb;"
            "   border: 2px solid #fef3c7;"
            "   border-radius: 27px;"
            "   font-size: 24px;"
            "}";
        break;
    case Critical:
        iconText = "✕";
        badgeStyle = 
            "QLabel {"
            "   background-color: #fef2f2;"
            "   border: 2px solid #fee2e2;"
            "   border-radius: 27px;"
            "   color: #dc2626;"
            "   font-size: 22px;"
            "   font-weight: bold;"
            "}";
        break;
    case Information:
    default:
        iconText = "✓";
        badgeStyle = 
            "QLabel {"
            "   background-color: #f0fdf4;"
            "   border: 2px solid #dcfce7;"
            "   border-radius: 27px;"
            "   color: #16a34a;"
            "   font-size: 22px;"
            "   font-weight: bold;"
            "}";
        break;
    }

    iconBadge->setStyleSheet(badgeStyle);
    iconBadge->setText(iconText);

    auto *iconRow = new QHBoxLayout();
    iconRow->addStretch();
    iconRow->addWidget(iconBadge);
    iconRow->addStretch();
    cardLayout->addLayout(iconRow);

    // Title Label
    auto *titleLabel = new QLabel(title, card);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 17px; font-weight: 700; color: #111827; letter-spacing: -0.3px;");
    cardLayout->addWidget(titleLabel);

    // Message Label
    auto *msgLabel = new QLabel(message, card);
    msgLabel->setAlignment(Qt::AlignCenter);
    msgLabel->setWordWrap(true);
    msgLabel->setStyleSheet("font-size: 13px; color: #4b5563; line-height: 1.4;");
    cardLayout->addWidget(msgLabel);

    cardLayout->addSpacing(8);

    // Action Button
    auto *okBtn = new QPushButton("Đồng ý", card);
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setFixedHeight(40);
    okBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #3d7b53;"
        "   color: #ffffff;"
        "   border: none;"
        "   border-radius: 10px;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "   padding: 0 20px;"
        "}"
        "QPushButton:hover { background-color: #346947; }"
        "QPushButton:pressed { background-color: #2b563b; }"
    );
    connect(okBtn, &QPushButton::clicked, this, &CustomMessageBox::accept);

    cardLayout->addWidget(okBtn);

    rootLayout->addWidget(card);
}

void CustomMessageBox::warning(QWidget *parent, const QString &title, const QString &message)
{
    CustomMessageBox box(Warning, title, message, parent);
    box.exec();
}

void CustomMessageBox::information(QWidget *parent, const QString &title, const QString &message)
{
    CustomMessageBox box(Information, title, message, parent);
    box.exec();
}

void CustomMessageBox::critical(QWidget *parent, const QString &title, const QString &message)
{
    CustomMessageBox box(Critical, title, message, parent);
    box.exec();
}
