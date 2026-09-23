#include "DeviceManagementPage.h"
#include "VirtualKeyboard.h"

#include <QButtonGroup>
#include <QCheckBox>
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
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

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
    explicit RelayToggle(bool on, const QString &label = QObject::tr("Đèn phòng"), QWidget *parent = nullptr)
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
        painter.setBrush(isDown() ? QColor("#1e293b") : QColor("#0f172a"));
        painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 8, 8);

        const QColor textColor = isEnabled() ? (m_on ? QColor("#fbbf24") : QColor("#94a3b8")) : QColor("#64748b");
        painter.setPen(textColor);
        QFont textFont = font();
        textFont.setPointSize(9);
        textFont.setWeight(QFont::Bold);
        painter.setFont(textFont);
        painter.drawText(QRect(10, 0, width() - 56, height()),
                         Qt::AlignVCenter | Qt::AlignLeft,
                         m_pending ? tr("Đang gửi…")
                                   : tr("%1: %2").arg(m_label, m_on ? tr("BẬT") : tr("TẮT")));

        const QRectF track(width() - 50, 7, 40, 18);
        painter.setPen(Qt::NoPen);
        painter.setBrush(!isEnabled() ? QColor("#334155")
                         : m_on ? QColor("#f59e0b") : QColor("#475569"));
        painter.drawRoundedRect(track, 9, 9);
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
      m_ownedEmpty(new QLabel(tr("Chưa có thiết bị nào được liên kết."), this)),
      m_availableEmpty(new QLabel(tr("Đang quét thiết bị ánh sáng online..."), this)),
      m_liveLabel(new QLabel(tr("● Realtime"), this)),
      m_refreshTimer(new QTimer(this))
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral(
        "DeviceManagementPage { background-color: #060b17; }"
        "QScrollArea, QScrollArea > QWidget, QScrollArea > QWidget > QWidget { background-color: #060b17; border: none; }"
        "QLabel { color: #e2e8f0; font-family: 'Segoe UI', 'Roboto', sans-serif; }"
    ));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 10);
    root->setSpacing(8);

    // Header Toolbar
    auto *header = new QHBoxLayout;
    header->setSpacing(8);

    auto *titles = new QVBoxLayout;
    titles->setSpacing(2);
    auto *title = new QLabel(tr("Quản Lý Thiết Bị Cảm Biến & Đèn Chiếu Sáng"), this);
    title->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: 800; color: #f8fafc;"));
    auto *subtitle = new QLabel(
        tr("Theo dõi trạng thái các node cảm biến quang học BH1750, phát hiện PIR và rơ-le đèn phòng."), this);
    subtitle->setStyleSheet(QStringLiteral("font-size: 10px; color: #94a3b8;"));
    titles->addWidget(title);
    titles->addWidget(subtitle);
    header->addLayout(titles, 1);

    m_liveLabel->setStyleSheet(QStringLiteral(
        "color: #10b981; font-size: 10px; font-weight: 700; background: #064e3b; "
        "border: 1px solid #059669; border-radius: 10px; padding: 2px 8px;"));
    header->addWidget(m_liveLabel, 0, Qt::AlignVCenter);

    auto *refreshButton = new QPushButton(tr("Làm mới"), this);
    refreshButton->setCursor(Qt::PointingHandCursor);
    refreshButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #1e293b; color: #cbd5e1; border: 1px solid #334155; "
        "border-radius: 6px; font-size: 10px; font-weight: 700; padding: 5px 12px; }"
        "QPushButton:hover { background: #334155; color: #ffffff; }"
    ));
    header->addWidget(refreshButton, 0, Qt::AlignVCenter);
    root->addLayout(header);

    // View Mode Tabs
    auto *toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);

    m_cardsTabBtn = new QPushButton(tr("Thẻ thiết bị & Điều khiển"), this);
    m_logTabBtn = new QPushButton(tr("Danh sách chi tiết"), this);
    for (auto *b : {m_cardsTabBtn, m_logTabBtn}) {
        b->setCursor(Qt::PointingHandCursor);
        b->setCheckable(true);
        b->setStyleSheet(QStringLiteral(
            "QPushButton { background: #0f172a; color: #64748b; border: 1px solid #1e293b; "
            "border-radius: 6px; font-size: 10px; font-weight: 700; padding: 5px 14px; }"
            "QPushButton:hover { background: #1e293b; color: #e2e8f0; }"
            "QPushButton:checked { background: #f59e0b; color: #020617; border-color: #fbbf24; }"
        ));
    }
    m_cardsTabBtn->setChecked(true);
    auto *tabGroup = new QButtonGroup(this);
    tabGroup->addButton(m_cardsTabBtn);
    tabGroup->addButton(m_logTabBtn);
    tabGroup->setExclusive(true);

    toolbar->addWidget(m_cardsTabBtn);
    toolbar->addWidget(m_logTabBtn);
    toolbar->addStretch();
    root->addLayout(toolbar);

    // Stacked Widget
    m_viewStack = new QStackedWidget(this);
    root->addWidget(m_viewStack, 1);

    // ==========================================
    // PAGE 0: CARDS VIEW (Owned & Available)
    // ==========================================
    auto *cardsScroll = new QScrollArea(this);
    cardsScroll->setWidgetResizable(true);
    auto *cardsContainer = new QWidget(cardsScroll);
    auto *cardsLayout = new QVBoxLayout(cardsContainer);
    cardsLayout->setContentsMargins(0, 4, 0, 4);
    cardsLayout->setSpacing(10);

    // Owned Section
    auto *ownedTitle = new QLabel(tr("THIẾT BỊ ĐÃ KẾT NỐI & ĐANG ĐIỀU KHIỂN"), cardsContainer);
    ownedTitle->setStyleSheet(QStringLiteral("font-size: 11px; font-weight: 800; color: #f59e0b; letter-spacing: 0.5px;"));
    cardsLayout->addWidget(ownedTitle);

    m_ownedGrid->setSpacing(8);
    cardsLayout->addLayout(m_ownedGrid);

    m_ownedEmpty->setStyleSheet(QStringLiteral("color: #64748b; font-style: italic; padding: 10px; font-size: 11px;"));
    cardsLayout->addWidget(m_ownedEmpty);

    // Available Section
    auto *availTitle = new QLabel(tr("THIẾT BỊ ONLINE CHỜ LIÊN KẾT"), cardsContainer);
    availTitle->setStyleSheet(QStringLiteral("font-size: 11px; font-weight: 800; color: #38bdf8; letter-spacing: 0.5px; margin-top: 6px;"));
    cardsLayout->addWidget(availTitle);

    m_availableGrid->setSpacing(8);
    cardsLayout->addLayout(m_availableGrid);

    m_availableEmpty->setStyleSheet(QStringLiteral("color: #64748b; font-style: italic; padding: 10px; font-size: 11px;"));
    cardsLayout->addWidget(m_availableEmpty);

    cardsLayout->addStretch(1);
    cardsScroll->setWidget(cardsContainer);
    m_viewStack->addWidget(cardsScroll);

    // ==========================================
    // PAGE 1: TABLE VIEW (Detailed List)
    // ==========================================
    auto *tableView = new QWidget(this);
    auto *tableLayout = new QVBoxLayout(tableView);
    tableLayout->setContentsMargins(0, 4, 0, 0);
    tableLayout->setSpacing(8);

    // KPI stats row
    auto *statRow = new QHBoxLayout;
    statRow->setSpacing(8);
    auto makeStatPill = [](const QString &label, QLabel *&valOut, const QString &color) {
        auto *frame = new QFrame;
        frame->setStyleSheet(QStringLiteral(
            "QFrame { background: #0f172a; border: 1px solid #1e293b; border-radius: 6px; }"));
        auto *l = new QHBoxLayout(frame);
        l->setContentsMargins(10, 6, 10, 6);
        auto *lbl = new QLabel(label);
        lbl->setStyleSheet(QStringLiteral("font-size: 10px; color: #94a3b8; font-weight: 700;"));
        valOut = new QLabel(QStringLiteral("0"));
        valOut->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 800; color: %1;").arg(color));
        l->addWidget(lbl);
        l->addStretch();
        l->addWidget(valOut);
        return frame;
    };
    statRow->addWidget(makeStatPill(tr("Tổng thiết bị:"), m_statTotalDevices, QStringLiteral("#38bdf8")), 1);
    statRow->addWidget(makeStatPill(tr("Online:"), m_statOnlineDevices, QStringLiteral("#10b981")), 1);
    statRow->addWidget(makeStatPill(tr("Người dùng:"), m_statLinkedUsers, QStringLiteral("#fbbf24")), 1);
    tableLayout->addLayout(statRow);

    // Search bar
    auto *searchRow = new QHBoxLayout;
    m_logSearchEdit = new QLineEdit(this);
    m_logSearchEdit->setPlaceholderText(tr("Tìm theo ID thiết bị, tên, người sở hữu..."));
    m_logSearchEdit->setStyleSheet(QStringLiteral(
        "QLineEdit { background: #0f172a; color: #f8fafc; border: 1px solid #1e293b; "
        "border-radius: 6px; padding: 4px 10px; font-size: 10px; }"
        "QLineEdit:focus { border-color: #f59e0b; }"
    ));
    searchRow->addWidget(m_logSearchEdit, 1);
    VirtualKeyboardDialog::attachToLineEdit(m_logSearchEdit, tr("Tìm kiếm thiết bị"));
    tableLayout->addLayout(searchRow);

    // Table
    m_deviceLogTable = new QTableWidget(this);
    m_deviceLogTable->setColumnCount(6);
    m_deviceLogTable->setHorizontalHeaderLabels({
        tr("Mã Thiết Bị"), tr("Tên / Vị Trí"), tr("Trạng Thái"),
        tr("Thông Số Cảm Biến"), tr("Chủ Sở Hữu"), tr("Thao Tác")
    });
    m_deviceLogTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_deviceLogTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_deviceLogTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_deviceLogTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_deviceLogTable->verticalHeader()->hide();
    m_deviceLogTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_deviceLogTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_deviceLogTable->setStyleSheet(QStringLiteral(
        "QTableWidget { background-color: #0b1329; alternate-background-color: #111c38; border: 1px solid #1e293b; border-radius: 6px; "
        "gridline-color: #1e293b; color: #f8fafc; font-size: 11px; }"
        "QHeaderView::section { background-color: #080d1a; color: #94a3b8; font-weight: 800; "
        "padding: 6px 8px; border: none; border-bottom: 1.5px solid #1e293b; font-size: 10px; }"
        "QTableWidget::item { padding: 5px 8px; color: #f8fafc; }"
        "QTableWidget::item:alternate { background-color: #111c38; color: #f8fafc; }"
        "QTableWidget::item:selected { background-color: #0284c7; color: #ffffff; }"
    ));
    tableLayout->addWidget(m_deviceLogTable, 1);

    m_viewStack->addWidget(tableView);

    // Tab Switching
    connect(m_cardsTabBtn, &QPushButton::clicked, this, [this] {
        m_viewStack->setCurrentIndex(0);
    });
    connect(m_logTabBtn, &QPushButton::clicked, this, [this] {
        m_viewStack->setCurrentIndex(1);
        rebuildLogTable();
    });
    connect(m_logSearchEdit, &QLineEdit::textChanged, this, &DeviceManagementPage::rebuildLogTable);

    m_refreshTimer->setInterval(5000);
    connect(m_refreshTimer, &QTimer::timeout, this, &DeviceManagementPage::refreshRequested);
    connect(refreshButton, &QPushButton::clicked, this, &DeviceManagementPage::refreshRequested);

    applyResponsiveLayout();
}

void DeviceManagementPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    applyResponsiveLayout();
}

void DeviceManagementPage::applyResponsiveLayout()
{
    const int cols = (width() < 700) ? 1 : 2;
    if (m_gridColumns == cols)
        return;
    m_gridColumns = cols;
    rebuildOwnedGrid();
    rebuildAvailableGrid();
}

void DeviceManagementPage::startRealtime()
{
    if (!m_refreshTimer->isActive())
        m_refreshTimer->start();
    m_liveLabel->setText(tr("● Realtime"));
    m_liveLabel->setStyleSheet(QStringLiteral(
        "color: #10b981; font-size: 10px; font-weight: 700; background: #064e3b; "
        "border: 1px solid #059669; border-radius: 10px; padding: 2px 8px;"));
}

void DeviceManagementPage::stopRealtime()
{
    m_refreshTimer->stop();
    m_liveLabel->setText(tr("Tạm dừng"));
    m_liveLabel->setStyleSheet(QStringLiteral(
        "color: #94a3b8; font-size: 10px; font-weight: 700; background: #1e293b; "
        "border: 1px solid #334155; border-radius: 10px; padding: 2px 8px;"));
}

void DeviceManagementPage::configSaved(const QString &deviceId, bool mqttPublished)
{
    Q_UNUSED(deviceId);
    Q_UNUSED(mqttPublished);
    emit refreshRequested();
}

