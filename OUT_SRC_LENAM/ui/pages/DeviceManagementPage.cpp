#include "DeviceManagementPage.h"

#include <QDateTime>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QStringList>

#include <functional>

namespace {
class ClickableFrame final : public QFrame
{
public:
    using QFrame::QFrame;
    std::function<void()> clicked;
protected:
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        QFrame::mouseReleaseEvent(event);
        if (event->button() == Qt::LeftButton && rect().contains(event->position().toPoint()) && clicked)
            clicked();
    }
};

class RelayToggle final : public QPushButton
{
public:
    explicit RelayToggle(bool on, QWidget *parent = nullptr)
        : QPushButton(parent), m_on(on)
    {
        setCursor(Qt::PointingHandCursor);
        setFixedHeight(32);
        setFlat(true);
    }

    void setPending()
    {
        m_pending = true;
        setEnabled(false);
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(isDown() ? QColor("#dbece5") : QColor("#edf7f2"));
        painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 9, 9);
        const QColor textColor = isEnabled() ? QColor("#29473a") : QColor("#8a9992");
        painter.setPen(textColor);
        QFont textFont = font();
        textFont.setPointSize(9);
        textFont.setWeight(QFont::DemiBold);
        painter.setFont(textFont);
        painter.drawText(QRect(10, 0, width() - 58, height()),
                         Qt::AlignVCenter | Qt::AlignLeft,
                         m_pending ? tr("Đang gửi…")
                                   : (m_on ? tr("Relay bật") : tr("Relay tắt")));

        const QRectF track(width() - 52, 7, 42, 18);
        painter.setPen(Qt::NoPen);
        painter.setBrush(!isEnabled() ? QColor("#ccd5d1")
                         : m_on ? QColor("#15945a") : QColor("#aab7b1"));
        painter.drawRoundedRect(track, 10, 10);
        const qreal knobX = m_on ? track.right() - 15 : track.left() + 2;
        painter.setBrush(Qt::white);
        painter.drawEllipse(QRectF(knobX, track.top() + 2, 14, 14));
    }

private:
    bool m_on;
    bool m_pending = false;
};
}

