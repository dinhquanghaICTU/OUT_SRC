#include "DeviceManagementPage.h"

#include <QButtonGroup>
#include <QDateTime>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QStringList>

#include "VirtualKeyboard.h"

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
        setFixedHeight(30);
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
        painter.setBrush(isDown() ? QColor("#0d2847") : QColor("#0e1f3d"));
        painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 6, 6);
        const QColor textColor = isEnabled() ? QColor("#ecf2ff") : QColor("#64748b");
        painter.setPen(textColor);
        QFont textFont = font();
        textFont.setPointSize(8);
        textFont.setBold(true);
        painter.setFont(textFont);
        painter.drawText(QRect(8, 0, width() - 48, height()),
                         Qt::AlignVCenter | Qt::AlignLeft,
                         m_pending ? tr("Đang gửi…")
                                   : (m_on ? tr("Quạt BẬT") : tr("Quạt TẮT")));

        const QRectF track(width() - 42, 6, 34, 18);
        painter.setPen(Qt::NoPen);
        painter.setBrush(!isEnabled() ? QColor("#334155")
                         : m_on ? QColor("#0284c7") : QColor("#475569"));
        painter.drawRoundedRect(track, 9, 9);
        const qreal knobX = m_on ? track.right() - 15 : track.left() + 2;
        painter.setBrush(Qt::white);
        painter.drawEllipse(QRectF(knobX, track.top() + 2, 14, 14));
    }

private:
    bool m_on;
    bool m_pending = false;
};

static bool confirmDeleteDevice(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setIcon(QMessageBox::Warning);
    auto *yesBtn = msgBox.addButton(QObject::tr("Xác nhận gỡ/xóa"), QMessageBox::YesRole);
    auto *noBtn = msgBox.addButton(QObject::tr("Hủy bỏ"), QMessageBox::NoRole);
    msgBox.setDefaultButton(noBtn);
    msgBox.setStyleSheet(QStringLiteral(
        "QMessageBox { background-color: #0b152d; border: 1.5px solid #1c2b54; border-radius: 8px; } "
        "QLabel { color: #f1f5f9; font-size: 12px; font-weight: 700; background: transparent; } "
        "QPushButton { min-width: 100px; min-height: 30px; border-radius: 6px; font-size: 11px; font-weight: 800; padding: 6px 14px; } "
    ));
    yesBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #dc2626; color: #ffffff; border: 1px solid #ef4444; border-radius: 6px; padding: 6px 14px; font-weight: 800; } "
        "QPushButton:hover { background-color: #b91c1c; } "
    ));
    noBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #1e293b; color: #cbd5e1; border: 1px solid #334155; border-radius: 6px; padding: 6px 14px; font-weight: 800; } "
        "QPushButton:hover { background-color: #334155; color: #ffffff; } "
    ));

    msgBox.exec();
    return msgBox.clickedButton() == yesBtn;
}
} // namespace

