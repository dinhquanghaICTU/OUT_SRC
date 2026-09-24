#include "ui/ProxyInputWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QClipboard>
#include <QGuiApplication>

ProxyInputWidget::ProxyInputWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void ProxyInputWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);

    // Header with quick clear
    auto *headerLayout = new QHBoxLayout();
    auto *lblTitle = new QLabel("🌐 Cấu hình Proxy thủ công:");
    lblTitle->setStyleSheet("font-weight: 700; color: #1e293b; font-size: 13px;");
    headerLayout->addWidget(lblTitle);
    headerLayout->addStretch();

    auto *btnClear = new QPushButton("⚡ Dùng Mạng Máy (Xóa Proxy)");
    btnClear->setCursor(Qt::PointingHandCursor);
    btnClear->setStyleSheet(
        "QPushButton { color: #ea580c; font-size: 11.5px; font-weight: 700; border: none; background: transparent; padding: 2px 4px; } "
        "QPushButton:hover { text-decoration: underline; color: #c2410c; }");
    connect(btnClear, &QPushButton::clicked, [this]() {
        m_updating = true;
        m_protocolCombo->setCurrentIndex(0);
        m_hostEdit->clear();
        m_portSpin->setValue(0);
        m_userEdit->clear();
        m_passEdit->clear();
        m_quickPasteEdit->clear();
        m_updating = false;
        emit proxyChanged();
    });
    headerLayout->addWidget(btnClear);
    mainLayout->addLayout(headerLayout);

    // Row 1: Protocol + IP/Host + Port
    auto *row1 = new QHBoxLayout();
    row1->setSpacing(8);

    auto *protoBox = new QVBoxLayout();
    protoBox->setSpacing(3);
    protoBox->addWidget(new QLabel("Giao thức:"));
    m_protocolCombo = new QComboBox();
    m_protocolCombo->addItem("HTTP", "http");
    m_protocolCombo->addItem("SOCKS5", "socks5");
    m_protocolCombo->addItem("SOCKS4", "socks4");
    m_protocolCombo->setFixedWidth(95);
    connect(m_protocolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProxyInputWidget::onFieldChanged);
    protoBox->addWidget(m_protocolCombo);
    row1->addLayout(protoBox);

    auto *hostBox = new QVBoxLayout();
    hostBox->setSpacing(3);
    hostBox->addWidget(new QLabel("IP / Host máy chủ:"));
    m_hostEdit = new QLineEdit();
    m_hostEdit->setPlaceholderText("Ví dụ: 103.149.28.12");
    connect(m_hostEdit, &QLineEdit::textChanged, this, &ProxyInputWidget::onFieldChanged);
    hostBox->addWidget(m_hostEdit);
    row1->addLayout(hostBox, 3);

    auto *portBox = new QVBoxLayout();
    portBox->setSpacing(3);
    portBox->addWidget(new QLabel("Cổng (Port):"));
    m_portSpin = new QSpinBox();
    m_portSpin->setRange(0, 65535);
    m_portSpin->setSpecialValueText("Cổng");
    m_portSpin->setValue(0);
    m_portSpin->setFixedWidth(100);
    connect(m_portSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ProxyInputWidget::onFieldChanged);
    portBox->addWidget(m_portSpin);
    row1->addLayout(portBox);

    mainLayout->addLayout(row1);

    // Row 2: User + Pass
    auto *row2 = new QHBoxLayout();
    row2->setSpacing(8);

    auto *userBox = new QVBoxLayout();
    userBox->setSpacing(3);
    userBox->addWidget(new QLabel("Tài khoản (Username - nếu có):"));
    m_userEdit = new QLineEdit();
    m_userEdit->setPlaceholderText("Tên đăng nhập proxy");
    connect(m_userEdit, &QLineEdit::textChanged, this, &ProxyInputWidget::onFieldChanged);
    userBox->addWidget(m_userEdit);
    row2->addLayout(userBox, 1);

    auto *passBox = new QVBoxLayout();
    passBox->setSpacing(3);
    passBox->addWidget(new QLabel("Mật khẩu (Password - nếu có):"));
    m_passEdit = new QLineEdit();
    m_passEdit->setPlaceholderText("Mật khẩu proxy");
    m_passEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    connect(m_passEdit, &QLineEdit::textChanged, this, &ProxyInputWidget::onFieldChanged);
    passBox->addWidget(m_passEdit);
    row2->addLayout(passBox, 1);

    mainLayout->addLayout(row2);

    // Row 3: Quick paste helper
    auto *quickBox = new QHBoxLayout();
    quickBox->setSpacing(6);

    auto *lblQuick = new QLabel("⚡ Dán nhanh chuỗi:");
    lblQuick->setStyleSheet("font-size: 11.5px; color: #64748b; font-weight: 600;");
    quickBox->addWidget(lblQuick);

    m_quickPasteEdit = new QLineEdit();
    m_quickPasteEdit->setPlaceholderText("Dán IP:Port hoặc IP:Port:User:Pass vào đây để tự chia...");
    m_quickPasteEdit->setStyleSheet("font-size: 11.5px; background: #f8fafc; border: 1px dashed #cbd5e1;");
    connect(m_quickPasteEdit, &QLineEdit::textChanged, this, &ProxyInputWidget::onQuickPasteChanged);
    quickBox->addWidget(m_quickPasteEdit, 1);

    auto *btnPaste = new QPushButton("📋 Dán");
    btnPaste->setCursor(Qt::PointingHandCursor);
    btnPaste->setStyleSheet(
        "QPushButton { background: #f1f5f9; color: #334155; border: 1px solid #cbd5e1; "
        "border-radius: 6px; padding: 4px 10px; font-size: 11px; font-weight: 600; } "
        "QPushButton:hover { background: #e2e8f0; }");
    connect(btnPaste, &QPushButton::clicked, [this]() {
        QClipboard *clip = QGuiApplication::clipboard();
        if (clip) {
            m_quickPasteEdit->setText(clip->text().trimmed());
        }
    });
    quickBox->addWidget(btnPaste);

    mainLayout->addLayout(quickBox);
}

