#include "ui/QuickProxyDialog.h"
#include "ui/ProxyInputWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

QuickProxyDialog::QuickProxyDialog(const QString &profileName, const QString &currentProxy, QWidget *parent)
    : QDialog(parent)
{
    setupUi(profileName, currentProxy);
}

void QuickProxyDialog::setupUi(const QString &profileName, const QString &currentProxy)
{
    setWindowTitle("Sửa Proxy Profile");
    setFixedSize(520, 340);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setStyleSheet(
        "* { font-family: 'Google Sans', 'Product Sans', -apple-system, sans-serif; }"
        "QDialog { background-color: #ffffff; border-radius: 16px; }"
        "QLineEdit, QSpinBox, QComboBox {"
        "   background-color: #f8fafc;"
        "   border: 1px solid #e2e8f0;"
        "   border-radius: 8px;"
        "   padding: 8px 12px;"
        "   font-size: 13px;"
        "   color: #1e293b;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border: 1.5px solid #2563eb; background: #ffffff; }"
        "QLabel { color: #334155; font-size: 13px; font-weight: 600; }"
    );

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(22, 20, 22, 20);
    mainLayout->setSpacing(14);

    // Title Row
    auto *titleRow = new QHBoxLayout();
    auto *iconLbl = new QLabel("🌐");
    iconLbl->setStyleSheet("font-size: 20px; background: #dbeafe; padding: 6px; border-radius: 10px;");
    auto *titleText = new QLabel(QString("Sửa Proxy: %1").arg(profileName));
    titleText->setStyleSheet("font-size: 16px; font-weight: 800; color: #0f172a;");
    titleRow->addWidget(iconLbl);
    titleRow->addWidget(titleText);
    titleRow->addStretch();
    mainLayout->addLayout(titleRow);

    auto *divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("color: #f1f5f9;");
    mainLayout->addWidget(divider);

    // Proxy Input Widget with divided fields (Giao thức, Host, Port, User, Pass)
    m_proxyWidget = new ProxyInputWidget(this);
    m_proxyWidget->setProxyString(currentProxy);
    mainLayout->addWidget(m_proxyWidget);

    mainLayout->addStretch();

    // Buttons
    auto *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    btnLayout->addStretch();

    auto *btnCancel = new QPushButton("Hủy");
    btnCancel->setCursor(Qt::PointingHandCursor);
    btnCancel->setStyleSheet(
        "QPushButton { background: #f1f5f9; color: #475569; font-weight: 600; "
        "border: none; border-radius: 8px; padding: 9px 18px; font-size: 13px; } "
        "QPushButton:hover { background: #e2e8f0; }");
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    auto *btnSave = new QPushButton("💾  Cập Nhật Proxy");
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setStyleSheet(
        "QPushButton { background: #2563eb; color: #ffffff; font-weight: 700; "
        "border: none; border-radius: 8px; padding: 9px 20px; font-size: 13px; } "
        "QPushButton:hover { background: #1d4ed8; }");
    connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);

    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnSave);
    mainLayout->addLayout(btnLayout);
}

QString QuickProxyDialog::getProxy() const
{
    return m_proxyWidget ? m_proxyWidget->getProxyString() : "";
}
