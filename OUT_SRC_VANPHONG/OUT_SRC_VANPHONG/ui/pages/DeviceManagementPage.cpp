#include "DeviceManagementPage.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QDateTime>
#include <QDoubleSpinBox>
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

    auto *topBar = new QHBoxLayout;
    auto *pageTitle = new QLabel(tr("📡 QUẢN LÝ THIẾT BỊ & THÔNG SỐ TƯỚI"), this);
    pageTitle->setStyleSheet(QStringLiteral("color: #34d399; font-size: 14px; font-weight: 800;"));
    topBar->addWidget(pageTitle);

    topBar->addStretch();

    m_cardsTabBtn = new QPushButton(tr("🎛️ Thiết Bị & Cài Đặt Ngưỡng"), this);
    m_cardsTabBtn->setCheckable(true);
    m_cardsTabBtn->setChecked(true);
    m_cardsTabBtn->setCursor(Qt::PointingHandCursor);
    m_cardsTabBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #0c2317; color: #a7f3d0; border: 1px solid #1b4332; border-radius: 6px; padding: 6px 14px; font-weight: 700; font-size: 11px; } "
        "QPushButton:checked { background: #059669; color: white; border-color: #10b981; }"));

    m_logTabBtn = new QPushButton(tr("📋 Nhật Ký Cảm Biến Trực Tiếp"), this);
    m_logTabBtn->setCheckable(true);
    m_logTabBtn->setCursor(Qt::PointingHandCursor);
    m_logTabBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #0c2317; color: #a7f3d0; border: 1px solid #1b4332; border-radius: 6px; padding: 6px 14px; font-weight: 700; font-size: 11px; } "
        "QPushButton:checked { background: #059669; color: white; border-color: #10b981; }"));

    topBar->addWidget(m_cardsTabBtn);
    topBar->addWidget(m_logTabBtn);

    auto *refreshBtn = new QPushButton(tr("🔄 Làm mới"), this);
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #1e3a2b; color: #f1f5f9; border: 1px solid #1b4332; border-radius: 6px; padding: 6px 12px; font-weight: 600; font-size: 11px; }"));
    connect(refreshBtn, &QPushButton::clicked, this, &DeviceManagementPage::refreshRequested);
    topBar->addWidget(refreshBtn);

    mainLayout->addLayout(topBar);

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

    // PAGE 0: Cards & Tuning Drawer
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

    auto *ownedTitle = new QLabel(tr("🌿 Bộ Điều Khiển Tưới Đã Gán"), leftContainer);
    ownedTitle->setStyleSheet(QStringLiteral("color: #34d399; font-size: 12px; font-weight: 800;"));
    leftLayout->addWidget(ownedTitle);

    m_ownedGrid = new QGridLayout;
    m_ownedGrid->setSpacing(10);
    leftLayout->addLayout(m_ownedGrid);

    m_ownedEmpty = new QLabel(tr("Chưa có thiết bị nào được gán."), leftContainer);
    m_ownedEmpty->setStyleSheet(QStringLiteral("color: #6ee7b7; font-style: italic; font-size: 11px;"));
    leftLayout->addWidget(m_ownedEmpty);

    auto *availTitle = new QLabel(tr("📡 Thiết Bị Mới Phát Hiện"), leftContainer);
    availTitle->setStyleSheet(QStringLiteral("color: #38bdf8; font-size: 12px; font-weight: 800;"));
    leftLayout->addWidget(availTitle);

    m_availableGrid = new QGridLayout;
    m_availableGrid->setSpacing(10);
    leftLayout->addLayout(m_availableGrid);

    m_availableEmpty = new QLabel(tr("Không có thiết bị trực tuyến chưa gán."), leftContainer);
    m_availableEmpty->setStyleSheet(QStringLiteral("color: #6ee7b7; font-style: italic; font-size: 11px;"));
    leftLayout->addWidget(m_availableEmpty);

    leftLayout->addStretch();
    leftScroll->setWidget(leftContainer);
    page0Layout->addWidget(leftScroll, 1);

    // Tuning Drawer
    m_drawer = new QFrame(page0);
    m_drawer->setMinimumWidth(280);
    m_drawer->setMaximumWidth(320);
    m_drawer->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #0c2317; border: 1.5px solid #1b4332; border-radius: 12px; }"));

    auto *drawerLayout = new QVBoxLayout(m_drawer);
    drawerLayout->setContentsMargins(14, 14, 14, 14);
    drawerLayout->setSpacing(10);

    m_drawerTitle = new QLabel(tr("⚙️ CÀI ĐẶT NGƯỠNG TƯỚI TỰ ĐỘNG"), m_drawer);
    m_drawerTitle->setStyleSheet(QStringLiteral("color: #34d399; font-size: 12px; font-weight: 800;"));
    drawerLayout->addWidget(m_drawerTitle);

    m_drawerId = new QLabel(tr("Chọn thiết bị để chỉnh sửa ngưỡng"), m_drawer);
    m_drawerId->setStyleSheet(QStringLiteral("color: #a7f3d0; font-size: 11px;"));
    drawerLayout->addWidget(m_drawerId);

    auto *formLayout = new QFormLayout;
    formLayout->setSpacing(8);

    m_inputMinSoil = new QDoubleSpinBox(m_drawer);
    m_inputMinSoil->setRange(10.0, 90.0);
    m_inputMinSoil->setValue(40.0);
    m_inputMinSoil->setSuffix(tr(" %"));
    m_inputMinSoil->setStyleSheet(QStringLiteral("background-color: #07170e; color: white; padding: 4px; border-radius: 4px;"));
    formLayout->addRow(new QLabel(tr("Bật bơm khi đất dưới:"), m_drawer), m_inputMinSoil);

    m_inputMaxSoil = new QDoubleSpinBox(m_drawer);
    m_inputMaxSoil->setRange(20.0, 100.0);
    m_inputMaxSoil->setValue(75.0);
    m_inputMaxSoil->setSuffix(tr(" %"));
    m_inputMaxSoil->setStyleSheet(QStringLiteral("background-color: #07170e; color: white; padding: 4px; border-radius: 4px;"));
    formLayout->addRow(new QLabel(tr("Ngắt bơm khi đất đạt:"), m_drawer), m_inputMaxSoil);

    m_inputMaxRuntime = new QSpinBox(m_drawer);
    m_inputMaxRuntime->setRange(1, 30);
    m_inputMaxRuntime->setValue(5);
    m_inputMaxRuntime->setSuffix(tr(" phút"));
    m_inputMaxRuntime->setStyleSheet(QStringLiteral("background-color: #07170e; color: white; padding: 4px; border-radius: 4px;"));
    formLayout->addRow(new QLabel(tr("Tưới tối đa mỗi lần:"), m_drawer), m_inputMaxRuntime);

    m_chkAutoWatering = new QCheckBox(tr("Kích hoạt chế độ tưới tự động"), m_drawer);
    m_chkAutoWatering->setChecked(true);
    m_chkAutoWatering->setStyleSheet(QStringLiteral("color: #e2e8f0; font-size: 11px;"));
    formLayout->addRow(QString(), m_chkAutoWatering);

    drawerLayout->addLayout(formLayout);

    m_saveConfigBtn = new QPushButton(tr("💾 Lưu & Đồng bộ MQTT"), m_drawer);
    m_saveConfigBtn->setCursor(Qt::PointingHandCursor);
    m_saveConfigBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #10b981; color: white; border: none; border-radius: 6px; padding: 8px; font-weight: 700; font-size: 11px; } "
        "QPushButton:hover { background: #059669; }"));
    connect(m_saveConfigBtn, &QPushButton::clicked, this, &DeviceManagementPage::saveThresholds);
    drawerLayout->addWidget(m_saveConfigBtn);

    m_releaseDevBtn = new QPushButton(tr("🗑️ Gỡ thiết bị"), m_drawer);
    m_releaseDevBtn->setCursor(Qt::PointingHandCursor);
    m_releaseDevBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #ef4444; color: white; border: none; border-radius: 6px; padding: 8px; font-weight: 700; font-size: 11px; } "
        "QPushButton:hover { background: #dc2626; }"));
    connect(m_releaseDevBtn, &QPushButton::clicked, this, [this] {
        QString did = m_selectedDevice.value(QStringLiteral("device_id")).toString();
        if (did.isEmpty() && !m_ownedDevices.isEmpty()) {
            did = m_ownedDevices.first().toObject().value(QStringLiteral("device_id")).toString();
        }
        if (!did.isEmpty()) {
            emit releaseDeviceRequested(did);
        }
    });
    drawerLayout->addWidget(m_releaseDevBtn);

    drawerLayout->addStretch();
    page0Layout->addWidget(m_drawer);

    m_viewStack->addWidget(page0);

    // PAGE 1: Real-time Telemetry Table
    auto *page1 = new QWidget(this);
    auto *page1Layout = new QVBoxLayout(page1);
    page1Layout->setContentsMargins(0, 0, 0, 0);

    m_deviceLogTable = new QTableWidget(page1);
    m_deviceLogTable->setColumnCount(6);
    m_deviceLogTable->setHorizontalHeaderLabels({
        tr("Thời gian"), tr("Device ID"), tr("Độ ẩm đất (%)"), tr("Nhiệt độ (°C)"), tr("Độ ẩm khí (%RH)"), tr("Máy bơm")
    });
    m_deviceLogTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_deviceLogTable->verticalHeader()->hide();
    m_deviceLogTable->setStyleSheet(QStringLiteral(
        "QTableWidget { background-color: #0c2317; border: 1px solid #1b4332; color: #f1f5f9; border-radius: 8px; gridline-color: #1b4332; } "
        "QHeaderView::section { background-color: #133925; color: #34d399; font-weight: 700; padding: 6px; border: none; }"));
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
        "QFrame#ownedCard { background-color: #0c2317; border: 1.5px solid #1b4332; border-radius: 10px; padding: 10px; } "
        "QFrame#ownedCard:hover { border-color: #34d399; }"));

    auto *layout = new QVBoxLayout(card);
    layout->setSpacing(6);

    const QString id = device.value(QStringLiteral("device_id")).toString();
    const QString name = device.value(QStringLiteral("name")).toString(id);
    const bool online = device.value(QStringLiteral("online")).toBool(true);

    auto *headerRow = new QHBoxLayout;
    auto *icon = new QLabel(QStringLiteral("🌿"), card);
    icon->setStyleSheet(QStringLiteral("font-size: 18px;"));
    headerRow->addWidget(icon);

    auto *nameLabel = new QLabel(name, card);
    nameLabel->setStyleSheet(QStringLiteral("color: #f1f5f9; font-size: 13px; font-weight: 700;"));
    headerRow->addWidget(nameLabel);

    headerRow->addStretch();

    auto *statusBadge = new QLabel(online ? tr("🟢 Trực tuyến") : tr("⚪ Ngoại tuyến"), card);
    statusBadge->setStyleSheet(online
        ? QStringLiteral("color: #34d399; font-size: 10px; font-weight: 700;")
        : QStringLiteral("color: #6ee7b7; font-size: 10px; font-weight: 700;"));
    headerRow->addWidget(statusBadge);
    layout->addLayout(headerRow);

    auto *idLabel = new QLabel(QStringLiteral("ID: %1").arg(id), card);
    idLabel->setStyleSheet(QStringLiteral("color: #a7f3d0; font-size: 11px;"));
    layout->addWidget(idLabel);

    auto *btnRow = new QHBoxLayout;
    auto *btnConfig = new QPushButton(tr("⚙️ Cài đặt"), card);
    btnConfig->setStyleSheet(QStringLiteral("background: #0284c7; color: white; border: none; border-radius: 4px; padding: 4px 10px; font-weight: 600; font-size: 10px;"));
    connect(btnConfig, &QPushButton::clicked, this, [this, device] {
        openDeviceDrawer(device);
    });
    btnRow->addWidget(btnConfig);

    auto *btnPump = new QPushButton(tr("💦 Bật tưới"), card);
    btnPump->setStyleSheet(QStringLiteral("background: #059669; color: white; border: none; border-radius: 4px; padding: 4px 10px; font-weight: 600; font-size: 10px;"));
    connect(btnPump, &QPushButton::clicked, this, [this, id] {
        emit relayControlRequested(id, true);
    });
    btnRow->addWidget(btnPump);

    auto *btnStop = new QPushButton(tr("🛑 Tắt"), card);
    btnStop->setStyleSheet(QStringLiteral("background: #dc2626; color: white; border: none; border-radius: 4px; padding: 4px 10px; font-weight: 600; font-size: 10px;"));
    connect(btnStop, &QPushButton::clicked, this, [this, id] {
        emit relayControlRequested(id, false);
    });
    btnRow->addWidget(btnStop);

    auto *btnRelease = new QPushButton(tr("🗑️ Gỡ"), card);
    btnRelease->setStyleSheet(QStringLiteral("background: #991b1b; color: white; border: none; border-radius: 4px; padding: 4px 10px; font-weight: 600; font-size: 10px;"));
    connect(btnRelease, &QPushButton::clicked, this, [this, id] {
        emit releaseDeviceRequested(id);
    });
    btnRow->addWidget(btnRelease);

    layout->addLayout(btnRow);
    return card;
}

