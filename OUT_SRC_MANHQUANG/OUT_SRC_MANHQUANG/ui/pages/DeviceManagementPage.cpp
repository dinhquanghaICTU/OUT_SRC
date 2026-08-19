#include "DeviceManagementPage.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QDateTime>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTimer>

DeviceManagementPage::DeviceManagementPage(QWidget *parent)
    : QWidget(parent), m_refreshTimer(new QTimer(this))
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 10, 12, 10);
    mainLayout->setSpacing(8);

    // Top Header / Tab switcher
    auto *topBar = new QHBoxLayout;
    auto *pageTitle = new QLabel(tr("📡 QUẢN LÝ THIẾT BỊ & THÔNG SỐ CỬA"), this);
    pageTitle->setStyleSheet(QStringLiteral("color: #38bdf8; font-size: 14px; font-weight: 800;"));
    topBar->addWidget(pageTitle);

    topBar->addStretch();

    m_cardsTabBtn = new QPushButton(tr("🎛️ Bảng Điều Khiển & Cài Đặt"), this);
    m_cardsTabBtn->setCheckable(true);
    m_cardsTabBtn->setChecked(true);
    m_cardsTabBtn->setCursor(Qt::PointingHandCursor);
    m_cardsTabBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #1e293b; color: #94a3b8; border: none; border-radius: 6px; padding: 6px 14px; font-weight: 700; font-size: 11px; } "
        "QPushButton:checked { background: #0284c7; color: white; }"));

    m_logTabBtn = new QPushButton(tr("📋 Nhật Ký Telemetry Trực Tiếp"), this);
    m_logTabBtn->setCheckable(true);
    m_logTabBtn->setCursor(Qt::PointingHandCursor);
    m_logTabBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #1e293b; color: #94a3b8; border: none; border-radius: 6px; padding: 6px 14px; font-weight: 700; font-size: 11px; } "
        "QPushButton:checked { background: #0284c7; color: white; }"));

    topBar->addWidget(m_cardsTabBtn);
    topBar->addWidget(m_logTabBtn);

    auto *refreshBtn = new QPushButton(tr("🔄 Làm mới"), this);
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #334155; color: #f1f5f9; border: none; border-radius: 6px; padding: 6px 12px; font-weight: 600; font-size: 11px; }"));
    connect(refreshBtn, &QPushButton::clicked, this, &DeviceManagementPage::refreshRequested);
    topBar->addWidget(refreshBtn);

    mainLayout->addLayout(topBar);

    // View Stack (Page 0: Cards & Drawer, Page 1: Log Table)
    m_viewStack = new QStackedWidget(this);
    mainLayout->addWidget(m_viewStack, 1);

    connect(m_cardsTabBtn, &QPushButton::clicked, this, [this] {
        m_cardsTabBtn->setChecked(true);
        m_logTabBtn->setChecked(false);
        m_viewStack->setCurrentIndex(0);
    });
    connect(m_logTabBtn, &QPushButton::clicked, this, [this] {
        m_logTabBtn->setChecked(true);
        m_cardsTabBtn->setChecked(false);
        m_viewStack->setCurrentIndex(1);
    });

    // PAGE 0: Cards and Drawer
    auto *page0 = new QWidget(this);
    auto *page0Layout = new QHBoxLayout(page0);
    page0Layout->setContentsMargins(0, 0, 0, 0);
    page0Layout->setSpacing(10);

    auto *leftScroll = new QScrollArea(page0);
    leftScroll->setWidgetResizable(true);
    leftScroll->setFrameShape(QFrame::NoFrame);
    leftScroll->setStyleSheet(QStringLiteral("background: transparent; border: none;"));

    auto *leftContainer = new QWidget(leftScroll);
    auto *leftLayout = new QVBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(12);

    auto *ownedTitle = new QLabel(tr("🚪 Thiết Bị Đã Gán Vào Tài Khoản"), leftContainer);
    ownedTitle->setStyleSheet(QStringLiteral("color: #10b981; font-size: 12px; font-weight: 800;"));
    leftLayout->addWidget(ownedTitle);

    m_ownedGrid = new QGridLayout;
    m_ownedGrid->setSpacing(10);
    leftLayout->addLayout(m_ownedGrid);

    m_ownedEmpty = new QLabel(tr("Chưa có thiết bị nào được gán."), leftContainer);
    m_ownedEmpty->setStyleSheet(QStringLiteral("color: #64748b; font-style: italic; font-size: 11px;"));
    leftLayout->addWidget(m_ownedEmpty);

    auto *availTitle = new QLabel(tr("📡 Thiết Bị Mới Phát Hiện (Chưa gán)"), leftContainer);
    availTitle->setStyleSheet(QStringLiteral("color: #38bdf8; font-size: 12px; font-weight: 800;"));
    leftLayout->addWidget(availTitle);

    m_availableGrid = new QGridLayout;
    m_availableGrid->setSpacing(10);
    leftLayout->addLayout(m_availableGrid);

    m_availableEmpty = new QLabel(tr("Không có thiết bị trực tuyến chưa gán."), leftContainer);
    m_availableEmpty->setStyleSheet(QStringLiteral("color: #64748b; font-style: italic; font-size: 11px;"));
    leftLayout->addWidget(m_availableEmpty);

    leftLayout->addStretch();
    leftScroll->setWidget(leftContainer);
    page0Layout->addWidget(leftScroll, 1);

    // Right Side: Device Config & Tuning Drawer
    m_drawer = new QFrame(page0);
    m_drawer->setMinimumWidth(280);
    m_drawer->setMaximumWidth(320);
    m_drawer->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #0f172a; border: 1.5px solid #1e293b; border-radius: 12px; }"));

    auto *drawerLayout = new QVBoxLayout(m_drawer);
    drawerLayout->setContentsMargins(14, 14, 14, 14);
    drawerLayout->setSpacing(10);

    m_drawerTitle = new QLabel(tr("⚙️ CẤU HÌNH ĐỘNG CƠ & CẢM BIẾN"), m_drawer);
    m_drawerTitle->setStyleSheet(QStringLiteral("color: #f59e0b; font-size: 12px; font-weight: 800;"));
    drawerLayout->addWidget(m_drawerTitle);

    m_drawerId = new QLabel(tr("Chọn thiết bị bên trái để chỉnh sửa"), m_drawer);
    m_drawerId->setStyleSheet(QStringLiteral("color: #94a3b8; font-size: 11px;"));
    drawerLayout->addWidget(m_drawerId);

    auto *formLayout = new QFormLayout;
    formLayout->setSpacing(8);

    m_inputMotorSpeed = new QSpinBox(m_drawer);
    m_inputMotorSpeed->setRange(100, 3200);
    m_inputMotorSpeed->setValue(800);
    m_inputMotorSpeed->setSuffix(tr(" steps/s"));
    m_inputMotorSpeed->setStyleSheet(QStringLiteral("background-color: #1e293b; color: white; padding: 4px; border-radius: 4px;"));
    formLayout->addRow(new QLabel(tr("Tốc độ động cơ:"), m_drawer), m_inputMotorSpeed);

    m_inputMaxSteps = new QSpinBox(m_drawer);
    m_inputMaxSteps->setRange(500, 10000);
    m_inputMaxSteps->setValue(3200);
    m_inputMaxSteps->setSuffix(tr(" steps"));
    m_inputMaxSteps->setStyleSheet(QStringLiteral("background-color: #1e293b; color: white; padding: 4px; border-radius: 4px;"));
    formLayout->addRow(new QLabel(tr("Hành trình mở (100%):"), m_drawer), m_inputMaxSteps);

    m_inputAutoCloseDelay = new QSpinBox(m_drawer);
    m_inputAutoCloseDelay->setRange(1, 60);
    m_inputAutoCloseDelay->setValue(5);
    m_inputAutoCloseDelay->setSuffix(tr(" s"));
    m_inputAutoCloseDelay->setStyleSheet(QStringLiteral("background-color: #1e293b; color: white; padding: 4px; border-radius: 4px;"));
    formLayout->addRow(new QLabel(tr("Trễ đóng tự động:"), m_drawer), m_inputAutoCloseDelay);

    m_inputMicrostepping = new QSpinBox(m_drawer);
    m_inputMicrostepping->setRange(1, 16);
    m_inputMicrostepping->setValue(8);
    m_inputMicrostepping->setPrefix(tr("1/"));
    m_inputMicrostepping->setStyleSheet(QStringLiteral("background-color: #1e293b; color: white; padding: 4px; border-radius: 4px;"));
    formLayout->addRow(new QLabel(tr("Chế độ vi bước:"), m_drawer), m_inputMicrostepping);

    m_chkAutoReverse = new QCheckBox(tr("Tự đảo chiều khi kẹt IR"), m_drawer);
    m_chkAutoReverse->setChecked(true);
    m_chkAutoReverse->setStyleSheet(QStringLiteral("color: #e2e8f0; font-size: 11px;"));
    formLayout->addRow(QString(), m_chkAutoReverse);

    drawerLayout->addLayout(formLayout);

    m_saveConfigBtn = new QPushButton(tr("💾 Lưu & Đồng bộ MQTT"), m_drawer);
    m_saveConfigBtn->setCursor(Qt::PointingHandCursor);
    m_saveConfigBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #10b981; color: white; border: none; border-radius: 6px; padding: 8px; font-weight: 700; font-size: 11px; } "
        "QPushButton:hover { background: #059669; }"));
    connect(m_saveConfigBtn, &QPushButton::clicked, this, &DeviceManagementPage::saveThresholds);
    drawerLayout->addWidget(m_saveConfigBtn);

    m_releaseDevBtn = new QPushButton(tr("🗑️ Gỡ thiết bị khỏi tài khoản"), m_drawer);
    m_releaseDevBtn->setCursor(Qt::PointingHandCursor);
    m_releaseDevBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #ef4444; color: white; border: none; border-radius: 6px; padding: 8px; font-weight: 700; font-size: 11px; } "
        "QPushButton:hover { background: #dc2626; }"));
    connect(m_releaseDevBtn, &QPushButton::clicked, this, [this] {
        const QString did = m_selectedDevice.value(QStringLiteral("device_id")).toString();
        if (!did.isEmpty()) {
            if (QMessageBox::question(this, tr("Xác nhận"), tr("Bạn có chắc muốn xóa thiết bị %1?").arg(did)) == QMessageBox::Yes) {
                emit releaseDeviceRequested(did);
            }
        }
    });
    drawerLayout->addWidget(m_releaseDevBtn);

    drawerLayout->addStretch();
    page0Layout->addWidget(m_drawer);

    m_viewStack->addWidget(page0);

    // PAGE 1: Real-time Device Log Table
    auto *page1 = new QWidget(this);
    auto *page1Layout = new QVBoxLayout(page1);
    page1Layout->setContentsMargins(0, 0, 0, 0);

    m_deviceLogTable = new QTableWidget(page1);
    m_deviceLogTable->setColumnCount(6);
    m_deviceLogTable->setHorizontalHeaderLabels({
        tr("Thời gian"), tr("Device ID"), tr("Trạng thái cửa"), tr("Cảm biến SR602"), tr("Cảm biến IR"), tr("Động cơ bước")
    });
    m_deviceLogTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_deviceLogTable->verticalHeader()->hide();
    m_deviceLogTable->setStyleSheet(QStringLiteral(
        "QTableWidget { background-color: #0f172a; border: 1px solid #1e293b; color: #f1f5f9; border-radius: 8px; gridline-color: #1e293b; } "
        "QHeaderView::section { background-color: #1e293b; color: #38bdf8; font-weight: 700; padding: 6px; border: none; }"));
    page1Layout->addWidget(m_deviceLogTable);

    m_viewStack->addWidget(page1);

    m_refreshTimer->setInterval(3000);
    connect(m_refreshTimer, &QTimer::timeout, this, &DeviceManagementPage::refreshRequested);
}

