#include "ui/CreateProfileDialog.h"
#include "ui/CustomMessageBox.h"
#include "ui/EditProfileDialog.h"
#include "ui/ProxyInputWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QComboBox>

CreateProfileDialog::CreateProfileDialog(const QString &suggestedName, int suggestedPort,
                                         const QStringList &existingNames, QWidget *parent)
    : QDialog(parent), m_existingNames(existingNames)
{
    setupUi(suggestedName, suggestedPort);
}

void CreateProfileDialog::setupUi(const QString &suggestedName, int suggestedPort)
{
    setWindowTitle("Tạo Chrome Profile Mới");
    setFixedSize(540, 680);
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
    mainLayout->setSpacing(13);

    // Title Row
    auto *titleRow = new QHBoxLayout();
    auto *iconLbl = new QLabel("👥");
    iconLbl->setStyleSheet("font-size: 22px; background: #ffedd5; padding: 6px; border-radius: 10px;");
    auto *titleText = new QLabel("Tạo Chrome Profile Mới");
    titleText->setStyleSheet("font-size: 17px; font-weight: 800; color: #0f172a;");
    titleRow->addWidget(iconLbl);
    titleRow->addWidget(titleText);
    titleRow->addStretch();
    mainLayout->addLayout(titleRow);

    auto *divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("color: #f1f5f9;");
    mainLayout->addWidget(divider);

    // 1. Profile Name Input
    auto *nameBox = new QVBoxLayout();
    nameBox->setSpacing(4);
    auto *lblName = new QLabel("Tên Profile (Không được trùng lặp):");
    m_nameEdit = new QLineEdit();
    m_nameEdit->setText(suggestedName);
    m_nameEdit->setPlaceholderText("Ví dụ: Profile 01 (Nuôi Facebook)");
    nameBox->addWidget(lblName);
    nameBox->addWidget(m_nameEdit);
    mainLayout->addLayout(nameBox);

    // 2. Device Type (User-Agent)
    auto *deviceBox = new QVBoxLayout();
    deviceBox->setSpacing(4);
    deviceBox->addWidget(new QLabel("Loại Máy & Hệ Điều Hành:"));
    m_deviceCombo = new QComboBox();
    m_deviceCombo->addItem("💻 Windows 11 Desktop (Chrome PC)", "windows");
    m_deviceCombo->addItem("🍏 macOS Sonoma (MacBook / iMac)", "macos");
    m_deviceCombo->addItem("🐧 Linux Desktop (Ubuntu)", "linux");
    m_deviceCombo->addItem("📱 iPhone 15 Pro Max (iOS Mobile)", "iphone");
    m_deviceCombo->addItem("🤖 Samsung Galaxy S24 Ultra (Android)", "android");
    m_deviceCombo->addItem("📱 iPad Pro (Tablet iOS)", "ipad");
    m_deviceCombo->addItem("⚙️ Tùy Chỉnh (Custom User-Agent)...", "custom");
    deviceBox->addWidget(m_deviceCombo);

    m_uaEdit = new QLineEdit();
    m_uaEdit->setText(EditProfileDialog::getUserAgentForDevice("windows"));
    m_uaEdit->setStyleSheet("font-size: 11.5px; color: #64748b; background: #f1f5f9;");
    m_uaEdit->setReadOnly(true);
    deviceBox->addWidget(m_uaEdit);
    mainLayout->addLayout(deviceBox);

    connect(m_deviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        QString dev = m_deviceCombo->itemData(idx).toString();
        if (dev != "custom") {
            m_uaEdit->setText(EditProfileDialog::getUserAgentForDevice(dev));
            m_uaEdit->setStyleSheet("font-size: 11.5px; color: #64748b; background: #f1f5f9;");
            m_uaEdit->setReadOnly(true);
        } else {
            m_uaEdit->setStyleSheet("font-size: 11.5px; color: #0f172a; background: #ffffff; border: 1.5px solid #f97316;");
            m_uaEdit->setReadOnly(false);
            m_uaEdit->setFocus();
        }
    });

    // 3. Proxy Input (Split IP, Port, User, Pass)
    m_proxyWidget = new ProxyInputWidget(this);
    mainLayout->addWidget(m_proxyWidget);

    // Remote Port Input
    auto *portBox = new QVBoxLayout();
    portBox->setSpacing(5);
    auto *lblPort = new QLabel("Cổng điều khiển Remote Debugging Port (CDP):");
    m_portSpin = new QSpinBox();
    m_portSpin->setRange(1024, 65535);
    m_portSpin->setValue(suggestedPort);
    portBox->addWidget(lblPort);
    portBox->addWidget(m_portSpin);
    mainLayout->addLayout(portBox);

    // Window Size / Aspect Ratio
    auto *sizeBox = new QVBoxLayout();
    sizeBox->setSpacing(5);
    auto *lblSize = new QLabel("Kích thước & Tỉ lệ mở cửa sổ Chrome:");
    m_sizeCombo = new QComboBox();
    m_sizeCombo->addItem("⚙ Theo cài đặt chung trên thanh công cụ", QPoint(0, 0));
    m_sizeCombo->addItem("📱 Mobile MMO Dọc (380 x 680)", QPoint(380, 680));
    m_sizeCombo->addItem("💻 Gọn Nuôi Nick (450 x 700)", QPoint(450, 700));
    m_sizeCombo->addItem("🖥️ Vừa phải (600 x 800)", QPoint(600, 800));
    m_sizeCombo->addItem("📺 HD Chuẩn (1280 x 720)", QPoint(1280, 720));
    m_sizeCombo->addItem("✏️ Tự nhập kích thước (Custom)...", QPoint(-1, -1));
    sizeBox->addWidget(lblSize);
    sizeBox->addWidget(m_sizeCombo);

    // Custom Width & Height Row
    m_customSizeWidget = new QWidget();
    auto *customLayout = new QHBoxLayout(m_customSizeWidget);
    customLayout->setContentsMargins(0, 0, 0, 0);
    customLayout->setSpacing(8);

    auto *lblW = new QLabel("Rộng (px):");
    m_widthSpin = new QSpinBox();
    m_widthSpin->setRange(200, 3840);
    m_widthSpin->setValue(450);

    auto *lblH = new QLabel("Cao (px):");
    m_heightSpin = new QSpinBox();
    m_heightSpin->setRange(200, 2160);
    m_heightSpin->setValue(700);

    customLayout->addWidget(lblW);
    customLayout->addWidget(m_widthSpin);
    customLayout->addWidget(lblH);
    customLayout->addWidget(m_heightSpin);

    m_customSizeWidget->setVisible(false);
    sizeBox->addWidget(m_customSizeWidget);

    connect(m_sizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_customSizeWidget->setVisible(idx == 5);
    });

    mainLayout->addLayout(sizeBox);

    mainLayout->addStretch();

    // Action Buttons
    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(12);

    auto *btnCancel = new QPushButton("Hủy bỏ");
    btnCancel->setCursor(Qt::PointingHandCursor);
    btnCancel->setFixedHeight(40);
    btnCancel->setStyleSheet(
        "QPushButton { background: #f1f5f9; color: #64748b; font-weight: 600; "
        "border: none; border-radius: 8px; font-size: 13px; } "
        "QPushButton:hover { background: #e2e8f0; color: #0f172a; }");
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    auto *btnOk = new QPushButton("✨ Tạo Profile");
    btnOk->setCursor(Qt::PointingHandCursor);
    btnOk->setFixedHeight(40);
    btnOk->setStyleSheet(
        "QPushButton { background: #f97316; color: #ffffff; font-weight: 700; "
        "border: none; border-radius: 8px; font-size: 13px; } "
        "QPushButton:hover { background: #ea580c; }");
    connect(btnOk, &QPushButton::clicked, [this]() {
        QString name = m_nameEdit->text().trimmed();
        if (name.isEmpty()) {
            CustomMessageBox::warning(this, "Thông báo", "Vui lòng nhập tên Profile!");
            m_nameEdit->setFocus();
            return;
        }

        // Kiểm tra trùng tên profile (không phân biệt hoa/thường)
        for (const QString &exist : m_existingNames) {
            if (exist.trimmed().compare(name, Qt::CaseInsensitive) == 0) {
                CustomMessageBox::warning(
                    this, "Trùng tên Profile",
                    QString("Tên Profile '%1' đã tồn tại!\nVui lòng đặt một tên khác.").arg(name));
                m_nameEdit->selectAll();
                m_nameEdit->setFocus();
                return;
            }
        }

        accept();
    });

    btnRow->addWidget(btnCancel, 1);
    btnRow->addWidget(btnOk, 1);
    mainLayout->addLayout(btnRow);
}

