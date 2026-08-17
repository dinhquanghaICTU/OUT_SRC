#include "DeviceManagementPage.h"
#include "VirtualKeyboard.h"

#include <QButtonGroup>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDialog>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
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
    explicit RelayToggle(bool on, const QString &label = QObject::tr("Relay"), QWidget *parent = nullptr)
        : QPushButton(parent), m_on(on), m_label(label)
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
                                   : tr("%1 %2").arg(m_label, m_on ? tr("bật") : tr("tắt")));

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
    QString m_label;
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
    m_outerLayout = outer;
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    auto *mainPanel = new QWidget(this);
    mainPanel->setObjectName(QStringLiteral("deviceMainPanel"));
    auto *root = new QVBoxLayout(mainPanel);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(10);

    auto *header = new QHBoxLayout;
    auto *titles = new QVBoxLayout;
    auto *title = new QLabel(tr("Quản lý thiết bị & Lịch sử thêm"), this);
    title->setObjectName(QStringLiteral("devicePageTitle"));
    auto *subtitle = new QLabel(
        tr("Theo dõi chi tiết thiết bị, người đã thêm (kèm ngày giờ) và điều khiển online."), this);
    subtitle->setObjectName(QStringLiteral("devicePageSubtitle"));
    titles->addWidget(title);
    titles->addWidget(subtitle);
    m_liveLabel->setObjectName(QStringLiteral("liveBadge"));
    header->addLayout(titles);
    header->addStretch();
    header->addWidget(m_liveLabel, 0, Qt::AlignTop);
    root->addLayout(header);

    // --- View Mode Tabs ---
    auto *toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);

    m_logTabBtn = new QPushButton(tr("📋  Nhật ký & Quản lý thiết bị"), this);
    m_cardsTabBtn = new QPushButton(tr("⊞  Thẻ điều khiển & Ngưỡng"), this);
    m_logTabBtn->setObjectName(QStringLiteral("deviceViewTabButton"));
    m_cardsTabBtn->setObjectName(QStringLiteral("deviceViewTabButton"));
    m_logTabBtn->setCheckable(true);
    m_cardsTabBtn->setCheckable(true);
    m_logTabBtn->setChecked(true);
    m_logTabBtn->setCursor(Qt::PointingHandCursor);
    m_cardsTabBtn->setCursor(Qt::PointingHandCursor);

    auto *tabGroup = new QButtonGroup(this);
    tabGroup->addButton(m_logTabBtn, 0);
    tabGroup->addButton(m_cardsTabBtn, 1);
    tabGroup->setExclusive(true);

    toolbar->addWidget(m_logTabBtn);
    toolbar->addWidget(m_cardsTabBtn);
    toolbar->addStretch();
    root->addLayout(toolbar);

    m_viewStack = new QStackedWidget(this);

    // === VIEW 0: LOG & MANAGEMENT TABLE ===
    auto *logPage = new QWidget(m_viewStack);
    auto *logLayout = new QVBoxLayout(logPage);
    logLayout->setContentsMargins(0, 4, 0, 4);
    logLayout->setSpacing(8);

    auto *topBar = new QHBoxLayout;
    topBar->setSpacing(8);
    m_logSearchEdit = new QLineEdit(logPage);
    m_logSearchEdit->setObjectName(QStringLiteral("logSearchInput"));
    m_logSearchEdit->setPlaceholderText(tr("🔍 Tìm kiếm User thêm, Mã ID, Tên..."));
    m_logSearchEdit->setClearButtonEnabled(true);
    VirtualKeyboardDialog::attachToLineEdit(m_logSearchEdit, tr("Tìm kiếm thiết bị / log"));

    m_statTotalDevices = new QLabel(tr("Tổng: 0 thiết bị"), logPage);
    m_statOnlineDevices = new QLabel(tr("Online: 0"), logPage);
    m_statLinkedUsers = new QLabel(tr("Người dùng: 0"), logPage);
    m_statTotalDevices->setObjectName(QStringLiteral("logStatBadge"));
    m_statOnlineDevices->setObjectName(QStringLiteral("logStatBadgeOnline"));
    m_statLinkedUsers->setObjectName(QStringLiteral("logStatBadge"));

    topBar->addWidget(m_logSearchEdit, 1);
    topBar->addWidget(m_statTotalDevices);
    topBar->addWidget(m_statOnlineDevices);
    topBar->addWidget(m_statLinkedUsers);
    logLayout->addLayout(topBar);

    m_deviceLogTable = new QTableWidget(logPage);
    m_deviceLogTable->setObjectName(QStringLiteral("deviceLogTable"));
    m_deviceLogTable->setColumnCount(9);
    m_deviceLogTable->setHorizontalHeaderLabels({
        tr("STT"), tr("Tên thiết bị"), tr("Mã thiết bị (ID)"), tr("Loại thiết bị"),
        tr("Người thêm (User)"), tr("Thời gian thêm"), tr("Trạng thái"),
        tr("Lần cuối online"), tr("Hành động")
    });
    m_deviceLogTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_deviceLogTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_deviceLogTable->horizontalHeader()->setStretchLastSection(false);
    m_deviceLogTable->verticalHeader()->setVisible(false);
    m_deviceLogTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_deviceLogTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_deviceLogTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_deviceLogTable->setAlternatingRowColors(true);
    logLayout->addWidget(m_deviceLogTable, 1);

    m_logEmptyLabel = new QLabel(tr("Chưa có thiết bị nào trong danh sách."), logPage);
    m_logEmptyLabel->setObjectName(QStringLiteral("deviceEmptyState"));
    m_logEmptyLabel->hide();
    logLayout->addWidget(m_logEmptyLabel);

    m_viewStack->addWidget(logPage);

    // === VIEW 1: CARDS GRID ===
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *content = new QWidget(scroll);
    content->setObjectName(QStringLiteral("devicePageContent"));
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 2, 10);
    contentLayout->setSpacing(10);

    auto *ownedTitle = new QLabel(tr("Thiết bị của tôi"), content);
    ownedTitle->setObjectName(QStringLiteral("deviceSectionTitle"));
    contentLayout->addWidget(ownedTitle);
    m_ownedGrid->setHorizontalSpacing(14);
    m_ownedGrid->setVerticalSpacing(14);
    contentLayout->addLayout(m_ownedGrid);
    m_ownedEmpty->setObjectName(QStringLiteral("deviceEmptyState"));
    contentLayout->addWidget(m_ownedEmpty);

    auto *availableHeader = new QHBoxLayout;
    auto *availableTitles = new QVBoxLayout;
    auto *availableTitle = new QLabel(tr("Có thể thêm"), content);
    availableTitle->setObjectName(QStringLiteral("deviceSectionTitle"));
    auto *availableHint = new QLabel(
        tr("Chỉ hiển thị thiết bị online chưa thuộc tài khoản nào"), content);
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
    m_viewStack->addWidget(scroll);

    root->addWidget(m_viewStack, 1);

    connect(m_logTabBtn, &QPushButton::clicked, this, [this] {
        m_viewStack->setCurrentIndex(0);
    });
    connect(m_cardsTabBtn, &QPushButton::clicked, this, [this] {
        m_viewStack->setCurrentIndex(1);
    });
    connect(m_logSearchEdit, &QLineEdit::textChanged, this, [this] {
        rebuildLogTable();
    });
    connect(m_deviceLogTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        if (row >= 0 && row < m_ownedDevices.size()) {
            openDeviceDrawer(m_ownedDevices.at(row).toObject());
        }
    });

    m_drawer->setObjectName(QStringLiteral("deviceDrawer"));
    m_drawer->setFixedWidth(240);
    auto *drawerLayout = new QVBoxLayout(m_drawer);
    drawerLayout->setContentsMargins(14, 14, 14, 14);
    drawerLayout->setSpacing(8);
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
    applyResponsiveLayout();

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
        const int availableWidth = parentWidget() ? parentWidget()->width() - 24 : 430;
        dialog.setFixedWidth(qBound(280, qMin(430, availableWidth), 520));
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

void DeviceManagementPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    applyResponsiveLayout();
}

void DeviceManagementPage::applyResponsiveLayout()
{
    const bool compact = width() <= 800;
    const int columns = compact ? 1 : 2;
    if (m_compact == compact && m_gridColumns == columns)
        return;

    m_compact = compact;
    m_gridColumns = columns;
    // Luôn giữ drawer bên phải (LeftToRight), không đổi hướng theo width
    m_outerLayout->setDirection(QBoxLayout::LeftToRight);
    if (m_drawer->isVisible() && width() < 500)
        m_drawer->hide();
    rebuildOwnedGrid();
    rebuildAvailableGrid();
}

void DeviceManagementPage::setOwnedDevices(const QJsonArray &devices)
{
    m_ownedDevices = devices;
    rebuildOwnedGrid();
    rebuildLogTable();
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

void DeviceManagementPage::rebuildLogTable()
{
    if (!m_deviceLogTable)
        return;

    const QString filter = m_logSearchEdit ? m_logSearchEdit->text().trimmed().toLower() : QString();
    m_deviceLogTable->setRowCount(0);

    QSet<QString> uniqueUsers;
    int onlineCount = 0;
    int displayedRow = 0;

    for (int i = 0; i < m_ownedDevices.size(); ++i) {
        const QJsonObject dev = m_ownedDevices.at(i).toObject();
        const QString devId = dev.value(QStringLiteral("device_id")).toString();
        const QString name = dev.value(QStringLiteral("name")).toString();
        const QString type = dev.value(QStringLiteral("device_type")).toString();
        const QString addedBy = dev.value(QStringLiteral("added_by")).toString();
        const QString createdAt = dev.value(QStringLiteral("created_at")).toString();
        const bool isOnline = dev.value(QStringLiteral("online")).toBool();
        const QString lastSeen = dev.value(QStringLiteral("last_seen_at")).toString();

        if (isOnline) onlineCount++;
        if (!addedBy.isEmpty() && addedBy != QStringLiteral("Chưa gán"))
            uniqueUsers.insert(addedBy);

        // Filter
        if (!filter.isEmpty()) {
            const QString searchTarget = QStringLiteral("%1 %2 %3 %4 %5")
                .arg(devId, name, addedBy, deviceTypeName(type), createdAt).toLower();
            if (!searchTarget.contains(filter))
                continue;
        }

        m_deviceLogTable->insertRow(displayedRow);
        m_deviceLogTable->setRowHeight(displayedRow, 44);

        // Format created time
        QDateTime createdDt = QDateTime::fromString(createdAt, Qt::ISODateWithMs);
        if (!createdDt.isValid()) createdDt = QDateTime::fromString(createdAt, Qt::ISODate);
        const QString createdStr = createdDt.isValid()
            ? createdDt.toLocalTime().toString(QStringLiteral("dd/MM/yyyy HH:mm:ss"))
            : (createdAt.isEmpty() ? QStringLiteral("--") : createdAt);

        // Format last seen time
        QDateTime lastSeenDt = QDateTime::fromString(lastSeen, Qt::ISODateWithMs);
        if (!lastSeenDt.isValid()) lastSeenDt = QDateTime::fromString(lastSeen, Qt::ISODate);
        const QString lastSeenStr = lastSeenDt.isValid()
            ? lastSeenDt.toLocalTime().toString(QStringLiteral("dd/MM/yyyy HH:mm:ss"))
            : (lastSeen.isEmpty() ? QStringLiteral("--") : lastSeen);

        auto *sttItem = new QTableWidgetItem(QString::number(displayedRow + 1));
        sttItem->setTextAlignment(Qt::AlignCenter);

        auto *nameItem = new QTableWidgetItem(name.isEmpty() ? devId : name);
        nameItem->setFont(QFont(font().family(), 10, QFont::Bold));

        auto *idItem = new QTableWidgetItem(devId);
        idItem->setTextAlignment(Qt::AlignCenter);

        auto *typeItem = new QTableWidgetItem(deviceTypeName(type));

        auto *userItem = new QTableWidgetItem(addedBy.isEmpty() ? QStringLiteral("Chưa gán") : addedBy);
        userItem->setTextAlignment(Qt::AlignCenter);
        userItem->setForeground(QColor("#15945a"));

        auto *timeItem = new QTableWidgetItem(createdStr);
        timeItem->setTextAlignment(Qt::AlignCenter);

        auto *statusItem = new QTableWidgetItem(isOnline ? tr("●  Online") : tr("○  Offline"));
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setForeground(isOnline ? QColor("#15945a") : QColor("#8a9992"));

        auto *lastSeenItem = new QTableWidgetItem(lastSeenStr);
        lastSeenItem->setTextAlignment(Qt::AlignCenter);

        m_deviceLogTable->setItem(displayedRow, 0, sttItem);
        m_deviceLogTable->setItem(displayedRow, 1, nameItem);
        m_deviceLogTable->setItem(displayedRow, 2, idItem);
        m_deviceLogTable->setItem(displayedRow, 3, typeItem);
        m_deviceLogTable->setItem(displayedRow, 4, userItem);
        m_deviceLogTable->setItem(displayedRow, 5, timeItem);
        m_deviceLogTable->setItem(displayedRow, 6, statusItem);
        m_deviceLogTable->setItem(displayedRow, 7, lastSeenItem);

        // Action widget with Config and Delete buttons
        auto *actionWidget = new QWidget(m_deviceLogTable);
        auto *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 2, 4, 2);
        actionLayout->setSpacing(6);

        auto *cfgBtn = new QPushButton(tr("⚙ Cấu hình"), actionWidget);
        cfgBtn->setObjectName(QStringLiteral("tableActionConfigBtn"));
        cfgBtn->setCursor(Qt::PointingHandCursor);
        connect(cfgBtn, &QPushButton::clicked, this, [this, dev] {
            openDeviceDrawer(dev);
        });

        auto *delBtn = new QPushButton(tr("🗑 Gỡ"), actionWidget);
        delBtn->setObjectName(QStringLiteral("tableActionDeleteBtn"));
        delBtn->setCursor(Qt::PointingHandCursor);
        connect(delBtn, &QPushButton::clicked, this, [this, devId, name] {
            if (QMessageBox::question(this, tr("Xác nhận gỡ thiết bị"),
                    tr("Gỡ thiết bị '%1' (ID: %2) khỏi tài khoản?\nThiết bị sẽ trở lại danh sách có thể thêm.")
                        .arg(name.isEmpty() ? devId : name, devId)) == QMessageBox::Yes) {
                emit releaseDeviceRequested(devId);
            }
        });

        actionLayout->addWidget(cfgBtn);
        actionLayout->addWidget(delBtn);
        m_deviceLogTable->setCellWidget(displayedRow, 8, actionWidget);

        displayedRow++;
    }

    if (m_statTotalDevices)
        m_statTotalDevices->setText(tr("Tổng: %1 thiết bị").arg(m_ownedDevices.size()));
    if (m_statOnlineDevices)
        m_statOnlineDevices->setText(tr("Online: %1").arg(onlineCount));
    if (m_statLinkedUsers)
        m_statLinkedUsers->setText(tr("Người dùng: %1").arg(uniqueUsers.size()));

    if (m_logEmptyLabel) {
        m_logEmptyLabel->setVisible(displayedRow == 0);
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
    if (m_compact) {
        card->setMinimumHeight(136);
        card->setMaximumHeight(QWIDGETSIZE_MAX);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    } else {
        card->setFixedSize(200, 156);
    }
    card->setCursor(Qt::PointingHandCursor);
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 13, 14, 13);
    layout->setSpacing(6);

    const QString type = device.value(QStringLiteral("device_type")).toString();
    card->setProperty("deviceType", type);
    const bool online = device.value(QStringLiteral("online")).toBool();
    const QString deviceId = device.value(QStringLiteral("device_id")).toString();
    const QJsonObject metricsObject = device.value(QStringLiteral("metrics")).toObject();
    auto *top = new QHBoxLayout;
    top->setSpacing(12);
    auto *icon = new QLabel(deviceIcon(type), card);
    icon->setObjectName(QStringLiteral("deviceTypeIcon"));
    icon->setProperty("deviceType", type);
    icon->setAlignment(Qt::AlignCenter);
    auto *metricBlock = new QVBoxLayout;
    metricBlock->setContentsMargins(0, 0, 0, 0);
    metricBlock->setSpacing(1);
    QString primaryMetric = QStringLiteral("--");
    QString secondaryMetric;
    if (metricsObject.contains(QStringLiteral("flow_l_min")))
        primaryMetric = QStringLiteral("%1 L/min").arg(metricsObject.value(QStringLiteral("flow_l_min")).toDouble(), 0, 'f', 2);
    else if (metricsObject.contains(QStringLiteral("distance_cm")))
        primaryMetric = QStringLiteral("%1 cm").arg(metricsObject.value(QStringLiteral("distance_cm")).toDouble(), 0, 'f', 1);
    else if (metricsObject.contains(QStringLiteral("temperature_c")))
        primaryMetric = QStringLiteral("%1°C").arg(metricsObject.value(QStringLiteral("temperature_c")).toDouble(), 0, 'f', 1);
    else if (metricsObject.contains(QStringLiteral("uv_index")))
        primaryMetric = QStringLiteral("UV %1").arg(metricsObject.value(QStringLiteral("uv_index")).toDouble(), 0, 'f', 1);
    else if (metricsObject.contains(QStringLiteral("pressure_hpa")))
        primaryMetric = QStringLiteral("%1 hPa").arg(metricsObject.value(QStringLiteral("pressure_hpa")).toDouble(), 0, 'f', 0);
    if (metricsObject.contains(QStringLiteral("total_liters")))
        secondaryMetric = tr("Tổng %1 L").arg(metricsObject.value(QStringLiteral("total_liters")).toDouble(), 0, 'f', 2);
    else if (metricsObject.contains(QStringLiteral("pump_on")))
        secondaryMetric = metricsObject.value(QStringLiteral("pump_on")).toBool() ? tr("Bơm đang bật") : tr("Bơm đang tắt");
    else if (metricsObject.contains(QStringLiteral("sound_vpp")))
        secondaryMetric = QStringLiteral("%1 Vpp").arg(metricsObject.value(QStringLiteral("sound_vpp")).toDouble(), 0, 'f', 3);
    else if (metricsObject.contains(QStringLiteral("pressure_hpa")))
        secondaryMetric = QStringLiteral("%1 hPa").arg(metricsObject.value(QStringLiteral("pressure_hpa")).toDouble(), 0, 'f', 0);
    auto *primary = new QLabel(primaryMetric, card);
    primary->setObjectName(QStringLiteral("devicePrimaryMetric"));
    auto *secondary = new QLabel(secondaryMetric.isEmpty() ? deviceTypeName(type) : secondaryMetric, card);
    secondary->setObjectName(QStringLiteral("deviceSecondaryMetric"));
    metricBlock->addWidget(primary);
    metricBlock->addWidget(secondary);
    auto *status = new QLabel(online ? tr("●  Online") : tr("●  Offline"), card);
    status->setObjectName(online ? QStringLiteral("onlinePill") : QStringLiteral("offlinePill"));
    top->addWidget(icon);
    top->addLayout(metricBlock, 1);
    top->addSpacing(8);
    top->addWidget(status, 0, Qt::AlignTop | Qt::AlignRight);
    layout->addLayout(top);

    auto *name = new QLabel(device.value(QStringLiteral("name")).toString(), card);
    name->setObjectName(QStringLiteral("deviceCardName"));
    name->setWordWrap(true);
    name->setFixedHeight(42);
    name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    const QString addedBy = device.value(QStringLiteral("added_by")).toString();
    auto *id = new QLabel(addedBy.isEmpty()
        ? tr("ID  %1").arg(deviceId)
        : tr("ID  %1 · Thêm bởi: %2").arg(deviceId, addedBy), card);
    id->setObjectName(QStringLiteral("deviceCardId"));
    id->setFixedHeight(17);
    id->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(name);
    layout->addWidget(id);
    layout->addStretch();
    auto *actionSlot = new QWidget(card);
    actionSlot->setFixedHeight(34);
    auto *actionLayout = new QVBoxLayout(actionSlot);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setSpacing(0);
    if (hasRelay) {
        const QJsonObject stateObject = device.value(QStringLiteral("state")).toObject();
        const bool relayOn = stateObject.contains(QStringLiteral("relay"))
                                 ? stateObject.value(QStringLiteral("relay")).toBool(false)
                                 : metricsObject.value(QStringLiteral("pump_on")).toBool(false);
        auto *relayButton = new RelayToggle(relayOn,
                                            (type == QStringLiteral("pump_distance") || type == QStringLiteral("water_flow_pump")) ? tr("Bơm") : tr("Relay"),
                                            card);
        relayButton->setEnabled(online);
        actionLayout->addWidget(relayButton);
        connect(relayButton, &QPushButton::clicked, this,
                [this, deviceId, relayOn, relayButton] {
                    relayButton->setPending();
                    emit relayControlRequested(deviceId, !relayOn);
                });
    } else if (metricsObject.contains(QStringLiteral("ir_detected"))) {
        const bool irDetected = metricsObject.value(QStringLiteral("ir_detected")).toInt() != 0;
        auto *irStatus = new QLabel(
            irDetected ? tr("●  Có vật")
                       : tr("●  Không có vật"), card);
        irStatus->setObjectName(irDetected ? QStringLiteral("irDetectedStatus")
                                           : QStringLiteral("irClearStatus"));
        irStatus->setAlignment(Qt::AlignCenter);
        actionLayout->addWidget(irStatus);
    }
    layout->addWidget(actionSlot);
    card->clicked = [this, device] { openDeviceDrawer(device); };
    return card;
}