DeviceManagementPage::DeviceManagementPage(QWidget *parent)
    : QWidget(parent),
      m_ownedGrid(new QGridLayout),
      m_availableGrid(new QGridLayout),
      m_ownedEmpty(new QLabel(tr("Bạn chưa thêm trạm làm mát nào."), this)),
      m_availableEmpty(new QLabel(tr("Đang tìm thiết bị online trong mạng..."), this)),
      m_liveLabel(new QLabel(tr("● Realtime"), this)),
      m_refreshTimer(new QTimer(this))
{
    setObjectName(QStringLiteral("DeviceManagementPage"));
    setAttribute(Qt::WA_StyledBackground, true);

    setStyleSheet(
        "QWidget#DeviceManagementPage { background-color: #070d1e; color: #ecf2ff; font-family: sans-serif; } "
        "QLabel#devicePageTitle { color: #38bdf8; font-size: 14px; font-weight: 900; } "
        "QLabel#devicePageSubtitle { color: #94a3b8; font-size: 10px; } "
        "QLabel#liveBadge { color: #10b981; font-size: 9px; font-weight: 900; background: rgba(16, 185, 129, 0.15); border-radius: 4px; padding: 2px 6px; } "
        "QPushButton#deviceViewTabButton { background: #0e1938; color: #94a3b8; border: 1px solid #223565; border-radius: 6px; padding: 4px 10px; font-size: 11px; font-weight: 800; } "
        "QPushButton#deviceViewTabButton:checked { background: #0284c7; color: #ffffff; border-color: #38bdf8; font-weight: 900; } "
        "QPushButton#deviceViewTabButton:hover { background: #1e293b; color: #ffffff; } "
        "QLineEdit#logSearchInput { background-color: #0f1c3f; color: #ffffff; border: 1px solid #233870; border-radius: 6px; padding: 3px 8px; font-size: 11px; } "
        "QLabel#logStatBadge { background: #0e1938; color: #94a3b8; border: 1px solid #1e293b; border-radius: 4px; padding: 3px 6px; font-size: 10px; font-weight: 700; } "
        "QLabel#logStatBadgeOnline { background: rgba(16, 185, 129, 0.15); color: #10b981; border: 1px solid #059669; border-radius: 4px; padding: 3px 6px; font-size: 10px; font-weight: 900; } "
        "QTableWidget#deviceLogTable { background-color: #0c1630; color: #ffffff; gridline-color: #1c2b54; border: 1px solid #1c2b54; border-radius: 8px; font-size: 10px; } "
        "QTableWidget#deviceLogTable QHeaderView::section { background-color: #111d3d; color: #94a3b8; font-weight: 800; font-size: 10px; padding: 4px; border: none; } "
        "QTableWidget#deviceLogTable::item { padding: 2px 4px; } "
        "QTableWidget#deviceLogTable::item:selected { background-color: #1e3a8a; color: #ffffff; } "
        "QPushButton#tableActionConfigBtn { background: #1e3a8a; color: #38bdf8; border: 1px solid #2563eb; border-radius: 4px; font-size: 9px; font-weight: 800; padding: 2px 6px; } "
        "QPushButton#tableActionDeleteBtn { background: #7f1d1d; color: #fca5a5; border: 1px solid #991b1b; border-radius: 4px; font-size: 9px; font-weight: 800; padding: 2px 6px; } "
        "QFrame#ownedDeviceCard, QFrame#availableDeviceCard { background-color: #0e1938; border: 1px solid #1c2b54; border-radius: 10px; } "
        "QFrame#ownedDeviceCard:hover, QFrame#availableDeviceCard:hover { border-color: #38bdf8; background-color: #122149; } "
        "QLabel#deviceSectionTitle { color: #38bdf8; font-size: 13px; font-weight: 800; } "
        "QLabel#deviceSectionHint { color: #94a3b8; font-size: 10px; } "
        "QLabel#deviceCardName { color: #ffffff; font-size: 11px; font-weight: 800; } "
        "QLabel#deviceCardId { color: #94a3b8; font-size: 9px; } "
        "QLabel#devicePrimaryMetric { color: #38bdf8; font-size: 13px; font-weight: 900; } "
        "QLabel#deviceSecondaryMetric { color: #94a3b8; font-size: 9px; } "
        "QLabel#onlinePill { background: rgba(16, 185, 129, 0.15); color: #10b981; border: 1px solid #059669; border-radius: 4px; padding: 2px 6px; font-size: 9px; font-weight: 900; } "
        "QLabel#offlinePill { background: rgba(239, 68, 68, 0.15); color: #ef4444; border: 1px solid #dc2626; border-radius: 4px; padding: 2px 6px; font-size: 9px; font-weight: 900; } "
        "QPushButton#claimDeviceButton { background-color: #10b981; color: #ffffff; border: none; border-radius: 6px; font-size: 10px; font-weight: 800; padding: 4px 8px; } "
        "QPushButton#refreshDevicesButton { background-color: #0e1938; color: #38bdf8; border: 1px solid #223565; border-radius: 6px; font-size: 10px; font-weight: 800; padding: 3px 8px; } "
        "QPushButton#cardConfigButton { background: #111d3d; color: #38bdf8; border: 1px solid #233870; border-radius: 6px; font-size: 9px; font-weight: 800; padding: 4px 8px; } "
        "QPushButton#cardConfigButton:hover { background: #172554; border-color: #38bdf8; }"
    );

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(10);

    // --- Header ---
    auto *header = new QHBoxLayout;
    auto *titles = new QVBoxLayout;
    auto *title = new QLabel(tr("Hệ Thống Trạm Làm Mát Tự Động"), this);
    title->setObjectName(QStringLiteral("devicePageTitle"));
    auto *subtitle = new QLabel(
        tr("Quản lý các trạm cảm biến nhiệt độ LM35 & điều khiển quạt tản nhiệt ESP32"), this);
    subtitle->setObjectName(QStringLiteral("devicePageSubtitle"));
    titles->addWidget(title);
    titles->addWidget(subtitle);
    m_liveLabel->setObjectName(QStringLiteral("liveBadge"));
    header->addLayout(titles);
    header->addStretch();
    header->addWidget(m_liveLabel, 0, Qt::AlignTop);
    root->addLayout(header);

    // --- View Mode Toolbar ---
    auto *toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);

    m_cardsTabBtn = new QPushButton(tr("Thẻ điều khiển"), this);
    m_logTabBtn = new QPushButton(tr("Danh sách thiết bị"), this);
    m_cardsTabBtn->setObjectName(QStringLiteral("deviceViewTabButton"));
    m_logTabBtn->setObjectName(QStringLiteral("deviceViewTabButton"));
    m_cardsTabBtn->setCheckable(true);
    m_logTabBtn->setCheckable(true);
    m_cardsTabBtn->setChecked(true);
    m_cardsTabBtn->setCursor(Qt::PointingHandCursor);
    m_logTabBtn->setCursor(Qt::PointingHandCursor);

    auto *tabGroup = new QButtonGroup(this);
    tabGroup->addButton(m_cardsTabBtn, 0);
    tabGroup->addButton(m_logTabBtn, 1);
    tabGroup->setExclusive(true);

    toolbar->addWidget(m_cardsTabBtn);
    toolbar->addWidget(m_logTabBtn);
    toolbar->addStretch();
    root->addLayout(toolbar);

    m_viewStack = new QStackedWidget(this);

    // === VIEW 0: CARDS GRID VIEW ===
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(QStringLiteral("QScrollArea { background-color: #070d1e; border: none; }"));
    auto *content = new QWidget(scroll);
    content->setObjectName(QStringLiteral("devicePageContent"));
    content->setAttribute(Qt::WA_StyledBackground, true);
    content->setStyleSheet(QStringLiteral("QWidget#devicePageContent { background-color: #070d1e; }"));
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 2, 10);
    contentLayout->setSpacing(10);
    contentLayout->setSpacing(10);

    auto *ownedTitle = new QLabel(tr("Trạm làm mát của tôi"), content);
    ownedTitle->setObjectName(QStringLiteral("deviceSectionTitle"));
    contentLayout->addWidget(ownedTitle);
    m_ownedGrid->setHorizontalSpacing(12);
    m_ownedGrid->setVerticalSpacing(12);
    contentLayout->addLayout(m_ownedGrid);
    m_ownedEmpty->setObjectName(QStringLiteral("deviceEmptyState"));
    contentLayout->addWidget(m_ownedEmpty);

    auto *availableHeader = new QHBoxLayout;
    auto *availableTitles = new QVBoxLayout;
    auto *availableTitle = new QLabel(tr("Thiết bị ESP32 trong mạng có thể thêm"), content);
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
    m_availableGrid->setHorizontalSpacing(12);
    m_availableGrid->setVerticalSpacing(12);
    contentLayout->addLayout(m_availableGrid);
    m_availableEmpty->setObjectName(QStringLiteral("deviceEmptyState"));
    contentLayout->addWidget(m_availableEmpty);
    contentLayout->addStretch();

    scroll->setWidget(content);
    m_viewStack->addWidget(scroll);

    // === VIEW 1: TABLE LOG VIEW ===
    auto *logPage = new QWidget(m_viewStack);
    auto *logLayout = new QVBoxLayout(logPage);
    logLayout->setContentsMargins(0, 4, 0, 4);
    logLayout->setSpacing(6);

    auto *topBar = new QHBoxLayout;
    topBar->setSpacing(6);
    m_logSearchEdit = new QLineEdit(logPage);
    m_logSearchEdit->setObjectName(QStringLiteral("logSearchInput"));
    m_logSearchEdit->setPlaceholderText(tr("Tìm kiếm theo tên / ID / User..."));
    m_logSearchEdit->setClearButtonEnabled(true);
    VirtualKeyboardDialog::attachToLineEdit(m_logSearchEdit, tr("Tìm kiếm thiết bị"));

    m_statTotalDevices = new QLabel(tr("Tổng: 0"), logPage);
    m_statOnlineDevices = new QLabel(tr("Online: 0"), logPage);
    m_statTotalDevices->setObjectName(QStringLiteral("logStatBadge"));
    m_statOnlineDevices->setObjectName(QStringLiteral("logStatBadgeOnline"));

    topBar->addWidget(m_logSearchEdit, 1);
    topBar->addWidget(m_statTotalDevices);
    topBar->addWidget(m_statOnlineDevices);
    logLayout->addLayout(topBar);

    m_deviceLogTable = new QTableWidget(logPage);
    m_deviceLogTable->setObjectName(QStringLiteral("deviceLogTable"));
    m_deviceLogTable->setColumnCount(6);
    m_deviceLogTable->setHorizontalHeaderLabels({
        tr("STT"), tr("Tên trạm"), tr("Mã ID"), tr("Cảm biến / Chỉ số"),
        tr("Trạng thái"), tr("Thao tác")
    });
    m_deviceLogTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_deviceLogTable->setColumnWidth(0, 36);
    m_deviceLogTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_deviceLogTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    m_deviceLogTable->setColumnWidth(2, 110);
    m_deviceLogTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
    m_deviceLogTable->setColumnWidth(3, 170);
    m_deviceLogTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);
    m_deviceLogTable->setColumnWidth(4, 90);
    m_deviceLogTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    m_deviceLogTable->setColumnWidth(5, 140);
    m_deviceLogTable->verticalHeader()->setVisible(false);
    m_deviceLogTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_deviceLogTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_deviceLogTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_deviceLogTable->setAlternatingRowColors(false);
    logLayout->addWidget(m_deviceLogTable, 1);

    m_logEmptyLabel = new QLabel(tr("Chưa có thiết bị nào trong danh sách."), logPage);
    m_logEmptyLabel->setObjectName(QStringLiteral("deviceEmptyState"));
    m_logEmptyLabel->hide();
    logLayout->addWidget(m_logEmptyLabel);

    m_viewStack->addWidget(logPage);

    root->addWidget(m_viewStack, 1);

    connect(m_cardsTabBtn, &QPushButton::clicked, this, [this] {
        m_viewStack->setCurrentIndex(0);
    });
    connect(m_logTabBtn, &QPushButton::clicked, this, [this] {
        m_viewStack->setCurrentIndex(1);
    });
    connect(m_logSearchEdit, &QLineEdit::textChanged, this, [this] {
        rebuildLogTable();
    });
    connect(m_deviceLogTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        if (row >= 0 && row < m_deviceLogTable->rowCount()) {
            const auto *idItem = m_deviceLogTable->item(row, 2);
            if (idItem) {
                const QString devId = idItem->text().trimmed();
                for (const QJsonValue &v : m_ownedDevices) {
                    const QJsonObject d = v.toObject();
                    if (d.value(QStringLiteral("device_id")).toString().compare(devId, Qt::CaseInsensitive) == 0) {
                        openDeviceDrawer(d);
                        break;
                    }
                }
            }
        }
    });

    m_refreshTimer->setInterval(5000);
    connect(m_refreshTimer, &QTimer::timeout, this, &DeviceManagementPage::refreshRequested);
    connect(refreshButton, &QPushButton::clicked, this, &DeviceManagementPage::refreshRequested);

    applyResponsiveLayout();
}