void DeviceManagementPage::setCurrentUser(const QString &username, bool isAdmin)
{
    m_currentUsername = username.trimmed();
    m_isAdmin = isAdmin;
    rebuildOwnedGrid();
    rebuildLogTable();
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
    rebuildLogTable();
}

void DeviceManagementPage::clearGrid(QGridLayout *layout)
{
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }
}

QWidget *DeviceManagementPage::createOwnedCard(const QJsonObject &device)
{
    auto *card = new ClickableFrame(this);
    card->setObjectName(QStringLiteral("ownedDeviceCard"));
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    card->setFixedHeight(120);
    card->setStyleSheet(QStringLiteral(
        "QFrame#ownedDeviceCard { background-color: #0d1527; border: 1px solid #1e293b; "
        "border-radius: 8px; }"
        "QFrame#ownedDeviceCard:hover { border-color: #f59e0b; background-color: #111c33; }"
    ));

    auto *layout = new QHBoxLayout(card);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(12);

    const QString deviceId = device.value(QStringLiteral("device_id")).toString();
    const QString name = device.value(QStringLiteral("name")).toString(deviceId);
    const QString addedBy = device.value(QStringLiteral("added_by")).toString();
    const bool online = device.value(QStringLiteral("online")).toBool();
    const QJsonObject metrics = device.value(QStringLiteral("metrics")).toObject();

    // Col 1: Icon & Identity
    auto *col1 = new QVBoxLayout;
    col1->setSpacing(3);
    auto *idRow = new QHBoxLayout;
    idRow->setSpacing(6);

    auto *iconBadge = new QLabel(QStringLiteral("LIGHT"), card);
    iconBadge->setStyleSheet(QStringLiteral(
        "font-size: 10px; font-weight: 900; color: #fbbf24; background: #1e293b; border-radius: 6px; padding: 4px 6px;"));
    iconBadge->setAlignment(Qt::AlignCenter);
    idRow->addWidget(iconBadge);

    auto *titleLayout = new QVBoxLayout;
    titleLayout->setSpacing(0);
    auto *nameLabel = new QLabel(name, card);
    nameLabel->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 800; color: #f8fafc;"));
    auto *idLabel = new QLabel(tr("ID: %1").arg(deviceId), card);
    idLabel->setStyleSheet(QStringLiteral("font-size: 9px; color: #64748b;"));
    titleLayout->addWidget(nameLabel);
    titleLayout->addWidget(idLabel);
    idRow->addLayout(titleLayout);
    idRow->addStretch();
    col1->addLayout(idRow);

    auto *ownerLabel = new QLabel(addedBy.isEmpty() ? tr("Chưa gán chủ") : tr("Sở hữu: %1").arg(addedBy), card);
    ownerLabel->setStyleSheet(QStringLiteral("font-size: 9px; color: #94a3b8;"));
    col1->addWidget(ownerLabel);

    auto *statusPill = new QLabel(online ? tr("● ONLINE") : tr("OFFLINE"), card);
    statusPill->setStyleSheet(online ? QStringLiteral("color: #10b981; font-size: 8px; font-weight: 800;")
                                     : QStringLiteral("color: #ef4444; font-size: 8px; font-weight: 800;"));
    col1->addWidget(statusPill);
    layout->addLayout(col1, 4);

    // Col 2: Live Metrics Readings
    auto *col2 = new QVBoxLayout;
    col2->setSpacing(4);
    col2->setAlignment(Qt::AlignVCenter);

    double luxVal = 0.0;
    if (metrics.contains(QStringLiteral("light_lux")))
        luxVal = metrics.value(QStringLiteral("light_lux")).toDouble();
    else if (metrics.contains(QStringLiteral("lux")))
        luxVal = metrics.value(QStringLiteral("lux")).toDouble();

    bool motion = metrics.value(QStringLiteral("motion_detected")).toInt() > 0 ||
                  metrics.value(QStringLiteral("motion")).toInt() > 0;

    auto *luxLabel = new QLabel(tr("Độ sáng: %1 Lux").arg(luxVal, 0, 'f', 1), card);
    luxLabel->setStyleSheet(QStringLiteral("font-size: 11px; font-weight: 700; color: #fbbf24;"));
    auto *motionLabel = new QLabel(motion ? tr("[!] Phát hiện chuyển động") : tr("[] Không có chuyển động"), card);
    motionLabel->setStyleSheet(motion ? QStringLiteral("font-size: 10px; font-weight: 700; color: #f43f5e;")
                                      : QStringLiteral("font-size: 10px; color: #94a3b8;"));

    col2->addWidget(luxLabel);
    col2->addWidget(motionLabel);
    layout->addLayout(col2, 3);

    // Col 3: Quick Action & Config Button
    auto *col3 = new QVBoxLayout;
    col3->setSpacing(6);
    col3->setAlignment(Qt::AlignVCenter);

    bool relayOn = metrics.value(QStringLiteral("relay_on")).toBool() ||
                   metrics.value(QStringLiteral("relay")).toInt() > 0;

    auto *toggle = new RelayToggle(relayOn, tr("Đèn"), card);
    connect(toggle, &QPushButton::clicked, this, [this, deviceId, relayOn, toggle] {
        toggle->setPending();
        emit relayControlRequested(deviceId, !relayOn);
    });
    col3->addWidget(toggle);

    auto *cfgBtn = new QPushButton(tr("Cài đặt"), card);
    cfgBtn->setCursor(Qt::PointingHandCursor);
    cfgBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #1e293b; color: #cbd5e1; border: 1px solid #334155; "
        "border-radius: 5px; font-size: 9px; font-weight: 700; padding: 4px; }"
        "QPushButton:hover { background: #334155; color: #f59e0b; }"
    ));
    connect(cfgBtn, &QPushButton::clicked, this, [this, device] {
        openDeviceConfigDialog(device);
    });
    col3->addWidget(cfgBtn);

    layout->addLayout(col3, 3);

    card->clicked = [this, device] {
        openDeviceConfigDialog(device);
    };

    return card;
}

