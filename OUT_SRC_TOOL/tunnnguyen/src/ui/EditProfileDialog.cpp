#include "ui/EditProfileDialog.h"
#include "ui/CustomMessageBox.h"
#include "ui/ProxyInputWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QClipboard>
#include <QGuiApplication>

QString EditProfileDialog::getUserAgentForDevice(const QString &deviceType, const QString &customUa)
{
    if (!customUa.trimmed().isEmpty()) return customUa.trimmed();
    QString d = deviceType.toLower().trimmed();
    if (d == "macos" || d == "mac") {
        return "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36";
    } else if (d == "linux") {
        return "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36";
    } else if (d == "iphone" || d == "ios") {
        return "Mozilla/5.0 (iPhone; CPU iPhone OS 17_5_1 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) CriOS/128.0.6613.98 Mobile/15E148 Safari/604.1";
    } else if (d == "android") {
        return "Mozilla/5.0 (Linux; Android 14; SM-S928B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.6613.88 Mobile Safari/537.36";
    } else if (d == "ipad") {
        return "Mozilla/5.0 (iPad; CPU OS 17_5 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) CriOS/128.0.6613.98 Mobile/15E148 Safari/604.1";
    }
    // Default Windows 11
    return "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36";
}

QString EditProfileDialog::getDeviceIcon(const QString &deviceType)
{
    QString d = deviceType.toLower().trimmed();
    if (d == "macos" || d == "mac") return "🍏";
    if (d == "linux") return "🐧";
    if (d == "iphone" || d == "ios") return "📱";
    if (d == "android") return "🤖";
    if (d == "ipad") return "📱";
    return "💻";
}

QString EditProfileDialog::getDeviceDisplayName(const QString &deviceType)
{
    QString d = deviceType.toLower().trimmed();
    if (d == "macos" || d == "mac") return "macOS";
    if (d == "linux") return "Linux";
    if (d == "iphone" || d == "ios") return "iPhone iOS";
    if (d == "android") return "Android";
    if (d == "ipad") return "iPad";
    return "Windows";
}

EditProfileDialog::EditProfileDialog(const QString &currentName,
                                     const QString &currentDevice,
                                     const QString &currentUserAgent,
                                     const QString &currentProxy,
                                     int currentPort,
                                     int currentWidth,
                                     int currentHeight,
                                     const QStringList &otherNames,
                                     QWidget *parent)
    : QDialog(parent),
      m_originalName(currentName),
      m_deviceType(currentDevice.isEmpty() ? "windows" : currentDevice),
      m_userAgent(currentUserAgent),
      m_proxy(currentProxy),
      m_port(currentPort),
      m_width(currentWidth),
      m_height(currentHeight),
      m_otherNames(otherNames)
{
    setupUi();
}