QString CreateProfileDialog::getProfileName() const
{
    return m_nameEdit ? m_nameEdit->text().trimmed() : "";
}

QString CreateProfileDialog::getDeviceType() const
{
    return m_deviceCombo ? m_deviceCombo->currentData().toString() : "windows";
}

QString CreateProfileDialog::getUserAgent() const
{
    return m_uaEdit ? m_uaEdit->text().trimmed() : "";
}

QString CreateProfileDialog::getProxy() const
{
    return m_proxyWidget ? m_proxyWidget->getProxyString() : "";
}

int CreateProfileDialog::getPort() const
{
    return m_portSpin ? m_portSpin->value() : 9222;
}

int CreateProfileDialog::getWindowWidth() const
{
    if (!m_sizeCombo) return 0;
    if (m_sizeCombo->currentIndex() == 5) {
        return m_widthSpin ? m_widthSpin->value() : 450;
    }
    QPoint pt = m_sizeCombo->currentData().toPoint();
    return pt.x();
}

int CreateProfileDialog::getWindowHeight() const
{
    if (!m_sizeCombo) return 0;
    if (m_sizeCombo->currentIndex() == 5) {
        return m_heightSpin ? m_heightSpin->value() : 700;
    }
    QPoint pt = m_sizeCombo->currentData().toPoint();
    return pt.y();
}
