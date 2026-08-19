#include "ErrorDialog.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ErrorDialog::ErrorDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(16, 16, 16, 16);

    auto *card = new QFrame(this);
    card->setObjectName(QStringLiteral("errorDialogCard"));
    card->setStyleSheet(QStringLiteral(
        "QFrame#errorDialogCard { background-color: #0d2116; border: 1.5px solid #ef4444; border-radius: 12px; }"));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);

    auto *headerLayout = new QHBoxLayout;
    m_iconLabel = new QLabel(QStringLiteral("⚠️"), card);
    m_iconLabel->setStyleSheet(QStringLiteral("font-size: 24px;"));
    headerLayout->addWidget(m_iconLabel);

    m_titleLabel = new QLabel(tr("Thông báo hệ thống tưới"), card);
    m_titleLabel->setStyleSheet(QStringLiteral("color: #f87171; font-size: 15px; font-weight: 700;"));
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    layout->addLayout(headerLayout);

    m_messageLabel = new QLabel(card);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setStyleSheet(QStringLiteral("color: #e2e8f0; font-size: 13px;"));
    layout->addWidget(m_messageLabel);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    m_closeBtn = new QPushButton(tr("Đã hiểu"), card);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #ef4444; color: white; border: none; border-radius: 6px; padding: 8px 20px; font-weight: 600; } "
        "QPushButton:hover { background-color: #dc2626; }"));
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(m_closeBtn);
    layout->addLayout(btnLayout);

    rootLayout->addWidget(card);
    resize(360, 180);
}

void ErrorDialog::showLoginError(QWidget *parent, const QString &message)
{
    ErrorDialog dlg(parent);
    dlg.m_titleLabel->setText(tr("Đăng nhập không thành công"));
    dlg.m_messageLabel->setText(message);
    dlg.exec();
}

void ErrorDialog::showCustomError(QWidget *parent, const QString &title, const QString &message)
{
    ErrorDialog dlg(parent);
    dlg.m_titleLabel->setText(title);
    dlg.m_messageLabel->setText(message);
    dlg.exec();
}