void DeviceManagementPage::setCurrentUser(const QString &username, bool isAdmin)
{
    m_currentUsername = username;
    m_isAdmin = isAdmin;
}

void DeviceManagementPage::setOwnedDevices(const QJsonArray &devices)
{
    m_ownedDevices = devices;
    rebuildOwnedGrid();
    rebuildLogTable();
}

void DeviceManagementPage::setAvailableDevices(const QJsonArray &devices)
{
    m_availableDevices = devices;
    rebuildAvailableGrid();
}

void DeviceManagementPage::startRealtime()
{
    m_refreshTimer->start();
}

void DeviceManagementPage::stopRealtime()
{
    m_refreshTimer->stop();
}

void DeviceManagementPage::configSaved(const QString &deviceId, bool mqttPublished)
{
    Q_UNUSED(deviceId);
    Q_UNUSED(mqttPublished);
}

void DeviceManagementPage::clearGrid(QGridLayout *layout)
{
    if (!layout) return;
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
}

QWidget *DeviceManagementPage::createOwnedCard(const QJsonObject &device)
{
    auto *card = new QFrame;
    card->setObjectName(QStringLiteral("ownedCard"));
    card->setStyleSheet(QStringLiteral(
        "QFrame#ownedCard { background-color: #0f172a; border: 1.5px solid #1e293b; border-radius: 10px; padding: 10px; } "
        "QFrame#ownedCard:hover { border-color: #38bdf8; }"));

    auto *layout = new QVBoxLayout(card);
    layout->setSpacing(6);

    const QString id = device.value(QStringLiteral("device_id")).toString();
    const QString name = device.value(QStringLiteral("name")).toString(id);
    const bool online = device.value(QStringLiteral("online")).toBool(true);

    auto *headerRow = new QHBoxLayout;
    auto *icon = new QLabel(QStringLiteral("🚪"), card);
    icon->setStyleSheet(QStringLiteral("font-size: 18px;"));
    headerRow->addWidget(icon);

    auto *nameLabel = new QLabel(name, card);
    nameLabel->setStyleSheet(QStringLiteral("color: #f1f5f9; font-size: 13px; font-weight: 700;"));
    headerRow->addWidget(nameLabel);

    headerRow->addStretch();

    auto *statusBadge = new QLabel(online ? tr("🟢 Trực tuyến") : tr("⚪ Ngoại tuyến"), card);
    statusBadge->setStyleSheet(online
        ? QStringLiteral("color: #10b981; font-size: 10px; font-weight: 700;")
        : QStringLiteral("color: #64748b; font-size: 10px; font-weight: 700;"));
    headerRow->addWidget(statusBadge);
    layout->addLayout(headerRow);

    auto *idLabel = new QLabel(QStringLiteral("ID: %1").arg(id), card);
    idLabel->setStyleSheet(QStringLiteral("color: #94a3b8; font-size: 11px;"));
    layout->addWidget(idLabel);

    auto *btnRow = new QHBoxLayout;
    auto *btnConfig = new QPushButton(tr("⚙️ Cài đặt"), card);
    btnConfig->setStyleSheet(QStringLiteral("background: #0284c7; color: white; border: none; border-radius: 4px; padding: 4px 10px; font-weight: 600; font-size: 10px;"));
    connect(btnConfig, &QPushButton::clicked, this, [this, device] {
        openDeviceDrawer(device);
    });
    btnRow->addWidget(btnConfig);

    auto *btnOpen = new QPushButton(tr("Mở"), card);
    btnOpen->setStyleSheet(QStringLiteral("background: #059669; color: white; border: none; border-radius: 4px; padding: 4px 10px; font-weight: 600; font-size: 10px;"));
    connect(btnOpen, &QPushButton::clicked, this, [this, id] {
        emit relayControlRequested(id, true);
    });
    btnRow->addWidget(btnOpen);

    auto *btnClose = new QPushButton(tr("Đóng"), card);
    btnClose->setStyleSheet(QStringLiteral("background: #2563eb; color: white; border: none; border-radius: 4px; padding: 4px 10px; font-weight: 600; font-size: 10px;"));
    connect(btnClose, &QPushButton::clicked, this, [this, id] {
        emit relayControlRequested(id, false);
    });
    btnRow->addWidget(btnClose);

    layout->addLayout(btnRow);
    return card;
}