void ProxyInputWidget::onQuickPasteChanged(const QString &text)
{
    if (m_updating) return;
    if (text.trimmed().isEmpty()) return;

    ProxyConfig cfg = ProxyConfig::fromString(text.trimmed());
    if (!cfg.isEmpty() || !cfg.host.isEmpty()) {
        m_updating = true;

        int pIdx = m_protocolCombo->findData(cfg.protocol.toLower());
        if (pIdx >= 0) m_protocolCombo->setCurrentIndex(pIdx);
        else m_protocolCombo->setCurrentIndex(0);

        m_hostEdit->setText(cfg.host);
        m_portSpin->setValue(cfg.port);
        m_userEdit->setText(cfg.username);
        m_passEdit->setText(cfg.password);

        m_updating = false;
        emit proxyChanged();
    }
}

void ProxyInputWidget::onFieldChanged()
{
    if (m_updating) return;
    emit proxyChanged();
}

void ProxyInputWidget::setProxy(const ProxyConfig &cfg)
{
    m_updating = true;
    int pIdx = m_protocolCombo->findData(cfg.protocol.toLower());
    if (pIdx >= 0) m_protocolCombo->setCurrentIndex(pIdx);
    else m_protocolCombo->setCurrentIndex(0);

    m_hostEdit->setText(cfg.host);
    m_portSpin->setValue(cfg.port);
    m_userEdit->setText(cfg.username);
    m_passEdit->setText(cfg.password);
    m_quickPasteEdit->clear();
    m_updating = false;
}

void ProxyInputWidget::setProxyString(const QString &rawProxy)
{
    ProxyConfig cfg = ProxyConfig::fromString(rawProxy);
    setProxy(cfg);
}

ProxyConfig ProxyInputWidget::getProxy() const
{
    ProxyConfig cfg;
    cfg.protocol = m_protocolCombo ? m_protocolCombo->currentData().toString() : "http";
    cfg.host = m_hostEdit ? m_hostEdit->text().trimmed() : "";
    cfg.port = m_portSpin ? m_portSpin->value() : 0;
    cfg.username = m_userEdit ? m_userEdit->text().trimmed() : "";
    cfg.password = m_passEdit ? m_passEdit->text().trimmed() : "";
    return cfg;
}

QString ProxyInputWidget::getProxyString() const
{
    return getProxy().toFormattedString();
}