QWidget *DeviceManagementPage::createAvailableCard(const QJsonObject &device)
{
    auto *card = new QFrame(this);
    card->setObjectName(QStringLiteral("availableDeviceCard"));
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    card->setFixedHeight(115);
    card->setStyleSheet(QStringLiteral(
        "QFrame#availableDeviceCard { background-color: #0c1524; border: 1px dashed #0284c7; "
        "border-radius: 8px; }"
    ));

    auto *layout = new QHBoxLayout(card);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(12);

    const QString deviceId = device.value(QStringLiteral("device_id")).toString();

    auto *iconBadge = new QLabel(QStringLiteral(""), card); iconBadge->hide();
    iconBadge->setStyleSheet(QStringLiteral(
        "font-size: 18px; background: #0369a1; border-radius: 6px; padding: 6px;"));
    iconBadge->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconBadge);

    auto *infoLayout = new QVBoxLayout;
    infoLayout->setSpacing(2);
    infoLayout->setAlignment(Qt::AlignVCenter);

    auto *title = new QLabel(tr("Cảm Biến Ánh Sáng Mới"), card);
    title->setStyleSheet(QStringLiteral("font-size: 11px; font-weight: 800; color: #38bdf8;"));
    auto *idLabel = new QLabel(tr("Device ID: %1").arg(deviceId), card);
    idLabel->setStyleSheet(QStringLiteral("font-size: 9px; color: #94a3b8;"));
    auto *hint = new QLabel(tr("Đang trực tuyến • Sẵn sàng thêm vào hệ thống"), card);
    hint->setStyleSheet(QStringLiteral("font-size: 9px; color: #10b981;"));

    infoLayout->addWidget(title);
    infoLayout->addWidget(idLabel);
    infoLayout->addWidget(hint);
    layout->addLayout(infoLayout, 1);

    auto *claimBtn = new QPushButton(tr("+ Thêm Thiết Bị"), card);
    claimBtn->setCursor(Qt::PointingHandCursor);
    claimBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #0284c7; color: #ffffff; border: none; "
        "border-radius: 6px; font-size: 10px; font-weight: 800; padding: 8px 14px; }"
        "QPushButton:hover { background: #0369a1; }"
    ));
    connect(claimBtn, &QPushButton::clicked, this, [this, deviceId] {
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Đặt Tên Thiết Bị"));
        dialog.setModal(true);
        dialog.setFixedWidth(400);
        dialog.setStyleSheet(QStringLiteral("background-color: #0b1329; color: #f8fafc;"));

        auto *root = new QVBoxLayout(&dialog);
        root->setContentsMargins(16, 16, 16, 16);
        root->setSpacing(12);

        auto *t = new QLabel(tr("Thêm Node Cảm Biến: %1").arg(deviceId), &dialog);
        t->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 800; color: #f59e0b;"));
        root->addWidget(t);

        auto *nameEdit = new QLineEdit(tr("Đèn Phòng Khách"), &dialog);
        nameEdit->setStyleSheet(QStringLiteral(
            "background: #1e293b; color: #ffffff; border: 1px solid #334155; "
            "border-radius: 6px; padding: 6px 10px; font-size: 11px;"));
        root->addWidget(nameEdit);
        VirtualKeyboardDialog::attachToLineEdit(nameEdit, tr("Tên vị trí lắp đặt"));

        auto *btns = new QHBoxLayout;
        btns->setSpacing(8);
        auto *cancel = new QPushButton(tr("Hủy"), &dialog);
        cancel->setStyleSheet(QStringLiteral(
            "background: #1e293b; color: #94a3b8; border-radius: 6px; padding: 6px 12px;"));
        auto *confirm = new QPushButton(tr("Xác Nhận Thêm"), &dialog);
        confirm->setStyleSheet(QStringLiteral(
            "background: #f59e0b; color: #020617; font-weight: 800; border-radius: 6px; padding: 6px 14px;"));
        btns->addStretch();
        btns->addWidget(cancel);
        btns->addWidget(confirm);
        root->addLayout(btns);

        connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
        connect(confirm, &QPushButton::clicked, &dialog, &QDialog::accept);

        if (dialog.exec() == QDialog::Accepted) {
            const QString finalName = nameEdit->text().trimmed();
            emit claimDeviceRequested(deviceId, finalName.isEmpty() ? deviceId : finalName);
        }
    });
    layout->addWidget(claimBtn, 0, Qt::AlignVCenter);

    return card;
}

