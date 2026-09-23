#include "UserManagementPage.h"
#include "VirtualKeyboard.h"
#include "ui_UserManagementPage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>

UserManagementPage::UserManagementPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::UserManagementPage)
{
    ui->setupUi(this);
    setStyleSheet(
        "QWidget#UserManagementPage { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #130f30, stop:1 #09071c); "
        "  color: #ecf2ff; "
        "  font-family: sans-serif; "
        "}"
    );

    setupCustomUI();
}

UserManagementPage::~UserManagementPage()
{
    delete ui;
}

void UserManagementPage::setUsers(const QJsonArray &users)
{
    m_users = users;

    int total = users.size();
    int admins = 0;
    int regularUsers = 0;

    for (const auto &val : users) {
        const auto u = val.toObject();
        if (u.value(QStringLiteral("role")).toString() == QStringLiteral("admin"))
            admins++;
        else
            regularUsers++;
    }

    if (m_filterAllBtn) m_filterAllBtn->setText(QStringLiteral("Tất Cả (%1)").arg(total));
    if (m_filterAdminBtn) m_filterAdminBtn->setText(QStringLiteral("Admin (%1)").arg(admins));
    if (m_filterUserBtn) m_filterUserBtn->setText(QStringLiteral("Người Dùng (%1)").arg(regularUsers));

    renderUserGrid();
}

void UserManagementPage::setLoginHistory(const QJsonArray &history)
{
    m_loginHistory = history;
    renderLoginHistory();
}

void UserManagementPage::setAuditLogs(const QJsonArray &logs)
{
    m_auditLogs = logs;
    renderAuditLogs();
}

void UserManagementPage::setCurrentUsername(const QString &username)
{
    m_currentUsername = username;
    if (m_auditSummaryLabel) {
        if (!m_adminEnabled) {
            m_auditSummaryLabel->setText(
                QStringLiteral("🔒 <b>Chế độ người dùng:</b> Chỉ hiển thị các thao tác do tài khoản <b>%1</b> thực hiện")
                    .arg(m_currentUsername.isEmpty() ? QStringLiteral("bạn") : m_currentUsername));
        }
    }
}

void UserManagementPage::setAdminEnabled(bool enabled)
{
    m_adminEnabled = enabled;

    m_tabUsersBtn->setVisible(enabled);
    m_tabLoginBtn->setVisible(enabled);
    m_tabAuditBtn->setVisible(true);

    if (!enabled) {
        // Normal user only sees Audit Logs of their own actions
        m_stack->setCurrentIndex(2);
        m_tabAuditBtn->setChecked(true);
        m_addUserBtn->setVisible(false);
        if (m_auditSummaryLabel) {
            m_auditSummaryLabel->setText(
                QStringLiteral("🔒 <b>Chế độ người dùng:</b> Chỉ hiển thị các thao tác do tài khoản <b>%1</b> thực hiện")
                    .arg(m_currentUsername.isEmpty() ? QStringLiteral("bạn") : m_currentUsername));
        }
    } else {
        m_tabUsersBtn->setChecked(m_stack->currentIndex() == 0);
        m_tabLoginBtn->setChecked(m_stack->currentIndex() == 1);
        m_tabAuditBtn->setChecked(m_stack->currentIndex() == 2);
        m_addUserBtn->setVisible(m_stack->currentIndex() == 0);
        if (m_auditSummaryLabel) {
            m_auditSummaryLabel->setText(
                QStringLiteral("👑 <b>Quyền Quản trị viên:</b> Hiển thị lịch sử điều khiển thiết bị & thao tác toàn hệ thống"));
        }
    }
}