QWidget *DeviceManagementPage::createAvailableCard(const QJsonObject &device)
{
    auto *card = new QFrame(this);
    card->setObjectName(QStringLiteral("availableDeviceCard"));
    if (m_compact) {
        card->setMinimumHeight(164);
        card->setMaximumHeight(QWIDGETSIZE_MAX);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    } else {
        card->setFixedSize(210, 178);
    }
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 13, 14, 13);
    layout->setSpacing(7);
    const QString deviceId = device.value(QStringLiteral("device_id")).toString();
    const QString type = device.value(QStringLiteral("device_type")).toString();
    card->setProperty("deviceType", type);

    auto *top = new QHBoxLayout;
    auto *icon = new QLabel(deviceIcon(type), card);
    icon->setObjectName(QStringLiteral("availableDeviceIcon"));
    icon->setProperty("deviceType", type);
    icon->setAlignment(Qt::AlignCenter);
    auto *status = new QLabel(tr("●  Online"), card);
    status->setObjectName(QStringLiteral("onlinePill"));
    top->addWidget(icon);
    top->addStretch();
    top->addWidget(status);
    layout->addLayout(top);

    auto *id = new QLabel(deviceId, card);
    id->setObjectName(QStringLiteral("availableDeviceId"));
    id->setWordWrap(true);
    auto *typeName = new QLabel(deviceTypeName(type), card);
    typeName->setObjectName(QStringLiteral("deviceCardType"));
    typeName->setWordWrap(true);
    auto *hint = new QLabel(tr("Thiết bị đang chờ liên kết"), card);
    hint->setObjectName(QStringLiteral("availableDeviceHint"));
    auto *button = new QPushButton(tr("+  Thêm thiết bị"), card);
    button->setObjectName(QStringLiteral("claimDeviceButton"));
    layout->addWidget(id);
    layout->addWidget(typeName);
    layout->addWidget(hint);
    layout->addStretch();
    layout->addWidget(button);

    connect(button, &QPushButton::clicked, this, [this, deviceId, type] {
        QDialog dialog(this);
        dialog.setObjectName(QStringLiteral("claimDeviceDialog"));
        dialog.setWindowTitle(tr("Đặt tên thiết bị"));
        dialog.setModal(true);
        const int availableWidth = parentWidget() ? parentWidget()->width() - 24 : 420;
        const int availableHeight = parentWidget() ? parentWidget()->height() - 16 : 480;
        dialog.setFixedWidth(qBound(280, qMin(420, availableWidth), 520));
        dialog.setMaximumHeight(qMax(300, availableHeight));

        auto *root = new QVBoxLayout(&dialog);
        root->setContentsMargins(14, 12, 14, 12);
        root->setSpacing(10);
        auto *scroll = new QScrollArea(&dialog);
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        auto *body = new QWidget(scroll);
        auto *bodyLayout = new QVBoxLayout(body);
        bodyLayout->setContentsMargins(10, 10, 10, 10);
        bodyLayout->setSpacing(12);
        scroll->setWidget(body);
        root->addWidget(scroll, 1);

        auto *head = new QHBoxLayout;
        auto *dialogIcon = new QLabel(deviceIcon(type), body);
        dialogIcon->setObjectName(QStringLiteral("claimDeviceDialogIcon"));
        dialogIcon->setProperty("deviceType", type);
        dialogIcon->setAlignment(Qt::AlignCenter);
        auto *titleBlock = new QVBoxLayout;
        auto *title = new QLabel(tr("Thêm thiết bị mới"), body);
        title->setObjectName(QStringLiteral("claimDeviceDialogTitle"));
        auto *subtitle = new QLabel(tr("Đặt tên dễ nhớ để quản lý trên app."), body);
        subtitle->setObjectName(QStringLiteral("claimDeviceDialogHint"));
        subtitle->setWordWrap(true);
        titleBlock->addWidget(title);
        titleBlock->addWidget(subtitle);
        head->addWidget(dialogIcon);
        head->addLayout(titleBlock, 1);
        bodyLayout->addLayout(head);

        auto *info = new QLabel(tr("ID: %1  •  %2").arg(deviceId, deviceTypeName(type)), body);
        info->setObjectName(QStringLiteral("claimDeviceInfo"));
        info->setWordWrap(true);
        bodyLayout->addWidget(info);

        auto *name = new QLineEdit(deviceTypeName(type), body);
        name->setObjectName(QStringLiteral("claimDeviceNameInput"));
        name->setPlaceholderText(tr("VD: Phòng khách, Khu A..."));
        name->selectAll();
        bodyLayout->addWidget(name);
        VirtualKeyboardDialog::attachToLineEdit(name, tr("Nhập tên thiết bị"));

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
        connect(name, &QLineEdit::returnPressed, &dialog, &QDialog::accept);
        name->setFocus();
        if (dialog.exec() != QDialog::Accepted)
            return;
        const QString displayName = name->text().trimmed();
        emit claimDeviceRequested(deviceId, displayName.isEmpty() ? deviceId : displayName);
    });
    return card;
}