void EditProfileDialog::setupUi()
{
    setWindowTitle("Chỉnh Sửa Cấu Hình Profile");
    setFixedSize(540, 640);
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
        "QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border: 1.5px solid #f97316; background: #ffffff; }"
        "QLabel { color: #334155; font-size: 13px; font-weight: 600; }"
    );

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(14);

    // Title Row
    auto *titleRow = new QHBoxLayout();
    auto *iconLbl = new QLabel("⚙️");
    iconLbl->setStyleSheet("font-size: 20px; background: #ffedd5; padding: 6px; border-radius: 10px;");
    auto *titleText = new QLabel(QString("Chỉnh Sửa Profile: %1").arg(m_originalName));
    titleText->setStyleSheet("font-size: 16px; font-weight: 800; color: #0f172a;");
    titleRow->addWidget(iconLbl);
    titleRow->addWidget(titleText);
    titleRow->addStretch();
    mainLayout->addLayout(titleRow);

    auto *divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("color: #f1f5f9;");
    mainLayout->addWidget(divider);

    // 1. Tên Profile
    auto *nameBox = new QVBoxLayout();
    nameBox->setSpacing(4);
    nameBox->addWidget(new QLabel("Tên Profile:"));
    m_nameEdit = new QLineEdit(m_originalName);
    nameBox->addWidget(m_nameEdit);
    mainLayout->addLayout(nameBox);

    // 2. Loại Máy / Thiết Bị (Device Fingerprint)
    auto *deviceBox = new QVBoxLayout();
    deviceBox->setSpacing(4);
    deviceBox->addWidget(new QLabel("Loại Máy & Hệ Điều Hành (Device User-Agent):"));
    m_deviceCombo = new QComboBox();
    m_deviceCombo->addItem("💻 Windows 11 Desktop (Chrome PC)", "windows");
    m_deviceCombo->addItem("🍏 macOS Sonoma (MacBook / iMac)", "macos");
    m_deviceCombo->addItem("🐧 Linux Desktop (Ubuntu / Debian)", "linux");
    m_deviceCombo->addItem("📱 iPhone 15 Pro Max (iOS Safari/Chrome)", "iphone");
    m_deviceCombo->addItem("🤖 Samsung Galaxy S24 Ultra (Android Mobile)", "android");
    m_deviceCombo->addItem("📱 iPad Pro (Tablet iOS)", "ipad");
    m_deviceCombo->addItem("⚙️ Tùy Chỉnh (Custom User-Agent)...", "custom");

    int devIdx = m_deviceCombo->findData(m_deviceType);
    if (devIdx >= 0) m_deviceCombo->setCurrentIndex(devIdx);
    else if (!m_userAgent.isEmpty()) m_deviceCombo->setCurrentIndex(6); // custom
    deviceBox->addWidget(m_deviceCombo);

    // User-Agent Display/Edit
    m_uaEdit = new QLineEdit();
    m_uaEdit->setText(m_userAgent.isEmpty() ? getUserAgentForDevice(m_deviceType) : m_userAgent);
    m_uaEdit->setStyleSheet("font-size: 11.5px; color: #64748b; background: #f1f5f9;");
    deviceBox->addWidget(m_uaEdit);
    mainLayout->addLayout(deviceBox);

    connect(m_deviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditProfileDialog::onDeviceChanged);

    // 3. Proxy Input with separate fields (Giao thức, IP, Cổng, User, Pass)
    m_proxyWidget = new ProxyInputWidget(this);
    m_proxyWidget->setProxyString(m_proxy);
    mainLayout->addWidget(m_proxyWidget);

    // 4. Remote Port & Window Size Row
    auto *row2 = new QHBoxLayout();
    row2->setSpacing(12);

    auto *portBox = new QVBoxLayout();
    portBox->setSpacing(4);
    portBox->addWidget(new QLabel("Remote Port (CDP):"));
    m_portSpin = new QSpinBox();
    m_portSpin->setRange(1024, 65535);
    m_portSpin->setValue(m_port);
    portBox->addWidget(m_portSpin);
    row2->addLayout(portBox, 1);

    auto *sizeBox = new QVBoxLayout();
    sizeBox->setSpacing(4);
    sizeBox->addWidget(new QLabel("Tỉ Lệ Mở Cửa Sổ:"));
    m_sizeCombo = new QComboBox();
    m_sizeCombo->addItem("⚙ Theo cài đặt chung", QPoint(0, 0));
    m_sizeCombo->addItem("📱 Mobile MMO (380 x 600)", QPoint(380, 600));
    m_sizeCombo->addItem("💻 Gọn Nuôi Nick (480 x 520)", QPoint(480, 520));
    m_sizeCombo->addItem("🖥️ Vừa phải (600 x 700)", QPoint(600, 700));
    m_sizeCombo->addItem("📺 HD Chuẩn (1280 x 720)", QPoint(1280, 720));
    m_sizeCombo->addItem("✏️ Tự nhập kích thước...", QPoint(-1, -1));

    // Match existing size
    if (m_width == 0 && m_height == 0) m_sizeCombo->setCurrentIndex(0);
    else if (m_width == 380 && (m_height == 600 || m_height == 680)) m_sizeCombo->setCurrentIndex(1);
    else if (m_width == 480 || (m_width == 450 && m_height == 700)) m_sizeCombo->setCurrentIndex(2);
    else if (m_width == 600 && (m_height == 700 || m_height == 800)) m_sizeCombo->setCurrentIndex(3);
    else if (m_width == 1280 && m_height == 720) m_sizeCombo->setCurrentIndex(4);
    else m_sizeCombo->setCurrentIndex(5);

    sizeBox->addWidget(m_sizeCombo);
    row2->addLayout(sizeBox, 2);
    mainLayout->addLayout(row2);

    // Custom Size Row
    m_customSizeWidget = new QWidget();
    auto *customLayout = new QHBoxLayout(m_customSizeWidget);
    customLayout->setContentsMargins(0, 0, 0, 0);
    customLayout->setSpacing(8);

    m_widthSpin = new QSpinBox();
    m_widthSpin->setRange(200, 3840);
    m_widthSpin->setValue(m_width > 0 ? m_width : 480);
    m_heightSpin = new QSpinBox();
    m_heightSpin->setRange(200, 2160);
    m_heightSpin->setValue(m_height > 0 ? m_height : 520);

    customLayout->addWidget(new QLabel("Rộng:"));
    customLayout->addWidget(m_widthSpin);
    customLayout->addWidget(new QLabel("Cao:"));
    customLayout->addWidget(m_heightSpin);

    m_customSizeWidget->setVisible(m_sizeCombo->currentIndex() == 5);
    mainLayout->addWidget(m_customSizeWidget);

    connect(m_sizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_customSizeWidget->setVisible(idx == 5);
    });

    mainLayout->addStretch();

    // Bottom Action Buttons
    auto *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    btnLayout->addStretch();

    auto *btnCancel = new QPushButton("Hủy Bỏ");
    btnCancel->setCursor(Qt::PointingHandCursor);
    btnCancel->setStyleSheet(
        "QPushButton { background: #f1f5f9; color: #475569; font-weight: 600; "
        "border: none; border-radius: 8px; padding: 10px 20px; font-size: 13px; } "
        "QPushButton:hover { background: #e2e8f0; }");
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    auto *btnSave = new QPushButton("💾  Lưu Thay Đổi");
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setStyleSheet(
        "QPushButton { background: #f97316; color: #ffffff; font-weight: 700; "
        "border: none; border-radius: 8px; padding: 10px 22px; font-size: 13px; } "
        "QPushButton:hover { background: #ea580c; }");

    connect(btnSave, &QPushButton::clicked, [this]() {
        QString name = m_nameEdit->text().trimmed();
        if (name.isEmpty()) {
            CustomMessageBox::warning(this, "Thiếu thông tin", "Vui lòng nhập tên cho Profile!");
            m_nameEdit->setFocus();
            return;
        }

        // Kiểm tra trùng tên với profile khác
        if (name.compare(m_originalName, Qt::CaseInsensitive) != 0) {
            for (const QString &existName : m_otherNames) {
                if (existName.compare(name, Qt::CaseInsensitive) == 0) {
                    CustomMessageBox::critical(this, "Trùng tên Profile",
                        QString("Tên Profile '%1' đã tồn tại! Vui lòng chọn tên khác.").arg(name));
                    m_nameEdit->setFocus();
                    return;
                }
            }
        }

        accept();
    });

    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnSave);
    mainLayout->addLayout(btnLayout);
}