DeviceManagementPage::DeviceManagementPage(QWidget *parent)
    : QWidget(parent),
      m_ownedGrid(new QGridLayout),
      m_availableGrid(new QGridLayout),
      m_ownedEmpty(new QLabel(tr("Bạn chưa thêm thiết bị nào."), this)),
      m_availableEmpty(new QLabel(tr("Đang tìm thiết bị online..."), this)),
      m_liveLabel(new QLabel(tr("●  Đang cập nhật realtime"), this)),
      m_refreshTimer(new QTimer(this)),
      m_drawer(new QFrame(this)),
      m_drawerIcon(new QLabel(this)),
      m_drawerName(new QLabel(this)),
      m_drawerId(new QLabel(this)),
      m_drawerMetrics(new QLabel(this)),
      m_thresholdTitle(new QLabel(tr("Ngưỡng cảnh báo"), this)),
      m_thresholdForm(new QFormLayout),
      m_samplingInterval(new QSpinBox(this)),
      m_saveThresholds(new QPushButton(tr("Lưu & gửi xuống thiết bị"), this)),
      m_releaseDevice(new QPushButton(tr("Xóa thiết bị khỏi tài khoản"), this))
{
    setObjectName(QStringLiteral("DeviceManagementPage"));
    auto *outer = new QHBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    auto *mainPanel = new QWidget(this);
    mainPanel->setObjectName(QStringLiteral("deviceMainPanel"));
    auto *root = new QVBoxLayout(mainPanel);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(18);

    auto *header = new QHBoxLayout;
    auto *titles = new QVBoxLayout;
    auto *title = new QLabel(tr("My Home"), this);
    title->setObjectName(QStringLiteral("devicePageTitle"));
    auto *subtitle = new QLabel(
        tr(""), this);
    subtitle->setObjectName(QStringLiteral("devicePageSubtitle"));
    titles->addWidget(title);
    titles->addWidget(subtitle);
    m_liveLabel->setObjectName(QStringLiteral("liveBadge"));
    header->addLayout(titles);
    header->addStretch();
    header->addWidget(m_liveLabel, 0, Qt::AlignTop);
    root->addLayout(header);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *content = new QWidget(scroll);
    content->setObjectName(QStringLiteral("devicePageContent"));
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 4, 20);
    contentLayout->setSpacing(16);

    auto *ownedTitle = new QLabel(tr("House  ›"), content);
    ownedTitle->setObjectName(QStringLiteral("deviceSectionTitle"));
    contentLayout->addWidget(ownedTitle);
    m_ownedGrid->setHorizontalSpacing(14);
    m_ownedGrid->setVerticalSpacing(14);
    contentLayout->addLayout(m_ownedGrid);
    m_ownedEmpty->setObjectName(QStringLiteral("deviceEmptyState"));
    contentLayout->addWidget(m_ownedEmpty);

    auto *availableHeader = new QHBoxLayout;
    auto *availableTitles = new QVBoxLayout;
    auto *availableTitle = new QLabel(tr("Bedroom  ›"), content);
    availableTitle->setObjectName(QStringLiteral("deviceSectionTitle"));
    auto *availableHint = new QLabel(
        tr("Thiết bị online chưa thuộc tài khoản nào"), content);
    availableHint->setObjectName(QStringLiteral("deviceSectionHint"));
    availableTitles->addWidget(availableTitle);
    availableTitles->addWidget(availableHint);
    auto *refreshButton = new QPushButton(tr("↻  Làm mới"), content);
    refreshButton->setObjectName(QStringLiteral("refreshDevicesButton"));
    availableHeader->addLayout(availableTitles);
    availableHeader->addStretch();
    availableHeader->addWidget(refreshButton);
    contentLayout->addLayout(availableHeader);
    m_availableGrid->setHorizontalSpacing(14);
    m_availableGrid->setVerticalSpacing(14);
    contentLayout->addLayout(m_availableGrid);
    m_availableEmpty->setObjectName(QStringLiteral("deviceEmptyState"));
    contentLayout->addWidget(m_availableEmpty);
    contentLayout->addStretch();

    scroll->setWidget(content);
    root->addWidget(scroll, 1);

    m_drawer->setObjectName(QStringLiteral("deviceDrawer"));
    m_drawer->setFixedWidth(330);
    auto *drawerLayout = new QVBoxLayout(m_drawer);
    drawerLayout->setContentsMargins(22, 20, 22, 20);
    drawerLayout->setSpacing(12);
    auto *drawerTop = new QHBoxLayout;
    auto *drawerTitle = new QLabel(tr("Chi tiết thiết bị"), m_drawer);
    drawerTitle->setObjectName(QStringLiteral("drawerTitle"));
    auto *closeDrawer = new QPushButton(QStringLiteral("×"), m_drawer);
    closeDrawer->setObjectName(QStringLiteral("closeDrawerButton"));
    drawerTop->addWidget(drawerTitle);
    drawerTop->addStretch();
    drawerTop->addWidget(closeDrawer);
    drawerLayout->addLayout(drawerTop);
    m_drawerIcon->setObjectName(QStringLiteral("drawerDeviceIcon"));
    m_drawerIcon->setAlignment(Qt::AlignCenter);
    m_drawerName->setObjectName(QStringLiteral("drawerDeviceName"));
    m_drawerId->setObjectName(QStringLiteral("drawerDeviceId"));
    m_drawerMetrics->setObjectName(QStringLiteral("drawerMetrics"));
    m_drawerMetrics->setWordWrap(true);
    drawerLayout->addWidget(m_drawerIcon, 0, Qt::AlignLeft);
    drawerLayout->addWidget(m_drawerName);
    drawerLayout->addWidget(m_drawerId);
    drawerLayout->addWidget(m_drawerMetrics);
    m_thresholdTitle->setObjectName(QStringLiteral("drawerSectionTitle"));
    drawerLayout->addWidget(m_thresholdTitle);
    auto *thresholdWidget = new QWidget(m_drawer);
    thresholdWidget->setLayout(m_thresholdForm);
    drawerLayout->addWidget(thresholdWidget);
    m_samplingInterval->setRange(1, 3600);
    m_samplingInterval->setSuffix(tr(" giây"));
    m_thresholdForm->addRow(tr("Chu kỳ gửi"), m_samplingInterval);
    m_saveThresholds->setObjectName(QStringLiteral("saveDeviceConfigButton"));
    drawerLayout->addWidget(m_saveThresholds);
    drawerLayout->addStretch();
    m_releaseDevice->setObjectName(QStringLiteral("releaseDeviceButton"));
    drawerLayout->addWidget(m_releaseDevice);
    m_drawer->hide();
    outer->addWidget(mainPanel, 1);
    outer->addWidget(m_drawer);

    m_refreshTimer->setInterval(5000);
    connect(m_refreshTimer, &QTimer::timeout, this, &DeviceManagementPage::refreshRequested);
    connect(refreshButton, &QPushButton::clicked, this, &DeviceManagementPage::refreshRequested);
    connect(closeDrawer, &QPushButton::clicked, m_drawer, &QWidget::hide);
    connect(m_saveThresholds, &QPushButton::clicked,
            this, &DeviceManagementPage::saveThresholds);
    connect(m_releaseDevice, &QPushButton::clicked, this, [this] {
        const QString deviceId = m_selectedDevice.value(QStringLiteral("device_id")).toString();
        if (deviceId.isEmpty())
            return;

        QDialog dialog(this);
        dialog.setObjectName(QStringLiteral("releaseDeviceDialog"));
        dialog.setWindowTitle(tr("Xóa thiết bị"));
        dialog.setModal(true);
        dialog.setFixedWidth(430);
        auto *root = new QVBoxLayout(&dialog);
        root->setContentsMargins(24, 22, 24, 22);
        root->setSpacing(16);

        auto *head = new QHBoxLayout;
        auto *icon = new QLabel(QStringLiteral("!"), &dialog);
        icon->setObjectName(QStringLiteral("releaseDeviceDialogIcon"));
        icon->setAlignment(Qt::AlignCenter);
        auto *titleBlock = new QVBoxLayout;
        auto *title = new QLabel(tr("Xóa thiết bị?"), &dialog);
        title->setObjectName(QStringLiteral("releaseDeviceDialogTitle"));
        auto *hint = new QLabel(tr("Thiết bị sẽ được gỡ khỏi tài khoản và có thể thêm lại nếu đang online."), &dialog);
        hint->setObjectName(QStringLiteral("releaseDeviceDialogHint"));
        hint->setWordWrap(true);
        titleBlock->addWidget(title);
        titleBlock->addWidget(hint);
        head->addWidget(icon);
        head->addLayout(titleBlock, 1);
        root->addLayout(head);

        auto *device = new QLabel(tr("Device ID: %1").arg(deviceId), &dialog);
        device->setObjectName(QStringLiteral("releaseDeviceInfo"));
        root->addWidget(device);

        auto *actions = new QHBoxLayout;
        actions->setSpacing(10);
        auto *cancel = new QPushButton(tr("Hủy"), &dialog);
        auto *confirm = new QPushButton(tr("Xóa thiết bị"), &dialog);
        cancel->setObjectName(QStringLiteral("releaseDeviceCancelButton"));
        confirm->setObjectName(QStringLiteral("releaseDeviceConfirmButton"));
        actions->addWidget(cancel);
        actions->addWidget(confirm);
        root->addLayout(actions);
        connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
        connect(confirm, &QPushButton::clicked, &dialog, &QDialog::accept);
        if (dialog.exec() != QDialog::Accepted)
            return;

        m_releaseDevice->setEnabled(false);
        m_releaseDevice->setText(tr("Đang xóa..."));
        emit releaseDeviceRequested(deviceId);
    });
}