void DeviceManagementPage::setCurrentUser(const QString &username, bool isAdmin)
{
    m_currentUsername = username.trimmed();
    m_isAdmin = isAdmin;
    rebuildOwnedGrid();
    rebuildLogTable();
}

void DeviceManagementPage::rebuildOwnedGrid()
{
    clearGrid(m_ownedGrid);
    QJsonArray userDevices;
    for (const QJsonValue &val : m_ownedDevices) {
        const QJsonObject dev = val.toObject();
        const QString addedBy = dev.value(QStringLiteral("added_by")).toString();
        if (m_currentUsername.isEmpty() || addedBy.isEmpty() || addedBy.compare(m_currentUsername, Qt::CaseInsensitive) == 0) {
            userDevices.append(dev);
        }
    }
    m_ownedEmpty->setVisible(userDevices.isEmpty());
    m_ownedEmpty->setText(m_currentUsername.isEmpty()
        ? tr("Bạn chưa thêm thiết bị nào.")
        : tr("Tài khoản '%1' chưa thêm thiết bị nào vào danh sách điều khiển.").arg(m_currentUsername));
    const int columns = m_gridColumns;
    for (int i = 0; i < userDevices.size(); ++i)
        m_ownedGrid->addWidget(createOwnedCard(userDevices.at(i).toObject()),
                               i / columns, i % columns,
                               m_compact ? Qt::AlignTop : Qt::AlignLeft | Qt::AlignTop);
    m_ownedGrid->setColumnStretch(0, m_compact ? 1 : 0);
    m_ownedGrid->setColumnStretch(columns, m_compact ? 0 : 1);
}