void UserManagementPage::setupCustomUI()
{
    while (QLayoutItem *item = ui->verticalLayout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    auto *mainLayout = ui->verticalLayout;
    mainLayout->setContentsMargins(10, 8, 10, 8);
    mainLayout->setSpacing(6);

    // ==========================================
    // TOP NAVIGATION & SUBTABS BAR
    // ==========================================
    auto *topBar = new QHBoxLayout;
    topBar->setSpacing(8);
    topBar->setContentsMargins(0, 0, 0, 6);

    auto *backBtn = new QPushButton(tr("← Giám sát"), this);
    backBtn->setObjectName(QStringLiteral("usersBackButton"));
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setToolTip(tr("Quay lại màn hình giám sát SCADA"));
    backBtn->setStyleSheet(
        "QPushButton { background: #1c1642; color: #94a3b8; border: 1px solid #2e2468; border-radius: 6px; font-size: 11px; font-weight: 700; padding: 5px 10px; } "
        "QPushButton:hover { background: #261e5a; color: #ffffff; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &UserManagementPage::backToDashboardRequested);
    topBar->addWidget(backBtn);

    auto makeNavBtn = [](const QString &text) {
        auto *btn = new QPushButton(text);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setCheckable(true);
        btn->setStyleSheet(
            "QPushButton { "
            "  background: #17123a; "
            "  color: #94a3b8; "
            "  border: 1px solid #2b2260; "
            "  border-radius: 6px; "
            "  font-size: 11px; "
            "  font-weight: 700; "
            "  padding: 5px 14px; "
            "} "
            "QPushButton:hover { background: #221a54; color: #ffffff; border-color: #3b82f6; } "
            "QPushButton:checked { background: #2563eb; color: #ffffff; border-color: #60a5fa; font-weight: 800; }"
        );
        return btn;
    };

    m_tabUsersBtn = makeNavBtn(QStringLiteral("Tài khoản"));
    m_tabLoginBtn = makeNavBtn(QStringLiteral("Lịch sử đăng nhập"));
    m_tabAuditBtn = makeNavBtn(QStringLiteral("Lịch sử điều khiển"));

    topBar->addWidget(m_tabUsersBtn);
    topBar->addWidget(m_tabLoginBtn);
    topBar->addWidget(m_tabAuditBtn);
    topBar->addStretch();

    // + Add User Button
    m_addUserBtn = new QPushButton(QStringLiteral("+ Tạo tài khoản"));
    m_addUserBtn->setCursor(Qt::PointingHandCursor);
    m_addUserBtn->setStyleSheet(
        "QPushButton { "
        "  background: #10b981; "
        "  color: #ffffff; "
        "  border: none; "
        "  border-radius: 6px; "
        "  font-size: 11px; "
        "  font-weight: 800; "
        "  padding: 5px 14px; "
        "} "
        "QPushButton:hover { background: #059669; } "
        "QPushButton:pressed { background: #047857; }"
    );
    connect(m_addUserBtn, &QPushButton::clicked, this, [this] { openEditDialog(); });
    topBar->addWidget(m_addUserBtn);

    // Refresh Button
    m_refreshBtn = new QPushButton(QStringLiteral("Làm mới"));
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setStyleSheet(
        "QPushButton { "
        "  background: #1e1b4b; "
        "  color: #38bdf8; "
        "  border: 1px solid #312e81; "
        "  border-radius: 6px; "
        "  font-size: 11px; "
        "  font-weight: 800; "
        "  padding: 5px 12px; "
        "} "
        "QPushButton:hover { background: #2e287a; color: #ffffff; }"
    );
    connect(m_refreshBtn, &QPushButton::clicked, this, [this] {
        if (m_stack->currentIndex() == 0) {
            emit refreshRequested();
        } else if (m_stack->currentIndex() == 1) {
            emit requestLoginHistoryRequested();
        } else if (m_stack->currentIndex() == 2) {
            emit requestAuditLogsRequested();
        }
    });
    topBar->addWidget(m_refreshBtn);

    mainLayout->addLayout(topBar);

    // ==========================================
    // STACKED WIDGET (3 PAGES)
    // ==========================================
    m_stack = new QStackedWidget(this);
    m_stack->setStyleSheet("background: transparent;");

    // ------------------------------------------
    // PAGE 0: USER MANAGEMENT
    // ------------------------------------------
    auto *usersPage = new QWidget;
    auto *uLayout = new QVBoxLayout(usersPage);
    uLayout->setContentsMargins(0, 4, 0, 0);
    uLayout->setSpacing(6);

    auto *filterBar = new QHBoxLayout;
    filterBar->setSpacing(6);

    auto makeFilterBtn = [&](const QString &label, const QString &mode) {
        auto *btn = new QPushButton(label);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setCheckable(true);
        btn->setStyleSheet(
            "QPushButton { background: #1c1642; color: #94a3b8; border: 1px solid #2e2468; border-radius: 5px; font-size: 10px; font-weight: 700; padding: 3px 8px; } "
            "QPushButton:hover { background: #261e5a; color: #ffffff; } "
            "QPushButton:checked { background: #3b82f6; color: #ffffff; border-color: #60a5fa; font-weight: 800; }"
        );
        connect(btn, &QPushButton::clicked, this, [this, mode] {
            m_currentFilter = mode;
            m_filterAllBtn->setChecked(mode == QStringLiteral("all"));
            m_filterAdminBtn->setChecked(mode == QStringLiteral("admin"));
            m_filterUserBtn->setChecked(mode == QStringLiteral("user"));
            renderUserGrid();
        });
        return btn;
    };

    m_filterAllBtn = makeFilterBtn(QStringLiteral("Tất Cả (0)"), QStringLiteral("all"));
    m_filterAdminBtn = makeFilterBtn(QStringLiteral("Admin (0)"), QStringLiteral("admin"));
    m_filterUserBtn = makeFilterBtn(QStringLiteral("Người Dùng (0)"), QStringLiteral("user"));
    m_filterAllBtn->setChecked(true);

    filterBar->addWidget(m_filterAllBtn);
    filterBar->addWidget(m_filterAdminBtn);
    filterBar->addWidget(m_filterUserBtn);
    filterBar->addStretch();
    uLayout->addLayout(filterBar);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("background: transparent; border: none;");

    auto *container = new QWidget;
    container->setStyleSheet("background: transparent;");
    m_gridLayout = new QGridLayout(container);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setHorizontalSpacing(8);
    m_gridLayout->setVerticalSpacing(8);

    m_emptyLabel = new QLabel(QStringLiteral("Đang tải danh sách tài khoản..."));
    m_emptyLabel->setStyleSheet("color: #64748b; font-style: italic; font-size: 11px; padding: 30px;");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_gridLayout->addWidget(m_emptyLabel, 0, 0, 1, 2);

    scroll->setWidget(container);
    uLayout->addWidget(scroll, 1);
    m_stack->addWidget(usersPage);

    // ------------------------------------------
    // PAGE 1: LOGIN HISTORY (Admin Only)
    // ------------------------------------------
    auto *loginPage = new QWidget;
    auto *lLayout = new QVBoxLayout(loginPage);
    lLayout->setContentsMargins(0, 4, 0, 0);
    lLayout->setSpacing(6);

    auto *loginTopRow = new QHBoxLayout;
    m_loginSummaryLabel = new QLabel(QStringLiteral("Đang tải nhật ký đăng nhập..."));
    m_loginSummaryLabel->setStyleSheet("color: #94a3b8; font-size: 11px;");
    loginTopRow->addWidget(m_loginSummaryLabel);
    loginTopRow->addStretch();
    lLayout->addLayout(loginTopRow);

    m_loginTable = new QTableWidget(0, 6, loginPage);
    m_loginTable->setHorizontalHeaderLabels({
        QStringLiteral("#"),
        QStringLiteral("Thời Gian"),
        QStringLiteral("Tài Khoản"),
        QStringLiteral("Vai Trò"),
        QStringLiteral("Địa Chỉ IP"),
        QStringLiteral("Trạng Thái Đăng Nhập")
    });
    m_loginTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_loginTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_loginTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_loginTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_loginTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_loginTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_loginTable->verticalHeader()->hide();
    m_loginTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_loginTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_loginTable->setAlternatingRowColors(true);
    m_loginTable->setStyleSheet(
        "QTableWidget { background-color: rgba(18, 14, 46, 0.9); border: 1px solid #2b2260; border-radius: 8px; gridline-color: #241c52; color: #ecf2ff; font-size: 11px; alternate-background-color: rgba(26, 20, 64, 0.6); } "
        "QHeaderView::section { background-color: #17113b; color: #94a3b8; font-weight: 800; font-size: 10px; padding: 6px; border: none; border-bottom: 2px solid #3b82f6; } "
        "QTableWidget::item { padding: 4px 8px; border-bottom: 1px solid rgba(43, 34, 96, 0.4); } "
        "QTableWidget::item:selected { background-color: #2563eb; color: #ffffff; }"
    );
    lLayout->addWidget(m_loginTable, 1);
    m_stack->addWidget(loginPage);

    // ------------------------------------------
    // PAGE 2: AUDIT LOGS & DEVICE CONTROLS
    // ------------------------------------------
    auto *auditPage = new QWidget;
    auto *aLayout = new QVBoxLayout(auditPage);
    aLayout->setContentsMargins(0, 4, 0, 0);
    aLayout->setSpacing(6);

    auto *auditTopRow = new QHBoxLayout;
    m_auditSummaryLabel = new QLabel(QStringLiteral("Đang tải lịch sử điều khiển & thao tác..."));
    m_auditSummaryLabel->setStyleSheet("color: #94a3b8; font-size: 11px;");
    auditTopRow->addWidget(m_auditSummaryLabel);
    auditTopRow->addStretch();

    auto *searchIcon = new QLabel(QStringLiteral("🔍"));
    searchIcon->setStyleSheet("background: transparent; font-size: 11px;");
    auditTopRow->addWidget(searchIcon);

    m_auditSearchEdit = new QLineEdit;
    m_auditSearchEdit->setPlaceholderText(QStringLiteral("Tìm kiếm thao tác, thiết bị..."));
    m_auditSearchEdit->setStyleSheet(
        "QLineEdit { background-color: #171338; color: #ffffff; border: 1px solid #2b235c; border-radius: 5px; padding: 3px 8px; font-size: 10px; width: 180px; } "
        "QLineEdit:focus { border: 1px solid #38bdf8; background-color: #1f1a4a; }"
    );
    VirtualKeyboardDialog::attachToLineEdit(m_auditSearchEdit, tr("Tìm kiếm thao tác"));
    connect(m_auditSearchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_auditFilterText = text.trimmed();
        renderAuditLogs();
    });
    auditTopRow->addWidget(m_auditSearchEdit);
    aLayout->addLayout(auditTopRow);

    m_auditTable = new QTableWidget(0, 6, auditPage);
    m_auditTable->setHorizontalHeaderLabels({
        QStringLiteral("#"),
        QStringLiteral("Thời Gian"),
        QStringLiteral("Tài Khoản"),
        QStringLiteral("Hành Động"),
        QStringLiteral("Thiết Bị / Mục Tiêu"),
        QStringLiteral("Chi Tiết Thao Tác")
    });
    m_auditTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_auditTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_auditTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_auditTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_auditTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_auditTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_auditTable->verticalHeader()->hide();
    m_auditTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_auditTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_auditTable->setAlternatingRowColors(true);
    m_auditTable->setStyleSheet(
        "QTableWidget { background-color: rgba(18, 14, 46, 0.9); border: 1px solid #2b2260; border-radius: 8px; gridline-color: #241c52; color: #ecf2ff; font-size: 11px; alternate-background-color: rgba(26, 20, 64, 0.6); } "
        "QHeaderView::section { background-color: #17113b; color: #94a3b8; font-weight: 800; font-size: 10px; padding: 6px; border: none; border-bottom: 2px solid #10b981; } "
        "QTableWidget::item { padding: 4px 8px; border-bottom: 1px solid rgba(43, 34, 96, 0.4); } "
        "QTableWidget::item:selected { background-color: #2563eb; color: #ffffff; }"
    );
    aLayout->addWidget(m_auditTable, 1);
    m_stack->addWidget(auditPage);

    mainLayout->addWidget(m_stack, 1);

    // ==========================================
    // TAB SWITCHING CONNECTIONS
    // ==========================================
    connect(m_tabUsersBtn, &QPushButton::clicked, this, [this] {
        m_stack->setCurrentIndex(0);
        m_tabUsersBtn->setChecked(true);
        m_tabLoginBtn->setChecked(false);
        m_tabAuditBtn->setChecked(false);
        m_addUserBtn->setVisible(m_adminEnabled);
        emit refreshRequested();
    });

    connect(m_tabLoginBtn, &QPushButton::clicked, this, [this] {
        m_stack->setCurrentIndex(1);
        m_tabUsersBtn->setChecked(false);
        m_tabLoginBtn->setChecked(true);
        m_tabAuditBtn->setChecked(false);
        m_addUserBtn->setVisible(false);
        emit requestLoginHistoryRequested();
    });

    connect(m_tabAuditBtn, &QPushButton::clicked, this, [this] {
        m_stack->setCurrentIndex(2);
        m_tabUsersBtn->setChecked(false);
        m_tabLoginBtn->setChecked(false);
        m_tabAuditBtn->setChecked(true);
        m_addUserBtn->setVisible(false);
        emit requestAuditLogsRequested();
    });

    m_tabUsersBtn->setChecked(true);
}

void UserManagementPage::renderUserGrid()
{
    // Clear old grid items
    QLayoutItem *item;
    while ((item = m_gridLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            if (item->widget() != m_emptyLabel)
                delete item->widget();
        }
        delete item;
    }

    QVector<QJsonObject> filteredUsers;
    for (const auto &val : m_users) {
        const auto u = val.toObject();
        const QString role = u.value(QStringLiteral("role")).toString(QStringLiteral("user"));

        if (m_currentFilter == QStringLiteral("admin") && role != QStringLiteral("admin"))
            continue;
        if (m_currentFilter == QStringLiteral("user") && role == QStringLiteral("admin"))
            continue;

        filteredUsers.append(u);
    }

    if (filteredUsers.isEmpty()) {
        m_emptyLabel->setText(QStringLiteral("Không tìm thấy tài khoản nào trong bộ lọc này."));
        m_emptyLabel->show();
        m_gridLayout->addWidget(m_emptyLabel, 0, 0, 1, 2);
        return;
    }

    m_emptyLabel->hide();

    // 2-Column compact grid for 7-inch displays
    int row = 0;
    int col = 0;

    for (const auto &u : filteredUsers) {
        const QString username = u.value(QStringLiteral("username")).toString();
        const QString role = u.value(QStringLiteral("role")).toString(QStringLiteral("user"));
        const bool isAdmin = (role == QStringLiteral("admin"));
        const bool isEnabled = u.value(QStringLiteral("enabled")).toBool(true);

        QStringList devices;
        for (const auto &d : u.value(QStringLiteral("device_ids")).toArray()) {
            devices.append(d.toString());
        }

        auto *card = new QFrame;
        card->setStyleSheet(
            "QFrame { "
            "  background-color: rgba(26, 21, 58, 0.9); "
            "  border: 1px solid #332a68; "
            "  border-radius: 8px; "
            "} "
            "QFrame:hover { "
            "  border-color: #38bdf8; "
            "  background-color: rgba(33, 27, 74, 0.95); "
            "}"
        );

        auto *cLayout = new QVBoxLayout(card);
        cLayout->setContentsMargins(12, 10, 12, 10);
        cLayout->setSpacing(8);

        // --- ROW 1: Header (Avatar + Role Pill + Status Badge) ---
        auto *headerRow = new QHBoxLayout;
        headerRow->setSpacing(8);

        const QString initial = username.isEmpty() ? QStringLiteral("U") : username.left(1).toUpper();
        auto *avatarLbl = new QLabel(initial);
        avatarLbl->setFixedSize(28, 28);
        avatarLbl->setAlignment(Qt::AlignCenter);
        avatarLbl->setStyleSheet(QStringLiteral(
            "QLabel { background: %1; color: %2; border-radius: 14px; font-size: 12px; font-weight: 900; border: 1px solid %2; }")
            .arg(isAdmin ? QStringLiteral("rgba(251, 191, 36, 0.25)") : QStringLiteral("rgba(56, 189, 248, 0.25)"))
            .arg(isAdmin ? QStringLiteral("#fbbf24") : QStringLiteral("#38bdf8"))
        );
        headerRow->addWidget(avatarLbl);

        auto *nameLbl = new QLabel(username);
        nameLbl->setStyleSheet("color: #ffffff; font-size: 13px; font-weight: 800; background: transparent;");
        headerRow->addWidget(nameLbl);

        auto *roleBadge = new QLabel(isAdmin ? QStringLiteral("ADMIN") : QStringLiteral("USER"));
        roleBadge->setStyleSheet(QStringLiteral(
            "background-color: %1; color: %2; font-size: 9px; font-weight: 800; border-radius: 4px; padding: 2px 8px;")
            .arg(isAdmin ? QStringLiteral("#451a03") : QStringLiteral("#082f49"))
            .arg(isAdmin ? QStringLiteral("#fbbf24") : QStringLiteral("#38bdf8"))
        );
        headerRow->addWidget(roleBadge);

        headerRow->addStretch();

        auto *statusBadge = new QLabel(isEnabled ? QStringLiteral("● Hoạt động") : QStringLiteral("● Đã khóa"));
        statusBadge->setStyleSheet(QStringLiteral(
            "color: %1; font-size: 10px; font-weight: 700; background: transparent;")
            .arg(isEnabled ? QStringLiteral("#10b981") : QStringLiteral("#ef4444"))
        );
        headerRow->addWidget(statusBadge);

        cLayout->addLayout(headerRow);

        // --- ROW 2: Devices section ---
        auto *devBox = new QFrame;
        devBox->setStyleSheet("background-color: rgba(15, 12, 36, 0.6); border-radius: 4px; padding: 3px;");
        auto *devBoxLay = new QVBoxLayout(devBox);
        devBoxLay->setContentsMargins(8, 6, 8, 6);
        devBoxLay->setSpacing(4);

        auto *devTitle = new QLabel(QStringLiteral("Thiết bị giám sát phụ trách (%1):").arg(devices.size()));
        devTitle->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: 700; background: transparent;");
        devBoxLay->addWidget(devTitle);

        if (devices.isEmpty()) {
            auto *noDev = new QLabel(QStringLiteral("Chưa ghép nối thiết bị nào"));
            noDev->setStyleSheet("color: #64748b; font-size: 9px; font-style: italic; background: transparent;");
            devBoxLay->addWidget(noDev);
        } else {
            for (const auto &devId : devices) {
                auto *dRow = new QHBoxLayout;
                dRow->setContentsMargins(0, 0, 0, 0);
                auto *dLbl = new QLabel(QStringLiteral("• %1").arg(devId));
                dLbl->setStyleSheet("color: #38bdf8; font-size: 10px; font-weight: 600; background: transparent;");
                dRow->addWidget(dLbl);
                dRow->addStretch();

                auto *unBtn = new QPushButton(QStringLiteral("Gỡ"));
                unBtn->setCursor(Qt::PointingHandCursor);
                unBtn->setStyleSheet("QPushButton { background: #3b1424; color: #f87171; border: 1px solid #7f1d1d; border-radius: 4px; font-size: 9px; font-weight: 700; padding: 2px 8px; } QPushButton:hover { background: #dc2626; color: #fff; }");
                connect(unBtn, &QPushButton::clicked, this, [this, username, devId] {
                    emit releaseUserDeviceRequested(username, devId);
                });
                dRow->addWidget(unBtn);
                devBoxLay->addLayout(dRow);
            }
        }
        cLayout->addWidget(devBox);

        // --- ROW 3: Card Actions (Edit & Delete) ---
        auto *actRow = new QHBoxLayout;
        actRow->setContentsMargins(0, 0, 0, 0);
        actRow->setSpacing(8);

        auto *editBtn = new QPushButton(QStringLiteral("Sửa"));
        editBtn->setCursor(Qt::PointingHandCursor);
        editBtn->setStyleSheet(
            "QPushButton { "
            "  background: #2563eb; "
            "  color: #ffffff; "
            "  border: none; "
            "  border-radius: 4px; "
            "  font-size: 10px; "
            "  font-weight: 700; "
            "  padding: 4px 12px; "
            "} "
            "QPushButton:hover { background: #1d4ed8; }"
        );
        connect(editBtn, &QPushButton::clicked, this, [this, u] { openEditDialog(u); });
        actRow->addWidget(editBtn);

        if (username != QStringLiteral("admin")) {
            auto *delBtn = new QPushButton(QStringLiteral("Xóa"));
            delBtn->setCursor(Qt::PointingHandCursor);
            delBtn->setStyleSheet(
                "QPushButton { "
                "  background: #7f1d1d; "
                "  color: #fecaca; "
                "  border: 1px solid #991b1b; "
                "  border-radius: 4px; "
                "  font-size: 10px; "
                "  font-weight: 700; "
                "  padding: 4px 12px; "
                "} "
                "QPushButton:hover { background: #dc2626; color: #ffffff; }"
            );
            connect(delBtn, &QPushButton::clicked, this, [this, u] { confirmDeleteUser(u); });
            actRow->addWidget(delBtn);
        }

        actRow->addStretch();
        cLayout->addLayout(actRow);

        m_gridLayout->addWidget(card, row, col);
        col++;
        if (col >= 2) {
            col = 0;
            row++;
        }
    }
}

void UserManagementPage::renderLoginHistory()
{
    if (!m_loginTable) return;

    int total = m_loginHistory.size();
    int successCount = 0;
    int failCount = 0;

    for (const auto &val : m_loginHistory) {
        const auto item = val.toObject();
        if (item.value(QStringLiteral("status")).toString().toLower() == QStringLiteral("success"))
            successCount++;
        else
            failCount++;
    }

    if (m_loginSummaryLabel) {
        m_loginSummaryLabel->setText(
            QStringLiteral("📊 <b>Tổng số:</b> %1 lượt | <span style='color:#10b981; font-weight:700;'>✓ Thành công: %2</span> | <span style='color:#ef4444; font-weight:700;'>✕ Thất bại: %3</span>")
                .arg(total).arg(successCount).arg(failCount));
    }

    m_loginTable->setRowCount(total);

    for (int i = 0; i < total; ++i) {
        const auto item = m_loginHistory.at(i).toObject();
        const QString timeStr = item.value(QStringLiteral("created_at")).toString();
        const QString user = item.value(QStringLiteral("username")).toString();
        const QString role = item.value(QStringLiteral("role")).toString();
        const QString ip = item.value(QStringLiteral("ip_address")).toString();
        const QString status = item.value(QStringLiteral("status")).toString();
        const bool isSuccess = (status.toLower() == QStringLiteral("success"));

        // Column 0: Index
        auto *itIdx = new QTableWidgetItem(QString::number(i + 1));
        itIdx->setTextAlignment(Qt::AlignCenter);

        // Column 1: Time
        auto *itTime = new QTableWidgetItem(timeStr);
        itTime->setTextAlignment(Qt::AlignCenter);

        // Column 2: Username
        auto *itUser = new QTableWidgetItem(user);
        itUser->setTextAlignment(Qt::AlignCenter);
        itUser->setForeground(QBrush(QColor(QStringLiteral("#38bdf8"))));
        QFont fontUser = itUser->font();
        fontUser.setBold(true);
        itUser->setFont(fontUser);

        // Column 3: Role
        auto *itRole = new QTableWidgetItem(role.toUpper());
        itRole->setTextAlignment(Qt::AlignCenter);
        if (role == QStringLiteral("admin")) {
            itRole->setForeground(QBrush(QColor(QStringLiteral("#fbbf24"))));
        } else {
            itRole->setForeground(QBrush(QColor(QStringLiteral("#94a3b8"))));
        }

        // Column 4: IP Address
        auto *itIp = new QTableWidgetItem(ip);
        itIp->setTextAlignment(Qt::AlignCenter);

        // Column 5: Status Badge
        auto *itStatus = new QTableWidgetItem(isSuccess ? QStringLiteral("✓ THÀNH CÔNG") : QStringLiteral("✕ THẤT BẠI"));
        itStatus->setTextAlignment(Qt::AlignCenter);
        QFont f = itStatus->font();
        f.setBold(true);
        itStatus->setFont(f);
        if (isSuccess) {
            itStatus->setForeground(QBrush(QColor(QStringLiteral("#10b981"))));
        } else {
            itStatus->setForeground(QBrush(QColor(QStringLiteral("#ef4444"))));
        }

        m_loginTable->setItem(i, 0, itIdx);
        m_loginTable->setItem(i, 1, itTime);
        m_loginTable->setItem(i, 2, itUser);
        m_loginTable->setItem(i, 3, itRole);
        m_loginTable->setItem(i, 4, itIp);
        m_loginTable->setItem(i, 5, itStatus);
        m_loginTable->setRowHeight(i, 32);
    }
}

void UserManagementPage::renderAuditLogs()
{
    if (!m_auditTable) return;

    QVector<QJsonObject> filtered;
    for (const auto &val : m_auditLogs) {
        const auto obj = val.toObject();
        if (m_auditFilterText.isEmpty()) {
            filtered.append(obj);
        } else {
            const QString act = obj.value(QStringLiteral("action")).toString();
            const QString target = obj.value(QStringLiteral("target")).toString();
            const QString det = obj.value(QStringLiteral("details")).toString();
            const QString usr = obj.value(QStringLiteral("username")).toString();
            if (act.contains(m_auditFilterText, Qt::CaseInsensitive)
                || target.contains(m_auditFilterText, Qt::CaseInsensitive)
                || det.contains(m_auditFilterText, Qt::CaseInsensitive)
                || usr.contains(m_auditFilterText, Qt::CaseInsensitive)) {
                filtered.append(obj);
            }
        }
    }

    if (m_auditSummaryLabel) {
        if (m_adminEnabled) {
            m_auditSummaryLabel->setText(
                QStringLiteral("👑 <b>Nhật ký thao tác hệ thống:</b> %1 sự kiện (Toàn bộ tài khoản)")
                    .arg(filtered.size()));
        } else {
            m_auditSummaryLabel->setText(
                QStringLiteral("🔒 <b>Nhật ký cá nhân:</b> %1 sự kiện của tài khoản <b>%2</b>")
                    .arg(filtered.size())
                    .arg(m_currentUsername.isEmpty() ? QStringLiteral("bạn") : m_currentUsername));
        }
    }

    m_auditTable->setRowCount(filtered.size());

    for (int i = 0; i < filtered.size(); ++i) {
        const auto item = filtered.at(i);
        const QString timeStr = item.value(QStringLiteral("created_at")).toString();
        const QString user = item.value(QStringLiteral("username")).toString();
        const QString act = item.value(QStringLiteral("action")).toString();
        const QString target = item.value(QStringLiteral("target")).toString();
        const QString details = item.value(QStringLiteral("details")).toString();

        // Column 0: Index
        auto *itIdx = new QTableWidgetItem(QString::number(i + 1));
        itIdx->setTextAlignment(Qt::AlignCenter);

        // Column 1: Time
        auto *itTime = new QTableWidgetItem(timeStr);
        itTime->setTextAlignment(Qt::AlignCenter);

        // Column 2: Username
        auto *itUser = new QTableWidgetItem(user);
        itUser->setTextAlignment(Qt::AlignCenter);
        itUser->setForeground(QBrush(QColor(QStringLiteral("#38bdf8"))));
        QFont fu = itUser->font();
        fu.setBold(true);
        itUser->setFont(fu);

        // Column 3: Action
        auto *itAct = new QTableWidgetItem(act);
        itAct->setTextAlignment(Qt::AlignCenter);
        QFont fa = itAct->font();
        fa.setBold(true);
        itAct->setFont(fa);

        if (act.contains(QStringLiteral("RƠ-LE"), Qt::CaseInsensitive)) {
            itAct->setForeground(QBrush(QColor(QStringLiteral("#fb923c")))); // Orange
        } else if (act.contains(QStringLiteral("CẤU HÌNH"), Qt::CaseInsensitive)
                   || act.contains(QStringLiteral("NGƯỠNG"), Qt::CaseInsensitive)) {
            itAct->setForeground(QBrush(QColor(QStringLiteral("#38bdf8")))); // Cyan
        } else if (act.contains(QStringLiteral("GHÉP"), Qt::CaseInsensitive)) {
            itAct->setForeground(QBrush(QColor(QStringLiteral("#34d399")))); // Emerald
        } else if (act.contains(QStringLiteral("GỠ"), Qt::CaseInsensitive)
                   || act.contains(QStringLiteral("HỦY"), Qt::CaseInsensitive)
                   || act.contains(QStringLiteral("XÓA"), Qt::CaseInsensitive)) {
            itAct->setForeground(QBrush(QColor(QStringLiteral("#f87171")))); // Red
        } else if (act.contains(QStringLiteral("ĐĂNG NHẬP"), Qt::CaseInsensitive)) {
            itAct->setForeground(QBrush(QColor(QStringLiteral("#a78bfa")))); // Purple
        } else {
            itAct->setForeground(QBrush(QColor(QStringLiteral("#facc15")))); // Yellow
        }

        // Column 4: Target
        auto *itTarget = new QTableWidgetItem(target.isEmpty() ? QStringLiteral("—") : target);
        itTarget->setTextAlignment(Qt::AlignCenter);
        itTarget->setForeground(QBrush(QColor(QStringLiteral("#cbd5e1"))));

        // Column 5: Details
        auto *itDetails = new QTableWidgetItem(details);
        itDetails->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        itDetails->setForeground(QBrush(QColor(QStringLiteral("#e2e8f0"))));

        m_auditTable->setItem(i, 0, itIdx);
        m_auditTable->setItem(i, 1, itTime);
        m_auditTable->setItem(i, 2, itUser);
        m_auditTable->setItem(i, 3, itAct);
        m_auditTable->setItem(i, 4, itTarget);
        m_auditTable->setItem(i, 5, itDetails);
        m_auditTable->setRowHeight(i, 32);
    }
}

void UserManagementPage::openEditDialog(const QJsonObject &user)
{
    const bool isEdit = !user.isEmpty();
    const QString oldUser = user.value(QStringLiteral("username")).toString();

    QDialog dlg(this);
    dlg.setWindowTitle(isEdit ? QStringLiteral("Cập Nhật Tài Khoản") : QStringLiteral("Tạo Tài Khoản Mới"));
    dlg.setFixedSize(400, 250);
    dlg.setStyleSheet(
        "QDialog { background-color: #0d0a26; color: #ecf2ff; font-family: sans-serif; } "
        "QLabel { color: #cbd5e1; font-weight: 700; font-size: 11px; } "
        "QLineEdit, QComboBox { background-color: #171338; color: #ffffff; border: 1px solid #2b235c; border-radius: 5px; padding: 5px 8px; font-size: 11px; font-weight: 600; } "
        "QLineEdit:focus, QComboBox:focus { border: 1px solid #38bdf8; background-color: #1f1a4a; }"
    );

    auto *mainLayout = new QVBoxLayout(&dlg);
    mainLayout->setContentsMargins(16, 14, 16, 14);
    mainLayout->setSpacing(10);

    auto *titleLbl = new QLabel(isEdit ? QStringLiteral("Chỉnh Sửa: %1").arg(oldUser) : QStringLiteral("Tạo Tài Khoản Mới"));
    titleLbl->setStyleSheet("color: #38bdf8; font-size: 13px; font-weight: 800;");
    mainLayout->addWidget(titleLbl);

    auto *form = new QFormLayout;
    form->setSpacing(8);

    auto *uInput = new QLineEdit(&dlg);
    uInput->setText(oldUser);
    uInput->setPlaceholderText(QStringLiteral("Nhập tên đăng nhập..."));
    VirtualKeyboardDialog::attachToLineEdit(uInput, tr("Tên đăng nhập"));
    form->addRow(QStringLiteral("Tài khoản:"), uInput);

    auto *pInput = new QLineEdit(&dlg);
    pInput->setEchoMode(QLineEdit::Password);
    pInput->setPlaceholderText(isEdit ? QStringLiteral("Để trống nếu giữ nguyên") : QStringLiteral("Nhập mật khẩu..."));
    VirtualKeyboardDialog::attachToLineEdit(pInput, tr("Mật khẩu"));
    form->addRow(QStringLiteral("Mật khẩu:"), pInput);

    auto *rCombo = new QComboBox(&dlg);
    rCombo->addItem(QStringLiteral("Người dùng (User)"), QStringLiteral("user"));
    rCombo->addItem(QStringLiteral("Quản trị viên (Admin)"), QStringLiteral("admin"));
    if (user.value(QStringLiteral("role")).toString() == QStringLiteral("admin")) {
        rCombo->setCurrentIndex(1);
    }
    form->addRow(QStringLiteral("Quyền hạn:"), rCombo);

    auto *enCheck = new QCheckBox(QStringLiteral("Kích hoạt hoạt động tài khoản"), &dlg);
    enCheck->setChecked(user.value(QStringLiteral("enabled")).toBool(true));
    enCheck->setStyleSheet("color: #cbd5e1; font-size: 11px; font-weight: 700;");
    form->addRow(QString(), enCheck);

    mainLayout->addLayout(form);
    mainLayout->addStretch();

    // Action Buttons
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();

    auto *cancelBtn = new QPushButton(QStringLiteral("Hủy"), &dlg);
    cancelBtn->setStyleSheet("QPushButton { background: #334155; color: #ffffff; border: none; border-radius: 4px; font-size: 11px; font-weight: 800; padding: 6px 14px; }");
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    auto *saveBtn = new QPushButton(isEdit ? QStringLiteral("Lưu") : QStringLiteral("Tạo Mới"), &dlg);
    saveBtn->setStyleSheet("QPushButton { background: #10b981; color: #ffffff; border: none; border-radius: 4px; font-size: 11px; font-weight: 800; padding: 6px 16px; } QPushButton:hover { background: #059669; }");
    connect(saveBtn, &QPushButton::clicked, &dlg, [&] {
        const QString u = uInput->text().trimmed();
        const QString p = pInput->text();
        const QString r = rCombo->currentData().toString();
        const bool en = enCheck->isChecked();

        if (u.isEmpty()) {
            QMessageBox::warning(&dlg, QStringLiteral("Lỗi"), QStringLiteral("Vui lòng nhập tên tài khoản."));
            return;
        }

        if (isEdit) {
            emit updateUserRequested(oldUser, u, p, r, en);
        } else {
            if (p.isEmpty()) {
                QMessageBox::warning(&dlg, QStringLiteral("Lỗi"), QStringLiteral("Vui lòng nhập mật khẩu cho tài khoản mới."));
                return;
            }
            emit createUserRequested(u, p, r);
        }
        dlg.accept();
    });
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);

    dlg.exec();
}

void UserManagementPage::confirmDeleteUser(const QJsonObject &user)
{
    const QString username = user.value(QStringLiteral("username")).toString();
    auto res = QMessageBox::question(
        this,
        QStringLiteral("Xác Nhận Xóa"),
        QStringLiteral("Bạn có chắc chắn muốn xóa tài khoản <b>%1</b> không?").arg(username),
        QMessageBox::Yes | QMessageBox::No
    );
    if (res == QMessageBox::Yes) {
        emit deleteUserRequested(username);
    }
}