void DeviceManagementPage::setCurrentUser(const QString &username, bool isAdmin)
{
    m_currentUsername = username;
    m_isAdmin = isAdmin;
}

void DeviceManagementPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    applyResponsiveLayout();
}

void DeviceManagementPage::applyResponsiveLayout()
{
    const bool compact = width() <= 500;
    const int columns = compact ? 1 : 2;
    if (m_compact == compact && m_gridColumns == columns)
        return;

    m_compact = compact;
    m_gridColumns = columns;
    rebuildOwnedGrid();
    rebuildAvailableGrid();
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
    m_liveLabel->setText(tr("●  Realtime %1")
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
    Q_UNUSED(deviceId);
    Q_UNUSED(mqttPublished);
}

QWidget *DeviceManagementPage::createOwnedCard(const QJsonObject &device)
{
    auto *card = new ClickableFrame(this);
    card->setObjectName(QStringLiteral("ownedDeviceCard"));
    const QJsonArray capabilities = device.value(QStringLiteral("capabilities")).toArray();
    bool hasRelay = false;
    for (const QJsonValue &capability : capabilities)
        hasRelay = hasRelay || capability.toString() == QStringLiteral("relay");

    card->setMinimumHeight(125);
    card->setMaximumHeight(155);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    card->setCursor(Qt::PointingHandCursor);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(6);

    const QString type = device.value(QStringLiteral("device_type")).toString();
    card->setProperty("deviceType", type);
    const bool online = device.value(QStringLiteral("online")).toBool();
    const QString deviceId = device.value(QStringLiteral("device_id")).toString();
    const QJsonObject metricsObject = device.value(QStringLiteral("metrics")).toObject();

    // Top row: Icon + Primary Metric + Online Pill
    auto *top = new QHBoxLayout;
    top->setSpacing(8);

    auto *icon = new QLabel(deviceIcon(type), card);
    icon->setStyleSheet("font-size: 24px; color: #38bdf8; background: #111d3d; border: 1px solid #233870; border-radius: 8px; padding: 4px 8px;");
    top->addWidget(icon);

    auto *metricBlock = new QVBoxLayout;
    metricBlock->setContentsMargins(0, 0, 0, 0);
    metricBlock->setSpacing(1);

    QString primaryMetric = QStringLiteral("--");
    if (metricsObject.contains(QStringLiteral("temperature_c"))) {
        primaryMetric = QStringLiteral("%1 °C").arg(metricsObject.value(QStringLiteral("temperature_c")).toDouble(), 0, 'f', 1);
    }
    QString secondaryMetric;
    if (metricsObject.contains(QStringLiteral("sound_vpp"))) {
        secondaryMetric = QStringLiteral("%1 Vpp").arg(metricsObject.value(QStringLiteral("sound_vpp")).toDouble(), 0, 'f', 3);
    }
    const bool relayOn = metricsObject.value(QStringLiteral("relay_state")).toBool(false)
        || device.value(QStringLiteral("state")).toObject().value(QStringLiteral("relay")).toBool(false);
    if (!secondaryMetric.isEmpty()) {
        secondaryMetric += relayOn ? tr("  •  Quạt ĐANG BẬT") : tr("  •  Quạt TẮT");
    }

    auto *primary = new QLabel(primaryMetric, card);
    primary->setObjectName(QStringLiteral("devicePrimaryMetric"));
    auto *secondary = new QLabel(secondaryMetric.isEmpty() ? deviceTypeName(type) : secondaryMetric, card);
    secondary->setObjectName(QStringLiteral("deviceSecondaryMetric"));

    metricBlock->addWidget(primary);
    metricBlock->addWidget(secondary);
    top->addLayout(metricBlock, 1);

    auto *status = new QLabel(online ? tr("● Online") : tr("● Offline"), card);
    status->setObjectName(online ? QStringLiteral("onlinePill") : QStringLiteral("offlinePill"));
    top->addWidget(status, 0, Qt::AlignTop | Qt::AlignRight);
    layout->addLayout(top);

    // Middle row: Name & ID
    QString devName = device.value(QStringLiteral("name")).toString();
    if (devName.isEmpty()) devName = tr("Trạm Làm Mát Tự Động");
    auto *name = new QLabel(devName, card);
    name->setObjectName(QStringLiteral("deviceCardName"));
    layout->addWidget(name);

    auto *id = new QLabel(tr("ID: %1").arg(deviceId), card);
    id->setObjectName(QStringLiteral("deviceCardId"));
    layout->addWidget(id);

    // Bottom row: Relay switch & Config button
    auto *bottom = new QHBoxLayout;
    bottom->setContentsMargins(0, 0, 0, 0);
    bottom->setSpacing(8);

    auto *configBtn = new QPushButton(tr("Cài đặt && Ngưỡng"), card);
    configBtn->setObjectName(QStringLiteral("cardConfigButton"));
    configBtn->setCursor(Qt::PointingHandCursor);
    connect(configBtn, &QPushButton::clicked, this, [this, device] {
        openDeviceDrawer(device);
    });
    bottom->addWidget(configBtn);
    bottom->addStretch();

    if (hasRelay || metricsObject.contains(QStringLiteral("relay_state")) || metricsObject.contains(QStringLiteral("temperature_c"))) {
        auto *relayButton = new RelayToggle(relayOn, card);
        relayButton->setEnabled(online);
        relayButton->setFixedSize(130, 30);
        bottom->addWidget(relayButton);
        connect(relayButton, &QPushButton::clicked, this, [this, deviceId, relayOn, relayButton] {
            relayButton->setPending();
            emit relayControlRequested(deviceId, !relayOn);
        });
    }

    layout->addLayout(bottom);

    card->clicked = [this, device] { openDeviceDrawer(device); };
    return card;
}

QWidget *DeviceManagementPage::createAvailableCard(const QJsonObject &device)
{
    auto *card = new QFrame(this);
    card->setObjectName(QStringLiteral("availableDeviceCard"));
    card->setMinimumHeight(100);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    const QString deviceId = device.value(QStringLiteral("device_id")).toString();
    const QString type = device.value(QStringLiteral("device_type")).toString();
    card->setProperty("deviceType", type);

    auto *layout = new QHBoxLayout(card);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(10);

    auto *icon = new QLabel(deviceIcon(type), card);
    icon->setStyleSheet("font-size: 24px; color: #38bdf8; background: #111d3d; border: 1px solid #233870; border-radius: 8px; padding: 4px 8px;");
    layout->addWidget(icon, 0, Qt::AlignVCenter);

    auto *textBlock = new QVBoxLayout;
    textBlock->setContentsMargins(0, 0, 0, 0);
    textBlock->setSpacing(2);

    auto *title = new QLabel(tr("Phát hiện thiết bị ESP32"), card);
    title->setObjectName(QStringLiteral("deviceCardName"));
    auto *id = new QLabel(tr("ID: %1").arg(deviceId), card);
    id->setObjectName(QStringLiteral("deviceCardId"));
    auto *typeLabel = new QLabel(deviceTypeName(type), card);
    typeLabel->setObjectName(QStringLiteral("deviceSecondaryMetric"));

    textBlock->addWidget(title);
    textBlock->addWidget(id);
    textBlock->addWidget(typeLabel);
    layout->addLayout(textBlock, 1);

    auto *claimButton = new QPushButton(tr("+ Thêm"), card);
    claimButton->setObjectName(QStringLiteral("claimDeviceButton"));
    claimButton->setCursor(Qt::PointingHandCursor);
    claimButton->setFixedSize(75, 30);
    connect(claimButton, &QPushButton::clicked, this, [this, deviceId, type] {
        const QString defaultName = tr("Trạm Làm Mát %1").arg(deviceId.right(4));
        emit claimDeviceRequested(deviceId, defaultName);
    });
    layout->addWidget(claimButton, 0, Qt::AlignVCenter);

    return card;
}

void DeviceManagementPage::rebuildOwnedGrid()
{
    clearGrid(m_ownedGrid);
    m_ownedEmpty->setVisible(m_ownedDevices.isEmpty());
    const int columns = m_gridColumns;
    for (int i = 0; i < m_ownedDevices.size(); ++i) {
        m_ownedGrid->addWidget(createOwnedCard(m_ownedDevices.at(i).toObject()),
                               i / columns, i % columns);
    }
    for (int c = 0; c < columns; ++c)
        m_ownedGrid->setColumnStretch(c, 1);
}

void DeviceManagementPage::rebuildAvailableGrid()
{
    clearGrid(m_availableGrid);
    m_availableEmpty->setVisible(m_availableDevices.isEmpty());
    const int columns = m_gridColumns;
    for (int i = 0; i < m_availableDevices.size(); ++i) {
        m_availableGrid->addWidget(createAvailableCard(m_availableDevices.at(i).toObject()),
                                   i / columns, i % columns);
    }
    for (int c = 0; c < columns; ++c)
        m_availableGrid->setColumnStretch(c, 1);
}

void DeviceManagementPage::rebuildLogTable()
{
    if (!m_deviceLogTable)
        return;

    const QString filter = m_logSearchEdit ? m_logSearchEdit->text().trimmed().toLower() : QString();
    m_deviceLogTable->setRowCount(0);

    int onlineCount = 0;
    int displayedRow = 0;

    for (int i = 0; i < m_ownedDevices.size(); ++i) {
        const QJsonObject dev = m_ownedDevices.at(i).toObject();
        const QString devId = dev.value(QStringLiteral("device_id")).toString();
        const QString name = dev.value(QStringLiteral("name")).toString();
        const QString type = dev.value(QStringLiteral("device_type")).toString();
        const bool isOnline = dev.value(QStringLiteral("online")).toBool();
        const QJsonObject metrics = dev.value(QStringLiteral("metrics")).toObject();

        if (isOnline) onlineCount++;

        // Filter
        if (!filter.isEmpty()) {
            const QString searchTarget = QStringLiteral("%1 %2 %3").arg(devId, name, deviceTypeName(type)).toLower();
            if (!searchTarget.contains(filter))
                continue;
        }

        m_deviceLogTable->insertRow(displayedRow);
        m_deviceLogTable->setRowHeight(displayedRow, 34);

        auto *sttItem = new QTableWidgetItem(QString::number(displayedRow + 1));
        sttItem->setTextAlignment(Qt::AlignCenter);

        QString cleanDisplayName = name.isEmpty() ? tr("Trạm Làm Mát Tự Động") : name;
        auto *nameItem = new QTableWidgetItem(cleanDisplayName);
        nameItem->setFont(QFont(font().family(), 10, QFont::Bold));

        auto *idItem = new QTableWidgetItem(devId);
        idItem->setTextAlignment(Qt::AlignCenter);

        QString summary = metricsSummary(metrics);
        auto *metricItem = new QTableWidgetItem(summary);
        metricItem->setForeground(QColor("#38bdf8"));

        auto *statusItem = new QTableWidgetItem(isOnline ? tr("Online") : tr("Offline"));
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setForeground(isOnline ? QColor("#10b981") : QColor("#ef4444"));

        m_deviceLogTable->setItem(displayedRow, 0, sttItem);
        m_deviceLogTable->setItem(displayedRow, 1, nameItem);
        m_deviceLogTable->setItem(displayedRow, 2, idItem);
        m_deviceLogTable->setItem(displayedRow, 3, metricItem);
        m_deviceLogTable->setItem(displayedRow, 4, statusItem);

        // Action widget with Config and Delete buttons
        auto *actionWidget = new QWidget(m_deviceLogTable);
        auto *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 2, 4, 2);
        actionLayout->setSpacing(4);

        auto *cfgBtn = new QPushButton(tr("Cấu hình"), actionWidget);
        cfgBtn->setObjectName(QStringLiteral("tableActionConfigBtn"));
        cfgBtn->setCursor(Qt::PointingHandCursor);
        connect(cfgBtn, &QPushButton::clicked, this, [this, dev] {
            openDeviceDrawer(dev);
        });

        auto *delBtn = new QPushButton(tr("Xóa"), actionWidget);
        delBtn->setObjectName(QStringLiteral("tableActionDeleteBtn"));
        delBtn->setCursor(Qt::PointingHandCursor);
        connect(delBtn, &QPushButton::clicked, this, [this, devId, name] {
            if (confirmDeleteDevice(this, tr("Xác nhận gỡ trạm làm mát"),
                    tr("Bạn có chắc chắn muốn gỡ trạm '%1' (ID: %2) khỏi tài khoản?")
                        .arg(name.isEmpty() ? devId : name, devId))) {
                emit releaseDeviceRequested(devId);
            }
        });

        actionLayout->addWidget(cfgBtn);
        actionLayout->addWidget(delBtn);
        m_deviceLogTable->setCellWidget(displayedRow, 5, actionWidget);

        displayedRow++;
    }

    if (m_statTotalDevices)
        m_statTotalDevices->setText(tr("Tổng: %1").arg(m_ownedDevices.size()));
    if (m_statOnlineDevices)
        m_statOnlineDevices->setText(tr("Online: %1").arg(onlineCount));

    if (m_logEmptyLabel) {
        m_logEmptyLabel->setVisible(displayedRow == 0);
    }
}