void DeviceManagementPage::rebuildAvailableGrid()
{
    clearGrid(m_availableGrid);
    m_availableEmpty->setVisible(m_availableDevices.isEmpty());
    m_availableEmpty->setText(tr("Không có thiết bị online nào đang chờ thêm."));
    const int columns = m_gridColumns;
    for (int i = 0; i < m_availableDevices.size(); ++i)
        m_availableGrid->addWidget(createAvailableCard(m_availableDevices.at(i).toObject()),
                                   i / columns, i % columns,
                                   m_compact ? Qt::AlignTop : Qt::AlignLeft | Qt::AlignTop);
    m_availableGrid->setColumnStretch(0, m_compact ? 1 : 0);
    m_availableGrid->setColumnStretch(columns, m_compact ? 0 : 1);
}

void DeviceManagementPage::openDeviceDrawer(const QJsonObject &device)
{
    m_selectedDevice = device;
    const QString addedBy = device.value(QStringLiteral("added_by")).toString();
    const bool isOwner = (m_currentUsername.isEmpty() || addedBy.isEmpty() || addedBy.compare(m_currentUsername, Qt::CaseInsensitive) == 0);

    m_releaseDevice->setEnabled(isOwner || m_isAdmin);
    m_releaseDevice->setText(tr("Xóa thiết bị khỏi tài khoản"));
    const QString type = device.value(QStringLiteral("device_type")).toString();
    m_drawerIcon->setText(deviceIcon(type));
    m_drawerName->setText(device.value(QStringLiteral("name")).toString());
    const QString createdAt = device.value(QStringLiteral("created_at")).toString();
    QDateTime createdTime = QDateTime::fromString(createdAt, Qt::ISODateWithMs);
    if (!createdTime.isValid()) createdTime = QDateTime::fromString(createdAt, Qt::ISODate);
    const QString createdStr = createdTime.isValid() ? createdTime.toLocalTime().toString(QStringLiteral("dd/MM/yyyy HH:mm")) : createdAt;
    m_drawerId->setText(addedBy.isEmpty()
        ? tr("Device ID: %1").arg(device.value(QStringLiteral("device_id")).toString())
        : tr("ID: %1 | Thêm bởi: %2 (%3)").arg(device.value(QStringLiteral("device_id")).toString(), addedBy, createdStr));
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
    const QString addedBy = device.value(QStringLiteral("added_by")).toString();
    const bool isOwner = (m_currentUsername.isEmpty() || addedBy.isEmpty() || addedBy.compare(m_currentUsername, Qt::CaseInsensitive) == 0);

    auto addThreshold = [this, &savedThresholds, isOwner](const QString &key, const QString &label,
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
        input->setEnabled(isOwner);
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
    } else if (type == QStringLiteral("water_flow_pump") || type == QStringLiteral("pump_distance")) {
        addThreshold(QStringLiteral("flow_l_min.min"), tr("Lưu lượng tối thiểu"), 0.20, 0, 60, tr(" L/min"));
        addThreshold(QStringLiteral("flow_l_min.max"), tr("Lưu lượng tối đa"), 20.00, 0, 60, tr(" L/min"));
        addThreshold(QStringLiteral("total_liters.max"), tr("Tổng nước cảnh báo"), 100.00, 0, 100000, tr(" L"));
    }

    m_samplingInterval = new QSpinBox(m_drawer);
    m_samplingInterval->setRange(1, 3600);
    m_samplingInterval->setSuffix(tr(" giây"));
    m_samplingInterval->setValue(saved.value(QStringLiteral("sampling_interval_ms"))
                                     .toInt(2000) / 1000);
    m_samplingInterval->setEnabled(isOwner);
    const bool hasThresholds = !m_thresholdInputs.isEmpty();
    m_thresholdForm->addRow(tr("Chu kỳ gửi"), m_samplingInterval);
    m_thresholdForm->setRowVisible(m_samplingInterval, hasThresholds);
    m_saveThresholds->setVisible(hasThresholds);
    m_saveThresholds->setEnabled(isOwner);
    m_thresholdTitle->setVisible(hasThresholds);

    if (!isOwner) {
        m_thresholdTitle->setText(tr("Ngưỡng cảnh báo (Sở hữu: %1 · Chỉ xem)").arg(addedBy));
        m_saveThresholds->setText(tr("Chỉ '%1' mới được đổi ngưỡng").arg(addedBy));
    } else {
        m_thresholdTitle->setText(tr("Ngưỡng cảnh báo"));
        m_saveThresholds->setText(tr("Lưu & gửi xuống thiết bị"));
    }
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
    if (type == QStringLiteral("power_monitor") || type == QStringLiteral("electric_power")) return QStringLiteral("⚡");
    if (type == QStringLiteral("uv_pressure")) return QStringLiteral("☀");
    if (type == QStringLiteral("temperature_sound")) return QStringLiteral("♫");
    if (type == QStringLiteral("weather_pressure")) return QStringLiteral("☁");
    if (type == QStringLiteral("pump_distance")) return QStringLiteral("💧");
    if (type == QStringLiteral("water_flow_pump")) return QStringLiteral("🚰");
    return QStringLiteral("⚡");
}