QWidget *DeviceManagementPage::createAvailableCard(const QJsonObject &device)
{
    auto *card = new QFrame;
    card->setObjectName(QStringLiteral("availCard"));
    card->setStyleSheet(QStringLiteral(
        "QFrame#availCard { background-color: #0f172a; border: 1.5px dashed #334155; border-radius: 10px; padding: 10px; }"));

    auto *layout = new QHBoxLayout(card);
    const QString id = device.value(QStringLiteral("device_id")).toString();

    auto *info = new QLabel(QStringLiteral("📡 %1 (Sẵn sàng)").arg(id), card);
    info->setStyleSheet(QStringLiteral("color: #e2e8f0; font-size: 12px; font-weight: 600;"));
    layout->addWidget(info);

    layout->addStretch();

    auto *btnClaim = new QPushButton(tr("+ Thêm vào tài khoản"), card);
    btnClaim->setStyleSheet(QStringLiteral("background: #10b981; color: white; border: none; border-radius: 5px; padding: 5px 12px; font-weight: 700; font-size: 11px;"));
    connect(btnClaim, &QPushButton::clicked, this, [this, id] {
        emit claimDeviceRequested(id, id);
    });
    layout->addWidget(btnClaim);

    return card;
}

void DeviceManagementPage::rebuildOwnedGrid()
{
    clearGrid(m_ownedGrid);
    m_ownedEmpty->setVisible(m_ownedDevices.isEmpty());

    int row = 0, col = 0;
    for (const auto &val : m_ownedDevices) {
        m_ownedGrid->addWidget(createOwnedCard(val.toObject()), row, col);
        col++;
        if (col >= 2) {
            col = 0;
            row++;
        }
    }
}