void DeviceManagementPage::openDeviceDrawer(const QJsonObject &device)
{
    const QString deviceId = device.value(QStringLiteral("device_id")).toString();

    // Luôn lấy đối tượng mới nhất
    QJsonObject currentDevice = device;
    for (const QJsonValue &val : m_ownedDevices) {
        const QJsonObject d = val.toObject();
        if (d.value(QStringLiteral("device_id")).toString().compare(deviceId, Qt::CaseInsensitive) == 0) {
            currentDevice = d;
            break;
        }
    }

    m_selectedDevice = currentDevice;
    const QString type = currentDevice.value(QStringLiteral("device_type")).toString();
    QString name = currentDevice.value(QStringLiteral("name")).toString();
    if (name.isEmpty()) name = tr("Trạm Làm Mát Tự Động");

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Chi tiết & Cài đặt trạm làm mát"));
    dlg.setModal(true);
    dlg.setFixedSize(540, 420);
    dlg.setStyleSheet(
        "QDialog { background-color: #0b152d; color: #ffffff; font-family: sans-serif; } "
        "QLabel { color: #f1f5f9; font-size: 11px; font-weight: 700; } "
        "QLabel#dlgTitle { color: #38bdf8; font-size: 15px; font-weight: 900; } "
        "QLabel#dlgSubtitle { color: #94a3b8; font-size: 10px; font-weight: 600; } "
        "QLabel#dlgMetrics { color: #34d399; font-size: 11px; font-weight: 800; background: #111d3d; border: 1px solid #233870; border-radius: 6px; padding: 6px 10px; } "
        "QLabel#dlgSection { color: #38bdf8; font-size: 12px; font-weight: 800; margin-top: 2px; } "
        "QDoubleSpinBox, QSpinBox { background-color: #111d3d; color: #ffffff; border: 1.5px solid #233870; border-radius: 6px; padding: 4px 8px; font-size: 12px; font-weight: 800; min-height: 28px; } "
        "QDoubleSpinBox:focus, QSpinBox:focus { border-color: #38bdf8; background-color: #172554; } "
        "QPushButton#saveBtn { background-color: #10b981; color: #ffffff; border: none; border-radius: 6px; font-size: 11px; font-weight: 900; padding: 7px 16px; } "
        "QPushButton#saveBtn:hover { background-color: #059669; } "
        "QPushButton#deleteBtn { background-color: #dc2626; color: #ffffff; border: none; border-radius: 6px; font-size: 11px; font-weight: 800; padding: 7px 14px; } "
        "QPushButton#deleteBtn:hover { background-color: #b91c1c; } "
        "QPushButton#closeBtn { background-color: #1e293b; color: #ffffff; border: 1px solid #475569; border-radius: 6px; font-size: 11px; font-weight: 800; padding: 7px 14px; } "
        "QPushButton#closeBtn:hover { background-color: #334155; }"
    );

    auto *dlgLayout = new QVBoxLayout(&dlg);
    dlgLayout->setContentsMargins(16, 14, 16, 14);
    dlgLayout->setSpacing(8);

    // Header
    auto *headLayout = new QHBoxLayout;
    auto *iconLabel = new QLabel(deviceIcon(type), &dlg);
    iconLabel->setStyleSheet("font-size: 26px; color: #38bdf8; background: #111d3d; border: 1px solid #233870; border-radius: 8px; padding: 6px 12px;");
    headLayout->addWidget(iconLabel);

    auto *infoLayout = new QVBoxLayout;
    infoLayout->setSpacing(2);
    auto *titleLabel = new QLabel(name, &dlg);
    titleLabel->setObjectName("dlgTitle");
    auto *subLabel = new QLabel(tr("Device ID: %1").arg(deviceId), &dlg);
    subLabel->setObjectName("dlgSubtitle");
    infoLayout->addWidget(titleLabel);
    infoLayout->addWidget(subLabel);
    headLayout->addLayout(infoLayout, 1);
    dlgLayout->addLayout(headLayout);

    // Current Metrics
    auto *metricsLabel = new QLabel(tr("Dữ liệu hiện tại: %1").arg(metricsSummary(currentDevice.value(QStringLiteral("metrics")).toObject())), &dlg);
    metricsLabel->setObjectName("dlgMetrics");
    metricsLabel->setWordWrap(true);
    dlgLayout->addWidget(metricsLabel);

    // Section title
    auto *formSection = new QLabel(tr("Cài đặt ngưỡng cảnh báo & tham số"), &dlg);
    formSection->setObjectName("dlgSection");
    dlgLayout->addWidget(formSection);

    // Form inside ScrollArea
    auto *scrollArea = new QScrollArea(&dlg);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background: transparent;");

    auto *formWidget = new QWidget(scrollArea);
    formWidget->setStyleSheet("background: transparent;");
    auto *formLayout = new QFormLayout(formWidget);
    formLayout->setContentsMargins(0, 4, 0, 4);
    formLayout->setSpacing(8);

    const QJsonObject saved = currentDevice.value(QStringLiteral("config")).toObject();
    const QJsonObject savedThresholds = saved.value(QStringLiteral("thresholds")).toObject();
    QHash<QString, QDoubleSpinBox *> inputs;

    auto addThreshold = [&](const QString &key, const QString &label,
                            double fallback, double minimum,
                            double maximum, const QString &suffix) {
        const QStringList parts = key.split('.');
        double val = fallback;
        if (parts.size() == 2)
            val = savedThresholds.value(parts.at(0)).toObject().value(parts.at(1)).toDouble(fallback);

        auto *spin = new QDoubleSpinBox(formWidget);
        spin->setRange(minimum, maximum);
        spin->setDecimals(2);
        spin->setValue(val);
        spin->setSuffix(suffix);
        inputs.insert(key, spin);

        auto *lbl = new QLabel(label, formWidget);
        lbl->setStyleSheet("color: #cbd5e1; font-size: 11px; font-weight: 700;");
        formLayout->addRow(lbl, spin);
    };

    // Smart Cooling System thresholds
    addThreshold(QStringLiteral("temperature_c.warning_above"), tr("Nhiệt độ cảnh báo (°C)"), 35.0, 0, 100, tr(" °C"));
    addThreshold(QStringLiteral("temperature_c.critical_above"), tr("Nhiệt độ nguy hiểm (°C)"), 42.0, 0, 100, tr(" °C"));
    addThreshold(QStringLiteral("sound_vpp.warning_above"), tr("Độ ồn cảnh báo (Vpp)"), 1.20, 0, 3.3, tr(" Vpp"));

    auto *intervalSpin = new QSpinBox(formWidget);
    intervalSpin->setRange(1, 3600);
    intervalSpin->setValue(saved.value(QStringLiteral("sampling_interval_ms")).toInt(2000) / 1000);
    intervalSpin->setSuffix(tr(" giây"));
    auto *lblInterval = new QLabel(tr("Chu kỳ gửi"), formWidget);
    lblInterval->setStyleSheet("color: #cbd5e1; font-size: 11px; font-weight: 700;");
    formLayout->addRow(lblInterval, intervalSpin);

    scrollArea->setWidget(formWidget);
    dlgLayout->addWidget(scrollArea, 1);

    // Bottom buttons
    auto *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(8);

    auto *deleteBtn = new QPushButton(tr("Xóa thiết bị"), &dlg);
    deleteBtn->setObjectName("deleteBtn");
    deleteBtn->setCursor(Qt::PointingHandCursor);

    auto *closeBtn = new QPushButton(tr("Đóng"), &dlg);
    closeBtn->setObjectName("closeBtn");
    closeBtn->setCursor(Qt::PointingHandCursor);

    auto *saveBtn = new QPushButton(tr("Lưu && gửi xuống thiết bị"), &dlg);
    saveBtn->setObjectName("saveBtn");
    saveBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(deleteBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    btnLayout->addWidget(saveBtn);
    dlgLayout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    connect(deleteBtn, &QPushButton::clicked, this, [this, &dlg, deviceId, name] {
        if (confirmDeleteDevice(&dlg, tr("Xác nhận gỡ thiết bị"),
                tr("Bạn có chắc chắn muốn gỡ trạm '%1' (ID: %2) khỏi tài khoản?")
                    .arg(name.isEmpty() ? deviceId : name, deviceId))) {
            emit releaseDeviceRequested(deviceId);
            dlg.accept();
        }
    });

    connect(saveBtn, &QPushButton::clicked, this, [this, &dlg, deviceId, inputs, intervalSpin] {
        QJsonObject thresholds;
        for (auto it = inputs.cbegin(); it != inputs.cend(); ++it) {
            const QStringList parts = it.key().split('.');
            if (parts.size() != 2) continue;
            QJsonObject sensor = thresholds.value(parts.at(0)).toObject();
            sensor.insert(parts.at(1), it.value()->value());
            thresholds.insert(parts.at(0), sensor);
        }
        const QJsonObject config{
            {QStringLiteral("sampling_interval_ms"), intervalSpin->value() * 1000},
            {QStringLiteral("thresholds"), thresholds}
        };
        emit deviceConfigRequested(deviceId, config);
        dlg.accept();
    });

    dlg.exec();
}

void DeviceManagementPage::openFirstDeviceConfig()
{
    if (!m_ownedDevices.isEmpty()) {
        openDeviceDrawer(m_ownedDevices.first().toObject());
    }
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
    Q_UNUSED(type);
    return QStringLiteral("");
}

QString DeviceManagementPage::deviceTypeName(const QString &type)
{
    Q_UNUSED(type);
    return QObject::tr("Trạm Làm Mát & Cảm Biến");
}

QString DeviceManagementPage::metricsSummary(const QJsonObject &metrics)
{
    QStringList parts;
    if (metrics.contains(QStringLiteral("temperature_c")))
        parts << QStringLiteral("Nhiệt độ %1 °C").arg(metrics.value(QStringLiteral("temperature_c")).toDouble(), 0, 'f', 1);
    if (metrics.contains(QStringLiteral("sound_vpp")))
        parts << QStringLiteral("Độ ồn %1 Vpp").arg(metrics.value(QStringLiteral("sound_vpp")).toDouble(), 0, 'f', 3);
    const bool relay = metrics.value(QStringLiteral("relay_state")).toBool(false);
    parts << (relay ? QObject::tr("Quạt BẬT") : QObject::tr("Quạt TẮT"));
    return parts.isEmpty() ? QObject::tr("Đang chờ dữ liệu...") : parts.join(QStringLiteral("  •  "));
}