QWidget *DeviceManagementPage::createAvailableCard(const QJsonObject &device)
{
    auto *card = new QFrame;
    card->setObjectName(QStringLiteral("availCard"));
    card->setStyleSheet(QStringLiteral(
        "QFrame#availCard { background-color: #0c2317; border: 1.5px dashed #1b4332; border-radius: 10px; padding: 10px; }"));

    auto *layout = new QHBoxLayout(card);
    const QString id = device.value(QStringLiteral("device_id")).toString();

    auto *info = new QLabel(QStringLiteral("📡 %1 (Sẵn sàng)").arg(id), card);
    info->setStyleSheet(QStringLiteral("color: #e2e8f0; font-size: 12px; font-weight: 600;"));
    layout->addWidget(info);

    layout->addStretch();

    auto *btnClaim = new QPushButton(tr("+ Thêm vào vườn"), card);
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

    if (!m_ownedDevices.isEmpty() && m_selectedDevice.isEmpty()) {
        openDeviceDrawer(m_ownedDevices.first().toObject());
    }

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
        m_deviceLogTable->setItem(row, 2, new QTableWidgetItem(QStringLiteral("%1%").arg(metrics.value(QStringLiteral("soil_moisture")).toDouble(55.0), 0, 'f', 1)));
        m_deviceLogTable->setItem(row, 3, new QTableWidgetItem(QStringLiteral("%1 °C").arg(metrics.value(QStringLiteral("temperature_c")).toDouble(27.5), 0, 'f', 1)));
        m_deviceLogTable->setItem(row, 4, new QTableWidgetItem(QStringLiteral("%1 %RH").arg(metrics.value(QStringLiteral("humidity")).toDouble(65.0), 0, 'f', 1)));
        m_deviceLogTable->setItem(row, 5, new QTableWidgetItem(metrics.value(QStringLiteral("pump_active")).toBool() ? tr("ĐANG BƠM 💦") : tr("TẮT ⚪")));
    }
}