void DeviceManagementPage::rebuildAvailableGrid()
{
    clearGrid(m_availableGrid);
    m_availableEmpty->setVisible(m_availableDevices.isEmpty());

    int row = 0;
    for (const auto &val : m_availableDevices) {
        m_availableGrid->addWidget(createAvailableCard(val.toObject()), row, 0);
        row++;
    }
}

void DeviceManagementPage::rebuildLogTable()
{
    m_deviceLogTable->setRowCount(0);
    const QString now = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"));

    for (const auto &val : m_ownedDevices) {
        const QJsonObject dev = val.toObject();
        const QString did = dev.value(QStringLiteral("device_id")).toString();
        const QJsonObject metrics = dev.value(QStringLiteral("metrics")).toObject();

        const int row = m_deviceLogTable->rowCount();
        m_deviceLogTable->insertRow(row);
        m_deviceLogTable->setItem(row, 0, new QTableWidgetItem(now));
        m_deviceLogTable->setItem(row, 1, new QTableWidgetItem(did));
        m_deviceLogTable->setItem(row, 2, new QTableWidgetItem(metrics.value(QStringLiteral("door_state")).toString(QStringLiteral("CLOSED"))));
        m_deviceLogTable->setItem(row, 3, new QTableWidgetItem(metrics.value(QStringLiteral("motion_detected")).toBool() ? tr("🚶 Có người") : tr("Thông thoáng")));
        m_deviceLogTable->setItem(row, 4, new QTableWidgetItem(metrics.value(QStringLiteral("ir_blocked")).toBool() ? tr("⚠️ Bị chắn") : tr("An toàn")));
        m_deviceLogTable->setItem(row, 5, new QTableWidgetItem(QStringLiteral("%1 RPM | %2%")
            .arg(metrics.value(QStringLiteral("motor_speed_rpm")).toDouble(), 0, 'f', 0)
            .arg(metrics.value(QStringLiteral("door_position_pct")).toDouble(), 0, 'f', 0)));
    }
}