void DeviceManagementPage::rebuildOwnedGrid()
{
    clearGrid(m_ownedGrid);
    QJsonArray userDevices;
    for (const QJsonValue &val : m_ownedDevices) {
        const QJsonObject dev = val.toObject();
        const QString addedBy = dev.value(QStringLiteral("added_by")).toString();
        if (m_isAdmin || m_currentUsername.isEmpty() || addedBy.isEmpty() ||
            addedBy.compare(m_currentUsername, Qt::CaseInsensitive) == 0) {
            userDevices.append(dev);
        }
    }

    m_ownedEmpty->setVisible(userDevices.isEmpty());
    const int columns = m_gridColumns;
    for (int i = 0; i < userDevices.size(); ++i) {
        m_ownedGrid->addWidget(createOwnedCard(userDevices.at(i).toObject()),
                               i / columns, i % columns);
    }
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
}

void DeviceManagementPage::rebuildLogTable()
{
    if (!m_deviceLogTable)
        return;

    const QString filter = m_logSearchEdit ? m_logSearchEdit->text().trimmed().toLower() : QString();
    m_deviceLogTable->setRowCount(0);

    QSet<QString> uniqueUsers;
    int onlineCount = 0;
    int rowIdx = 0;

    for (int i = 0; i < m_ownedDevices.size(); ++i) {
        const QJsonObject dev = m_ownedDevices.at(i).toObject();
        const QString devId = dev.value(QStringLiteral("device_id")).toString();
        const QString name = dev.value(QStringLiteral("name")).toString(devId);
        const QString addedBy = dev.value(QStringLiteral("added_by")).toString(tr("Chưa gán"));
        const bool online = dev.value(QStringLiteral("online")).toBool();
        const QJsonObject metrics = dev.value(QStringLiteral("metrics")).toObject();

        if (online) onlineCount++;
        if (!addedBy.isEmpty() && addedBy != tr("Chưa gán")) uniqueUsers.insert(addedBy);

        if (!filter.isEmpty() && !devId.toLower().contains(filter) &&
            !name.toLower().contains(filter) && !addedBy.toLower().contains(filter)) {
            continue;
        }

        m_deviceLogTable->insertRow(rowIdx);

        auto *item0 = new QTableWidgetItem(devId);
        item0->setTextAlignment(Qt::AlignCenter);
        m_deviceLogTable->setItem(rowIdx, 0, item0);

        auto *item1 = new QTableWidgetItem(name);
        m_deviceLogTable->setItem(rowIdx, 1, item1);

        auto *item2 = new QTableWidgetItem(online ? tr("● Online") : tr("Offline"));
        item2->setForeground(online ? QColor("#10b981") : QColor("#ef4444"));
        item2->setTextAlignment(Qt::AlignCenter);
        m_deviceLogTable->setItem(rowIdx, 2, item2);

        auto *item3 = new QTableWidgetItem(metricsSummary(metrics));
        m_deviceLogTable->setItem(rowIdx, 3, item3);

        auto *item4 = new QTableWidgetItem(addedBy);
        item4->setTextAlignment(Qt::AlignCenter);
        m_deviceLogTable->setItem(rowIdx, 4, item4);

        auto *actionBtn = new QPushButton(tr("Cấu hình"), m_deviceLogTable);
        actionBtn->setCursor(Qt::PointingHandCursor);
        actionBtn->setStyleSheet(QStringLiteral(
            "background: #1e293b; color: #f59e0b; border: 1px solid #334155; "
            "border-radius: 4px; font-size: 9px; font-weight: 700; padding: 2px 8px;"));
        connect(actionBtn, &QPushButton::clicked, this, [this, dev] {
            openDeviceConfigDialog(dev);
        });
        m_deviceLogTable->setCellWidget(rowIdx, 5, actionBtn);

        rowIdx++;
    }

    if (m_statTotalDevices) m_statTotalDevices->setText(QString::number(m_ownedDevices.size()));
    if (m_statOnlineDevices) m_statOnlineDevices->setText(QString::number(onlineCount));
    if (m_statLinkedUsers) m_statLinkedUsers->setText(QString::number(uniqueUsers.size()));
}