void DeviceManagementPage::openDeviceDrawer(const QJsonObject &device)
{
    m_selectedDevice = device;
    const QString did = device.value(QStringLiteral("device_id")).toString();
    const QString name = device.value(QStringLiteral("name")).toString(did);

    m_drawerId->setText(QStringLiteral("Bộ tưới: %1 [%2]").arg(name, did));

    const QJsonObject cfg = device.value(QStringLiteral("config")).toObject();
    m_inputMinSoil->setValue(cfg.value(QStringLiteral("min_soil_moisture")).toDouble(40.0));
    m_inputMaxSoil->setValue(cfg.value(QStringLiteral("max_soil_moisture")).toDouble(75.0));
    m_inputMaxRuntime->setValue(cfg.value(QStringLiteral("max_pump_runtime_m")).toInt(5));
    m_chkAutoWatering->setChecked(cfg.value(QStringLiteral("auto_watering")).toBool(true));
}

void DeviceManagementPage::saveThresholds()
{
    const QString did = m_selectedDevice.value(QStringLiteral("device_id")).toString();
    if (did.isEmpty()) return;

    QJsonObject cfg;
    cfg.insert(QStringLiteral("min_soil_moisture"), m_inputMinSoil->value());
    cfg.insert(QStringLiteral("max_soil_moisture"), m_inputMaxSoil->value());
    cfg.insert(QStringLiteral("max_pump_runtime_m"), m_inputMaxRuntime->value());
    cfg.insert(QStringLiteral("auto_watering"), m_chkAutoWatering->isChecked());

    emit deviceConfigRequested(did, cfg);
}