void EditProfileDialog::onDeviceChanged(int index)
{
    QString dev = m_deviceCombo->itemData(index).toString();
    if (dev != "custom") {
        m_uaEdit->setText(getUserAgentForDevice(dev));
        m_uaEdit->setStyleSheet("font-size: 11.5px; color: #64748b; background: #f1f5f9;");
        m_uaEdit->setReadOnly(true);
    } else {
        m_uaEdit->setStyleSheet("font-size: 11.5px; color: #0f172a; background: #ffffff; border: 1.5px solid #f97316;");
        m_uaEdit->setReadOnly(false);
        m_uaEdit->setFocus();
    }
}

QString EditProfileDialog::getProfileName() const
{
    return m_nameEdit ? m_nameEdit->text().trimmed() : m_originalName;
}

QString EditProfileDialog::getDeviceType() const
{
    return m_deviceCombo ? m_deviceCombo->currentData().toString() : "windows";
}

QString EditProfileDialog::getUserAgent() const
{
    return m_uaEdit ? m_uaEdit->text().trimmed() : "";
}

QString EditProfileDialog::getProxy() const
{
    return m_proxyWidget ? m_proxyWidget->getProxyString() : "";
}

int EditProfileDialog::getPort() const
{
    return m_portSpin ? m_portSpin->value() : 9222;
}

int EditProfileDialog::getWindowWidth() const
{
    if (!m_sizeCombo) return 0;
    QPoint pt = m_sizeCombo->currentData().toPoint();
    if (pt.x() == -1 && m_widthSpin) {
        return m_widthSpin->value();
    }
    return pt.x();
}

int EditProfileDialog::getWindowHeight() const
{
    if (!m_sizeCombo) return 0;
    QPoint pt = m_sizeCombo->currentData().toPoint();
    if (pt.y() == -1 && m_heightSpin) {
        return m_heightSpin->value();
    }
    return pt.y();
}