QString DeviceManagementPage::deviceTypeName(const QString &type)
{
    if (type == QStringLiteral("power_monitor") || type == QStringLiteral("electric_power")) return tr("Đo AC RMS & Công suất tải");
    if (type == QStringLiteral("uv_pressure")) return tr("Cảm biến UV & áp suất");
    if (type == QStringLiteral("temperature_sound")) return tr("Nhiệt độ & âm thanh");
    if (type == QStringLiteral("weather_pressure")) return tr("Cảm biến môi trường");
    if (type == QStringLiteral("pump_distance")) return tr("Bơm nước & khoảng cách");
    if (type == QStringLiteral("water_flow_pump")) return tr("Bơm & lưu lượng nước");
    return tr("Giám sát điện năng");
}

QString DeviceManagementPage::metricsSummary(const QJsonObject &metrics)
{
    QStringList values;
    if (metrics.contains(QStringLiteral("voltage_v")))
        values << tr("Điện áp %1 V").arg(
            metrics.value(QStringLiteral("voltage_v")).toDouble(), 0, 'f', 1);
    if (metrics.contains(QStringLiteral("current_a")))
        values << tr("Dòng tải %1 A").arg(
            metrics.value(QStringLiteral("current_a")).toDouble(), 0, 'f', 2);
    if (metrics.contains(QStringLiteral("power_w")))
        values << tr("Công suất %1 W").arg(
            metrics.value(QStringLiteral("power_w")).toDouble(), 0, 'f', 1);
    if (metrics.contains(QStringLiteral("frequency_hz")))
        values << tr("Tần số %1 Hz").arg(
            metrics.value(QStringLiteral("frequency_hz")).toDouble(), 0, 'f', 2);
    if (metrics.contains(QStringLiteral("uv_index")))
        values << tr("UV %1").arg(metrics.value(QStringLiteral("uv_index")).toDouble(), 0, 'f', 2);
    if (metrics.contains(QStringLiteral("pressure_hpa")))
        values << tr("Áp suất %1 hPa").arg(
            metrics.value(QStringLiteral("pressure_hpa")).toDouble(), 0, 'f', 1);
    if (metrics.contains(QStringLiteral("temperature_c")))
        values << tr("Nhiệt độ %1 °C").arg(
            metrics.value(QStringLiteral("temperature_c")).toDouble(), 0, 'f', 1);
    if (metrics.contains(QStringLiteral("distance_cm")))
        values << tr("Khoảng cách %1 cm").arg(
            metrics.value(QStringLiteral("distance_cm")).toDouble(), 0, 'f', 1);
    return values.isEmpty() ? tr("Đang chờ dữ liệu cảm biến")
                            : values.join(QStringLiteral("  •  "));
}