void DeviceManagementPage::openDeviceConfigDialog(const QJsonObject &device)
{
    const QString deviceId = device.value(QStringLiteral("device_id")).toString();
    const QString name = device.value(QStringLiteral("name")).toString(deviceId);
    const QString addedBy = device.value(QStringLiteral("added_by")).toString();
    const bool isOwner = (m_isAdmin || m_currentUsername.isEmpty() || addedBy.isEmpty() ||
                          addedBy.compare(m_currentUsername, Qt::CaseInsensitive) == 0);

    const QJsonObject saved = device.value(QStringLiteral("config")).toObject();
    const QJsonObject savedThresholds = saved.value(QStringLiteral("thresholds")).toObject();
    const QJsonObject luxCfg = savedThresholds.value(QStringLiteral("lux")).toObject();

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Cấu Hình Ngưỡng Ánh Sáng & Thiết Bị"));
    dlg.setModal(true);
    dlg.setFixedWidth(460);
    dlg.setStyleSheet(QStringLiteral(
        "QDialog { background-color: #0b1329; color: #f8fafc; font-family: 'Segoe UI', sans-serif; }"
        "QLabel { color: #e2e8f0; font-size: 10px; font-weight: 600; }"
        "QSpinBox, QDoubleSpinBox { background: #1e293b; color: #fbbf24; border: 1px solid #334155; "
        "border-radius: 6px; padding: 4px 8px; font-size: 11px; font-weight: 700; }"
        "QCheckBox { color: #f8fafc; font-size: 10px; font-weight: 700; }"
    ));

    auto *root = new QVBoxLayout(&dlg);
    root->setContentsMargins(18, 16, 18, 16);
    root->setSpacing(12);

    // Header
    auto *head = new QHBoxLayout;
    auto *icon = new QLabel(QStringLiteral("LIGHT"), &dlg);
    icon->setStyleSheet(QStringLiteral("font-size: 10px; font-weight: 900; color: #fbbf24; background: #1e293b; border-radius: 6px; padding: 4px 8px;"));
    head->addWidget(icon);

    auto *titles = new QVBoxLayout;
    titles->setSpacing(1);
    auto *t = new QLabel(name, &dlg);
    t->setStyleSheet(QStringLiteral("font-size: 13px; font-weight: 800; color: #f8fafc;"));
    auto *sub = new QLabel(tr("ID: %1  •  Chủ sở hữu: %2").arg(deviceId, addedBy.isEmpty() ? tr("Chưa gán") : addedBy), &dlg);
    sub->setStyleSheet(QStringLiteral("font-size: 9px; color: #94a3b8;"));
    titles->addWidget(t);
    titles->addWidget(sub);
    head->addLayout(titles, 1);
    root->addLayout(head);

    auto *line = new QFrame(&dlg);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QStringLiteral("color: #1e293b;"));
    root->addWidget(line);

    // Form
    auto *form = new QFormLayout;
    form->setSpacing(10);

    auto *luxMin = new QDoubleSpinBox(&dlg);
    luxMin->setRange(0.0, 5000.0);
    luxMin->setDecimals(1);
    luxMin->setValue(luxCfg.value(QStringLiteral("min")).toDouble(50.0));
    luxMin->setSuffix(tr(" Lux (Bật đèn khi tối)"));
    form->addRow(tr("Ngưỡng tối thiểu:"), luxMin);

    auto *luxMax = new QDoubleSpinBox(&dlg);
    luxMax->setRange(0.0, 5000.0);
    luxMax->setDecimals(1);
    luxMax->setValue(luxCfg.value(QStringLiteral("max")).toDouble(300.0));
    luxMax->setSuffix(tr(" Lux (Tắt đèn khi đủ sáng)"));
    form->addRow(tr("Ngưỡng tối đa:"), luxMax);

    auto *interval = new QSpinBox(&dlg);
    interval->setRange(1, 300);
    interval->setValue(saved.value(QStringLiteral("sampling_interval_ms")).toInt(2000) / 1000);
    interval->setSuffix(tr(" giây"));
    form->addRow(tr("Chu kỳ đọc cảm biến:"), interval);

    auto *autoCheck = new QCheckBox(tr("Kích hoạt chế độ điều khiển tự động"), &dlg);
    autoCheck->setChecked(saved.value(QStringLiteral("auto_mode")).toBool(true));
    form->addRow(autoCheck);

    root->addLayout(form);

    // Bottom Action Buttons
    auto *bottom = new QHBoxLayout;
    bottom->setSpacing(8);

    auto *delBtn = new QPushButton(tr("Xóa thiết bị"), &dlg);
    delBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #7f1d1d; color: #fecaca; border: 1px solid #991b1b; "
        "border-radius: 6px; font-size: 10px; font-weight: 700; padding: 6px 12px; }"
        "QPushButton:hover { background: #991b1b; }"
    ));
    delBtn->setEnabled(isOwner);
    connect(delBtn, &QPushButton::clicked, &dlg, [this, deviceId, &dlg] {
        if (QMessageBox::question(&dlg, tr("Xác nhận"),
                                  tr("Bạn có chắc chắn muốn xóa thiết bị '%1' khỏi tài khoản?").arg(deviceId))
            == QMessageBox::Yes) {
            emit releaseDeviceRequested(deviceId);
            dlg.accept();
        }
    });
    bottom->addWidget(delBtn);
    bottom->addStretch();

    auto *cancelBtn = new QPushButton(tr("Đóng"), &dlg);
    cancelBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #1e293b; color: #94a3b8; border-radius: 6px; font-size: 10px; font-weight: 700; padding: 6px 14px; }"));
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    bottom->addWidget(cancelBtn);

    auto *saveBtn = new QPushButton(tr("Lưu & Gửi Thiết Bị"), &dlg);
    saveBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #f59e0b; color: #020617; border-radius: 6px; font-size: 10px; font-weight: 800; padding: 6px 16px; }"
        "QPushButton:hover { background: #d97706; }"
    ));
    saveBtn->setEnabled(isOwner);
    connect(saveBtn, &QPushButton::clicked, &dlg, [this, deviceId, luxMin, luxMax, interval, autoCheck, &dlg] {
        QJsonObject thresholds;
        QJsonObject luxObj;
        luxObj.insert(QStringLiteral("min"), luxMin->value());
        luxObj.insert(QStringLiteral("max"), luxMax->value());
        thresholds.insert(QStringLiteral("lux"), luxObj);

        const QJsonObject config{
            {QStringLiteral("sampling_interval_ms"), interval->value() * 1000},
            {QStringLiteral("thresholds"), thresholds},
            {QStringLiteral("auto_mode"), autoCheck->isChecked()}
        };
        emit deviceConfigRequested(deviceId, config);
        dlg.accept();
    });
    bottom->addWidget(saveBtn);

    root->addLayout(bottom);
    dlg.exec();
}