void DeviceManagementPage::openDeviceDrawer(const QJsonObject &device)
{
    m_selectedDevice = device;
    const QString did = device.value(QStringLiteral("device_id")).toString();
    const QString name = device.value(QStringLiteral("name")).toString(did);

    m_drawerId->setText(QStringLiteral("Thiết bị: %1 [%2]").arg(name, did));

    const QJsonObject cfg = device.value(QStringLiteral("config")).toObject();
    m_inputMotorSpeed->setValue(cfg.value(QStringLiteral("motor_speed_steps")).toInt(800));
    m_inputMaxSteps->setValue(cfg.value(QStringLiteral("max_travel_steps")).toInt(3200));
    m_inputAutoCloseDelay->setValue(cfg.value(QStringLiteral("auto_close_delay_s")).toInt(5));
    m_inputMicrostepping->setValue(cfg.value(QStringLiteral("microstepping")).toInt(8));
    m_chkAutoReverse->setChecked(cfg.value(QStringLiteral("auto_reverse")).toBool(true));
}

void DeviceManagementPage::saveThresholds()
{
    const QString did = m_selectedDevice.value(QStringLiteral("device_id")).toString();
    if (did.isEmpty()) return;

    QJsonObject cfg;
    cfg.insert(QStringLiteral("motor_speed_steps"), m_inputMotorSpeed->value());
    cfg.insert(QStringLiteral("max_travel_steps"), m_inputMaxSteps->value());
    cfg.insert(QStringLiteral("auto_close_delay_s"), m_inputAutoCloseDelay->value());
    cfg.insert(QStringLiteral("microstepping"), m_inputMicrostepping->value());
    cfg.insert(QStringLiteral("auto_reverse"), m_chkAutoReverse->isChecked());

    emit deviceConfigRequested(did, cfg);
}