void DeviceManagementPage::setOwnedDevices(const QJsonArray &devices)
{
    m_ownedDevices = devices;
    rebuildOwnedGrid();
    if (m_drawer->isVisible() && !m_selectedDevice.isEmpty()) {
        const QString selectedId = m_selectedDevice.value(QStringLiteral("device_id")).toString();
        bool selectedStillExists = false;
        for (const QJsonValue &value : devices) {
            const QJsonObject current = value.toObject();
            if (current.value(QStringLiteral("device_id")).toString() == selectedId) {
                selectedStillExists = true;
                m_selectedDevice = current;
                m_drawerMetrics->setText(metricsSummary(
                    current.value(QStringLiteral("metrics")).toObject()));
                break;
            }
        }
        if (!selectedStillExists) {
            m_selectedDevice = {};
            m_drawer->hide();
        }
    }
}

void DeviceManagementPage::setAvailableDevices(const QJsonArray &devices)
{
    m_availableDevices = devices;
    rebuildAvailableGrid();
    m_liveLabel->setText(tr("●  Vừa cập nhật %1")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"))));
}

void DeviceManagementPage::startRealtime()
{
    if (!m_refreshTimer->isActive())
        m_refreshTimer->start();
    emit refreshRequested();
}

void DeviceManagementPage::stopRealtime()
{
    m_refreshTimer->stop();
}

void DeviceManagementPage::configSaved(const QString &deviceId, bool mqttPublished)
{
    if (m_selectedDevice.value(QStringLiteral("device_id")).toString() != deviceId)
        return;
    m_saveThresholds->setEnabled(true);
    m_saveThresholds->setText(mqttPublished
        ? tr("Đã lưu và gửi xuống thiết bị")
        : tr("Đã lưu · MQTT đang offline"));
    QTimer::singleShot(1800, this, [this] {
        m_saveThresholds->setText(tr("Lưu & gửi xuống thiết bị"));
    });
}

QWidget *DeviceManagementPage::createOwnedCard(const QJsonObject &device)
{
    auto *card = new ClickableFrame(this);
    card->setObjectName(QStringLiteral("ownedDeviceCard"));
    const QJsonArray capabilities = device.value(QStringLiteral("capabilities")).toArray();
    bool hasRelay = false;
    for (const QJsonValue &capability : capabilities)
        hasRelay = hasRelay || capability.toString() == QStringLiteral("relay");

    card->setFixedSize(520, 122);
    card->setCursor(Qt::PointingHandCursor);
    const QString type = device.value(QStringLiteral("device_type")).toString();
    const bool online = device.value(QStringLiteral("online")).toBool();
    const QString deviceId = device.value(QStringLiteral("device_id")).toString();
    const QJsonObject metricsObject = device.value(QStringLiteral("metrics")).toObject();
    card->setProperty("deviceType", type);

    auto *layout = new QHBoxLayout(card);
    layout->setContentsMargins(18, 15, 18, 15);
    layout->setSpacing(10);

    auto *icon = new QLabel(deviceIcon(type), card);
    icon->setObjectName(QStringLiteral("deviceTypeIcon"));
    icon->setProperty("deviceType", type);
    icon->setAlignment(Qt::AlignCenter);
    layout->addWidget(icon, 0, Qt::AlignVCenter);

    auto *textBlock = new QVBoxLayout;
    textBlock->setContentsMargins(0, 0, 0, 0);
    textBlock->setSpacing(5);

    auto *name = new QLabel(device.value(QStringLiteral("name")).toString(), card);
    name->setObjectName(QStringLiteral("deviceCardName"));
    name->setWordWrap(false);
    name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    textBlock->addWidget(name);

    auto *id = new QLabel(tr("ID  %1").arg(deviceId), card);
    id->setObjectName(QStringLiteral("deviceCardId"));
    textBlock->addWidget(id);

    QStringList metricParts;
    if (metricsObject.contains(QStringLiteral("flow_l_min")))
        metricParts << QStringLiteral("%1 L/min").arg(metricsObject.value(QStringLiteral("flow_l_min")).toDouble(), 0, 'f', 2);
    if (metricsObject.contains(QStringLiteral("total_liters")))
        metricParts << tr("Tổng %1 L").arg(metricsObject.value(QStringLiteral("total_liters")).toDouble(), 0, 'f', 2);
    if (metricsObject.contains(QStringLiteral("pump_on")))
        metricParts << (metricsObject.value(QStringLiteral("pump_on")).toBool() ? tr("Bơm bật") : tr("Bơm tắt"));
    if (metricsObject.contains(QStringLiteral("temperature_c")))
        metricParts << QStringLiteral("%1°C").arg(metricsObject.value(QStringLiteral("temperature_c")).toDouble(), 0, 'f', 1);
    if (metricsObject.contains(QStringLiteral("sound_vpp")))
        metricParts << QStringLiteral("%1 Vpp").arg(metricsObject.value(QStringLiteral("sound_vpp")).toDouble(), 0, 'f', 3);
    if (metricsObject.contains(QStringLiteral("pressure_hpa")))
        metricParts << QStringLiteral("%1 hPa").arg(metricsObject.value(QStringLiteral("pressure_hpa")).toDouble(), 0, 'f', 0);
    if (metricsObject.contains(QStringLiteral("uv_index")))
        metricParts << QStringLiteral("UV %1").arg(metricsObject.value(QStringLiteral("uv_index")).toDouble(), 0, 'f', 1);
    auto *metrics = new QLabel(metricParts.isEmpty() ? deviceTypeName(type) : metricParts.join(QStringLiteral("  •  ")), card);
    metrics->setObjectName(QStringLiteral("deviceSecondaryMetric"));
    metrics->setWordWrap(false);
    textBlock->addWidget(metrics);
    textBlock->addStretch();
    layout->addLayout(textBlock, 1);

    auto *rightBlock = new QVBoxLayout;
    rightBlock->setContentsMargins(0, 0, 0, 0);
    rightBlock->setSpacing(10);
    auto *status = new QLabel(online ? tr("●  Online") : tr("●  Offline"), card);
    status->setObjectName(online ? QStringLiteral("onlinePill") : QStringLiteral("offlinePill"));
    status->setAlignment(Qt::AlignCenter);
    status->setFixedSize(96, 28);
    rightBlock->addWidget(status, 0, Qt::AlignRight);
    rightBlock->addStretch();

    if (hasRelay) {
        const QJsonObject stateObject = device.value(QStringLiteral("state")).toObject();
        const bool relayOn = stateObject.contains(QStringLiteral("relay"))
                                 ? stateObject.value(QStringLiteral("relay")).toBool(false)
                                 : metricsObject.value(QStringLiteral("pump_on")).toBool(false);
        auto *relayButton = new RelayToggle(relayOn, card);
        relayButton->setEnabled(online);
        relayButton->setFixedSize(150, 32);
        rightBlock->addWidget(relayButton, 0, Qt::AlignRight);
        connect(relayButton, &QPushButton::clicked, this,
                [this, deviceId, relayOn, relayButton] {
                    relayButton->setPending();
                    emit relayControlRequested(deviceId, !relayOn);
                });
    } else if (metricsObject.contains(QStringLiteral("ir_detected"))) {
        const bool irDetected = metricsObject.value(QStringLiteral("ir_detected")).toInt() != 0;
        auto *irStatus = new QLabel(irDetected ? tr("●  Có vật") : tr("●  Không có vật"), card);
        irStatus->setObjectName(irDetected ? QStringLiteral("irDetectedStatus")
                                           : QStringLiteral("irClearStatus"));
        irStatus->setAlignment(Qt::AlignCenter);
        irStatus->setFixedSize(150, 32);
        rightBlock->addWidget(irStatus, 0, Qt::AlignRight);
    }
    layout->addLayout(rightBlock);

    card->clicked = [this, device] { openDeviceDrawer(device); };
    return card;
}

QWidget *DeviceManagementPage::createAvailableCard(const QJsonObject &device)
{
    auto *card = new QFrame(this);
    card->setObjectName(QStringLiteral("availableDeviceCard"));
    card->setFixedSize(520, 122);
    const QString deviceId = device.value(QStringLiteral("device_id")).toString();
    const QString type = device.value(QStringLiteral("device_type")).toString();
    card->setProperty("deviceType", type);

    auto *layout = new QHBoxLayout(card);
    layout->setContentsMargins(18, 15, 18, 15);
    layout->setSpacing(10);

    auto *icon = new QLabel(deviceIcon(type), card);
    icon->setObjectName(QStringLiteral("availableDeviceIcon"));
    icon->setProperty("deviceType", type);
    icon->setAlignment(Qt::AlignCenter);
    layout->addWidget(icon, 0, Qt::AlignVCenter);

    auto *textBlock = new QVBoxLayout;
    textBlock->setContentsMargins(0, 0, 0, 0);
    textBlock->setSpacing(5);
    auto *id = new QLabel(deviceId, card);
    id->setObjectName(QStringLiteral("availableDeviceId"));
    id->setWordWrap(false);
    auto *typeName = new QLabel(deviceTypeName(type), card);
    typeName->setObjectName(QStringLiteral("deviceCardType"));
    typeName->setWordWrap(false);
    auto *hint = new QLabel(tr("Thiết bị đang chờ liên kết"), card);
    hint->setObjectName(QStringLiteral("availableDeviceHint"));
    hint->setWordWrap(false);
    textBlock->addWidget(id);
    textBlock->addWidget(typeName);
    textBlock->addWidget(hint);
    textBlock->addStretch();
    layout->addLayout(textBlock, 1);

    auto *rightBlock = new QVBoxLayout;
    rightBlock->setContentsMargins(0, 0, 0, 0);
    rightBlock->setSpacing(10);
    auto *status = new QLabel(tr("●  Online"), card);
    status->setObjectName(QStringLiteral("onlinePill"));
    status->setAlignment(Qt::AlignCenter);
    status->setFixedSize(96, 28);
    rightBlock->addWidget(status, 0, Qt::AlignRight);
    rightBlock->addStretch();
    auto *button = new QPushButton(tr("+  Thêm"), card);
    button->setObjectName(QStringLiteral("claimDeviceButton"));
    button->setFixedSize(150, 32);
    rightBlock->addWidget(button, 0, Qt::AlignRight);
    layout->addLayout(rightBlock);

    connect(button, &QPushButton::clicked, this, [this, deviceId, type] {
        QDialog dialog(this);
        dialog.setObjectName(QStringLiteral("claimDeviceDialog"));
        dialog.setWindowTitle(tr("Đặt tên thiết bị"));
        dialog.setModal(true);
        dialog.setFixedWidth(420);

        auto *root = new QVBoxLayout(&dialog);
        root->setContentsMargins(24, 22, 24, 22);
        root->setSpacing(16);

        auto *head = new QHBoxLayout;
        auto *dialogIcon = new QLabel(deviceIcon(type), &dialog);
        dialogIcon->setObjectName(QStringLiteral("claimDeviceDialogIcon"));
        dialogIcon->setProperty("deviceType", type);
        dialogIcon->setAlignment(Qt::AlignCenter);
        auto *titleBlock = new QVBoxLayout;
        auto *title = new QLabel(tr("Thêm thiết bị mới"), &dialog);
        title->setObjectName(QStringLiteral("claimDeviceDialogTitle"));
        auto *subtitle = new QLabel(tr("Đặt tên dễ nhớ để quản lý trên app."), &dialog);
        subtitle->setObjectName(QStringLiteral("claimDeviceDialogHint"));
        subtitle->setWordWrap(true);
        titleBlock->addWidget(title);
        titleBlock->addWidget(subtitle);
        head->addWidget(dialogIcon);
        head->addLayout(titleBlock, 1);
        root->addLayout(head);

        auto *info = new QLabel(tr("ID: %1  •  %2").arg(deviceId, deviceTypeName(type)), &dialog);
        info->setObjectName(QStringLiteral("claimDeviceInfo"));
        info->setWordWrap(true);
        root->addWidget(info);

        auto *name = new QLineEdit(deviceTypeName(type), &dialog);
        name->setObjectName(QStringLiteral("claimDeviceNameInput"));
        name->setPlaceholderText(tr("VD: Phòng khách, Khu A..."));
        name->selectAll();
        root->addWidget(name);

        auto *actions = new QHBoxLayout;
        actions->setSpacing(10);
        auto *cancel = new QPushButton(tr("Hủy"), &dialog);
        auto *save = new QPushButton(tr("Thêm thiết bị"), &dialog);
        cancel->setObjectName(QStringLiteral("claimDeviceCancelButton"));
        save->setObjectName(QStringLiteral("claimDeviceSaveButton"));
        actions->addWidget(cancel);
        actions->addWidget(save);
        root->addLayout(actions);

        connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
        connect(save, &QPushButton::clicked, &dialog, &QDialog::accept);
        if (dialog.exec() != QDialog::Accepted)
            return;
        const QString displayName = name->text().trimmed();
        emit claimDeviceRequested(deviceId, displayName.isEmpty() ? deviceId : displayName);
    });
    return card;
}

void DeviceManagementPage::rebuildOwnedGrid()
{
    clearGrid(m_ownedGrid);
    m_ownedEmpty->setVisible(m_ownedDevices.isEmpty());
    for (int i = 0; i < m_ownedDevices.size(); ++i)
        m_ownedGrid->addWidget(createOwnedCard(m_ownedDevices.at(i).toObject()),
                               i / 2, i % 2, Qt::AlignLeft | Qt::AlignTop);
    m_ownedGrid->setColumnStretch(2, 1);
}

void DeviceManagementPage::rebuildAvailableGrid()
{
    clearGrid(m_availableGrid);
    m_availableEmpty->setVisible(m_availableDevices.isEmpty());
    m_availableEmpty->setText(tr("Không có thiết bị online nào đang chờ thêm."));
    for (int i = 0; i < m_availableDevices.size(); ++i)
        m_availableGrid->addWidget(createAvailableCard(m_availableDevices.at(i).toObject()),
                                   i / 2, i % 2, Qt::AlignLeft | Qt::AlignTop);
    m_availableGrid->setColumnStretch(2, 1);
}

void DeviceManagementPage::openDeviceDrawer(const QJsonObject &device)
{
    m_selectedDevice = device;
    m_releaseDevice->setEnabled(true);
    m_releaseDevice->setText(tr("Xóa thiết bị khỏi tài khoản"));
    const QString type = device.value(QStringLiteral("device_type")).toString();
    m_drawerIcon->setText(deviceIcon(type));
    m_drawerName->setText(device.value(QStringLiteral("name")).toString());
    m_drawerId->setText(tr("Device ID  %1")
        .arg(device.value(QStringLiteral("device_id")).toString()));
    m_drawerMetrics->setText(metricsSummary(
        device.value(QStringLiteral("metrics")).toObject()));
    rebuildThresholdForm(device);
    m_drawer->show();
}

void DeviceManagementPage::rebuildThresholdForm(const QJsonObject &device)
{
    while (QLayoutItem *item = m_thresholdForm->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    m_thresholdInputs.clear();
    const QString type = device.value(QStringLiteral("device_type")).toString();
    const QJsonObject saved = device.value(QStringLiteral("config")).toObject();
    const QJsonObject savedThresholds = saved.value(QStringLiteral("thresholds")).toObject();

    auto addThreshold = [this, &savedThresholds](const QString &key, const QString &label,
                                                 double fallback, double minimum,
                                                 double maximum, const QString &suffix) {
        const QStringList parts = key.split('.');
        double value = fallback;
        if (parts.size() == 2)
            value = savedThresholds.value(parts.at(0)).toObject()
                        .value(parts.at(1)).toDouble(fallback);
        auto *input = new QDoubleSpinBox(m_drawer);
        input->setRange(minimum, maximum);
        input->setDecimals(2);
        input->setValue(value);
        input->setSuffix(suffix);
        m_thresholdInputs.insert(key, input);
        m_thresholdForm->addRow(label, input);
    };

    if (type == QStringLiteral("uv_pressure")) {
        addThreshold(QStringLiteral("uv_index.warning_above"), tr("UV cảnh báo"), 6, 0, 20, QString());
        addThreshold(QStringLiteral("uv_index.critical_above"), tr("UV nguy hiểm"), 8, 0, 20, QString());
        addThreshold(QStringLiteral("pressure_hpa.min"), tr("Áp suất thấp"), 990, 100, 1500, tr(" hPa"));
        addThreshold(QStringLiteral("pressure_hpa.max"), tr("Áp suất cao"), 1030, 100, 1500, tr(" hPa"));
    } else if (type == QStringLiteral("temperature_sound")) {
        addThreshold(QStringLiteral("temperature_c.warning_above"), tr("Nhiệt độ cảnh báo"), 40, -40, 150, tr(" °C"));
        addThreshold(QStringLiteral("temperature_c.critical_above"), tr("Nhiệt độ nguy hiểm"), 50, -40, 150, tr(" °C"));
        addThreshold(QStringLiteral("sound_vpp.warning_above"), tr("Âm thanh cảnh báo"), 1.5, 0, 3.3, tr(" Vpp"));
    } else if (type == QStringLiteral("weather_pressure")) {
        addThreshold(QStringLiteral("temperature_c.min"), tr("Nhiệt độ thấp"), 0, -40, 150, tr(" °C"));
        addThreshold(QStringLiteral("temperature_c.max"), tr("Nhiệt độ cao"), 50, -40, 150, tr(" °C"));
        addThreshold(QStringLiteral("pressure_hpa.min"), tr("Áp suất thấp"), 990, 100, 1500, tr(" hPa"));
        addThreshold(QStringLiteral("pressure_hpa.max"), tr("Áp suất cao"), 1030, 100, 1500, tr(" hPa"));
    } else if (type == QStringLiteral("water_flow_pump")) {
        addThreshold(QStringLiteral("flow_l_min.min"), tr("Lưu lượng tối thiểu"), 0.20, 0, 60, tr(" L/min"));
        addThreshold(QStringLiteral("flow_l_min.max"), tr("Lưu lượng tối đa"), 20.00, 0, 60, tr(" L/min"));
        addThreshold(QStringLiteral("total_liters.max"), tr("Tổng nước cảnh báo"), 100.00, 0, 100000, tr(" L"));
    }

    m_samplingInterval = new QSpinBox(m_drawer);
    m_samplingInterval->setRange(1, 3600);
    m_samplingInterval->setSuffix(tr(" giây"));
    m_samplingInterval->setValue(saved.value(QStringLiteral("sampling_interval_ms"))
                                     .toInt(2000) / 1000);
    const bool hasThresholds = !m_thresholdInputs.isEmpty();
    m_thresholdForm->addRow(tr("Chu kỳ gửi"), m_samplingInterval);
    m_thresholdForm->setRowVisible(m_samplingInterval, hasThresholds);
    m_saveThresholds->setVisible(hasThresholds);
    m_thresholdTitle->setVisible(hasThresholds);
}

void DeviceManagementPage::saveThresholds()
{
    if (m_selectedDevice.isEmpty() || m_thresholdInputs.isEmpty())
        return;
    QJsonObject thresholds;
    for (auto it = m_thresholdInputs.cbegin(); it != m_thresholdInputs.cend(); ++it) {
        const QStringList parts = it.key().split('.');
        if (parts.size() != 2)
            continue;
        QJsonObject sensor = thresholds.value(parts.at(0)).toObject();
        sensor.insert(parts.at(1), it.value()->value());
        thresholds.insert(parts.at(0), sensor);
    }
    const QJsonObject config{{"sampling_interval_ms", m_samplingInterval->value() * 1000},
                             {"thresholds", thresholds}};
    m_saveThresholds->setEnabled(false);
    m_saveThresholds->setText(tr("Đang lưu..."));
    emit deviceConfigRequested(
        m_selectedDevice.value(QStringLiteral("device_id")).toString(), config);
}

void DeviceManagementPage::clearGrid(QGridLayout *layout)
{
    while (QLayoutItem *item = layout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
}

QString DeviceManagementPage::deviceIcon(const QString &type)
{
    if (type == QStringLiteral("uv_pressure")) return QStringLiteral("☀");
    if (type == QStringLiteral("temperature_sound")) return QStringLiteral("♫");
    if (type == QStringLiteral("weather_pressure")) return QStringLiteral("☁");
    if (type == QStringLiteral("electric_power")) return QStringLiteral("⚡");
    if (type == QStringLiteral("water_flow_pump")) return QStringLiteral("🚰");
    return QStringLiteral("◆");
}

QString DeviceManagementPage::deviceTypeName(const QString &type)
{
    if (type == QStringLiteral("uv_pressure")) return tr("Cảm biến UV & áp suất");
    if (type == QStringLiteral("temperature_sound")) return tr("Nhiệt độ & âm thanh");
    if (type == QStringLiteral("weather_pressure")) return tr("Cảm biến môi trường");
    if (type == QStringLiteral("electric_power")) return tr("Đo điện áp & dòng điện");
    if (type == QStringLiteral("water_flow_pump")) return tr("Bơm & lưu lượng nước");
    return tr("Thiết bị IoT");
}

QString DeviceManagementPage::metricsSummary(const QJsonObject &metrics)
{
    QStringList values;
    if (metrics.contains(QStringLiteral("uv_index")))
        values << tr("UV %1").arg(metrics.value(QStringLiteral("uv_index")).toDouble(), 0, 'f', 2);
    if (metrics.contains(QStringLiteral("uv_voltage")))
        values << tr("UV %1 V").arg(metrics.value(QStringLiteral("uv_voltage")).toDouble(), 0, 'f', 3);
    if (metrics.contains(QStringLiteral("pressure_hpa")))
        values << tr("Áp suất %1 hPa").arg(
            metrics.value(QStringLiteral("pressure_hpa")).toDouble(), 0, 'f', 1);
    if (metrics.contains(QStringLiteral("temperature_c")))
        values << tr("Nhiệt độ %1 °C").arg(
            metrics.value(QStringLiteral("temperature_c")).toDouble(), 0, 'f', 1);
    if (metrics.contains(QStringLiteral("sound_vpp")))
        values << tr("Âm thanh %1 Vpp").arg(
            metrics.value(QStringLiteral("sound_vpp")).toDouble(), 0, 'f', 3);
    if (metrics.contains(QStringLiteral("flow_l_min")))
        values << tr("Lưu lượng %1 L/min").arg(
            metrics.value(QStringLiteral("flow_l_min")).toDouble(), 0, 'f', 2);
    if (metrics.contains(QStringLiteral("total_liters")))
        values << tr("Tổng %1 L").arg(
            metrics.value(QStringLiteral("total_liters")).toDouble(), 0, 'f', 2);
    if (metrics.contains(QStringLiteral("pump_on")))
        values << (metrics.value(QStringLiteral("pump_on")).toBool() ? tr("Bơm đang bật") : tr("Bơm đang tắt"));
    if (metrics.contains(QStringLiteral("current_a")))
        values << tr("Dòng %1 A").arg(
            metrics.value(QStringLiteral("current_a")).toDouble(), 0, 'f', 3);
    if (metrics.contains(QStringLiteral("voltage_v")))
        values << tr("Áp %1 V").arg(
            metrics.value(QStringLiteral("voltage_v")).toDouble(), 0, 'f', 1);
    return values.isEmpty() ? tr("Đang chờ dữ liệu cảm biến")
                            : values.join(QStringLiteral("  •  "));
}