QString DeviceManagementPage::deviceIcon(const QString &type)
{
    Q_UNUSED(type);
    return QStringLiteral("LIGHT");
}

QString DeviceManagementPage::deviceTypeName(const QString &type)
{
    Q_UNUSED(type);
    return tr("Hệ thống chiếu sáng & Cảm biến phòng");
}

QString DeviceManagementPage::metricsSummary(const QJsonObject &metrics)
{
    QStringList values;
    if (metrics.contains(QStringLiteral("light_lux")))
        values << tr("Ánh sáng: %1 Lux").arg(metrics.value(QStringLiteral("light_lux")).toDouble(), 0, 'f', 1);
    else if (metrics.contains(QStringLiteral("lux")))
        values << tr("Ánh sáng: %1 Lux").arg(metrics.value(QStringLiteral("lux")).toDouble(), 0, 'f', 1);

    if (metrics.contains(QStringLiteral("motion_detected"))) {
        const bool motion = metrics.value(QStringLiteral("motion_detected")).toInt() > 0;
        values << (motion ? tr("PIR: Có người") : tr("PIR: Trống"));
    }

    if (metrics.contains(QStringLiteral("relay_on"))) {
        const bool r = metrics.value(QStringLiteral("relay_on")).toBool();
        values << (r ? tr("Đèn: BẬT") : tr("Đèn: TẮT"));
    } else if (metrics.contains(QStringLiteral("relay"))) {
        const bool r = metrics.value(QStringLiteral("relay")).toInt() > 0;
        values << (r ? tr("Đèn: BẬT") : tr("Đèn: TẮT"));
    }

    if (metrics.contains(QStringLiteral("temperature_c")))
        values << tr("Nhiệt độ: %1 °C").arg(metrics.value(QStringLiteral("temperature_c")).toDouble(), 0, 'f', 1);

    return values.isEmpty() ? tr("Chờ dữ liệu cảm biến...") : values.join(QStringLiteral("  •  "));
}
