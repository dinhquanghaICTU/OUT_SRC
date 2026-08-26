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
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
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
    if (m_filterAdminBtn) m_filterAdminBtn->setText(QStringLiteral("👑 Admin (%1)").arg(admins));
    if (m_filterUserBtn) m_filterUserBtn->setText(QStringLiteral("👤 Người Dùng (%1)").arg(regularUsers));

    renderUserGrid();
}

void UserManagementPage::setAdminEnabled(bool enabled)
{
    m_adminEnabled = enabled;
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
    // TOP COMMAND BAR (Title, Filter Tabs, Add)
    // ==========================================
    auto *topBar = new QHBoxLayout;
    topBar->setSpacing(6);

    auto *titleIcon = new QLabel(QStringLiteral("🛡️"));
    titleIcon->setStyleSheet("font-size: 14px; background: transparent;");
    auto *titleLbl = new QLabel(QStringLiteral("QUẢN LÝ TÀI KHOẢN"));
    titleLbl->setStyleSheet("color: #ffffff; font-size: 12px; font-weight: 800; background: transparent; letter-spacing: 0.5px;");

    topBar->addWidget(titleIcon);
    topBar->addWidget(titleLbl);
    topBar->addSpacing(10);

    // Filter Buttons
    auto makeFilterBtn = [&](const QString &label, const QString &mode) {
        auto *btn = new QPushButton(label);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setCheckable(true);
        btn->setStyleSheet(
            "QPushButton { "
            "  background: #1c1642; "
            "  color: #94a3b8; "
            "  border: 1px solid #2e2468; "
            "  border-radius: 5px; "
            "  font-size: 10px; "
            "  font-weight: 700; "
            "  padding: 3px 8px; "
            "} "
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
    m_filterAdminBtn = makeFilterBtn(QStringLiteral("👑 Admin (0)"), QStringLiteral("admin"));
    m_filterUserBtn = makeFilterBtn(QStringLiteral("👤 Người Dùng (0)"), QStringLiteral("user"));
    m_filterAllBtn->setChecked(true);

    topBar->addWidget(m_filterAllBtn);
    topBar->addWidget(m_filterAdminBtn);
    topBar->addWidget(m_filterUserBtn);
    topBar->addStretch();

    // + Add User Button
    auto *addUserBtn = new QPushButton(QStringLiteral("＋ TẠO TÀI KHOẢN"));
    addUserBtn->setCursor(Qt::PointingHandCursor);
    addUserBtn->setStyleSheet(
        "QPushButton { "
        "  background: #10b981; "
        "  color: #ffffff; "
        "  border: none; "
        "  border-radius: 5px; "
        "  font-size: 10px; "
        "  font-weight: 800; "
        "  padding: 4px 12px; "
        "} "
        "QPushButton:hover { background: #059669; } "
        "QPushButton:pressed { background: #047857; }"
    );
    connect(addUserBtn, &QPushButton::clicked, this, [this] { openEditDialog(); });
    topBar->addWidget(addUserBtn);

    mainLayout->addLayout(topBar);

    // ==========================================
    // GRID MATRIX AREA (Scrollable)
    // ==========================================
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
    mainLayout->addWidget(scroll, 1);
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
        cLayout->setContentsMargins(10, 8, 10, 8);
        cLayout->setSpacing(5);

        // --- ROW 1: Header (Avatar + Role Pill + Status Badge) ---
        auto *headerRow = new QHBoxLayout;
        headerRow->setSpacing(6);

        auto *avatarLbl = new QLabel(isAdmin ? QStringLiteral("👑") : QStringLiteral("👤"));
        avatarLbl->setFixedSize(24, 24);
        avatarLbl->setAlignment(Qt::AlignCenter);
        avatarLbl->setStyleSheet(QStringLiteral(
            "background: %1; border-radius: 12px; font-size: 13px; border: 1px solid %2;")
            .arg(isAdmin ? QStringLiteral("rgba(251, 191, 36, 0.2)") : QStringLiteral("rgba(56, 189, 248, 0.2)"))
            .arg(isAdmin ? QStringLiteral("#fbbf24") : QStringLiteral("#38bdf8"))
        );
        headerRow->addWidget(avatarLbl);

        auto *uNameLbl = new QLabel(username);
        uNameLbl->setStyleSheet("color: #ffffff; font-size: 13px; font-weight: 800; background: transparent;");
        headerRow->addWidget(uNameLbl);

        auto *roleBadge = new QLabel(isAdmin ? QStringLiteral("Admin") : QStringLiteral("User"));
        roleBadge->setStyleSheet(isAdmin
            ? "color: #fbbf24; font-size: 8px; font-weight: 800; background: rgba(251, 191, 36, 0.15); border-radius: 3px; padding: 1px 5px;"
            : "color: #38bdf8; font-size: 8px; font-weight: 800; background: rgba(56, 189, 248, 0.15); border-radius: 3px; padding: 1px 5px;");
        headerRow->addWidget(roleBadge);
        headerRow->addStretch();

        auto *statusBadge = new QLabel(isEnabled ? QStringLiteral("🟢 Bật") : QStringLiteral("🔴 Tắt"));
        statusBadge->setStyleSheet(isEnabled
            ? "color: #10b981; font-size: 8px; font-weight: 800; background: rgba(16, 185, 129, 0.12); border-radius: 3px; padding: 1px 5px;"
            : "color: #ef4444; font-size: 8px; font-weight: 800; background: rgba(239, 68, 68, 0.12); border-radius: 3px; padding: 1px 5px;");
        headerRow->addWidget(statusBadge);
        cLayout->addLayout(headerRow);

        // --- ROW 2: Bound Devices Box ---
        auto *devBox = new QFrame;
        devBox->setStyleSheet("background: #110d2e; border: 1px solid #272054; border-radius: 5px;");
        auto *devBoxLay = new QVBoxLayout(devBox);
        devBoxLay->setContentsMargins(6, 4, 6, 4);
        devBoxLay->setSpacing(3);

        if (devices.isEmpty()) {
            auto *noDevLbl = new QLabel(QStringLiteral("📡 Chưa gán thiết bị"));
            noDevLbl->setStyleSheet("color: #64748b; font-size: 9px; font-style: italic; background: transparent;");
            devBoxLay->addWidget(noDevLbl);
        } else {
            for (const QString &devId : devices) {
                auto *dRow = new QHBoxLayout;
                dRow->setContentsMargins(0, 0, 0, 0);
                auto *dName = new QLabel(QStringLiteral("🖲 %1").arg(devId));
                dName->setStyleSheet("color: #38bdf8; font-size: 9px; font-weight: 700; background: transparent;");
                dRow->addWidget(dName);
                dRow->addStretch();

                auto *unBtn = new QPushButton(QStringLiteral("✕ Gỡ"));
                unBtn->setCursor(Qt::PointingHandCursor);
                unBtn->setStyleSheet("QPushButton { background: #3b1424; color: #f87171; border: 1px solid #7f1d1d; border-radius: 3px; font-size: 8px; font-weight: 700; padding: 1px 4px; } QPushButton:hover { background: #dc2626; color: #fff; }");
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
        actRow->setSpacing(6);

        auto *editBtn = new QPushButton(QStringLiteral("✏ Sửa"));
        editBtn->setCursor(Qt::PointingHandCursor);
        editBtn->setStyleSheet(
            "QPushButton { "
            "  background: #2563eb; "
            "  color: #ffffff; "
            "  border: none; "
            "  border-radius: 4px; "
            "  font-size: 9px; "
            "  font-weight: 800; "
            "  padding: 3px 10px; "
            "} "
            "QPushButton:hover { background: #1d4ed8; }"
        );
        connect(editBtn, &QPushButton::clicked, this, [this, u] { openEditDialog(u); });
        actRow->addWidget(editBtn);

        if (username != QStringLiteral("admin")) {
            auto *delBtn = new QPushButton(QStringLiteral("🗑 Xóa"));
            delBtn->setCursor(Qt::PointingHandCursor);
            delBtn->setStyleSheet(
                "QPushButton { "
                "  background: #7f1d1d; "
                "  color: #fecaca; "
                "  border: 1px solid #991b1b; "
                "  border-radius: 4px; "
                "  font-size: 9px; "
                "  font-weight: 800; "
                "  padding: 3px 10px; "
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

void UserManagementPage::openEditDialog(const QJsonObject &user)
{
    const bool isEdit = !user.isEmpty();
    const QString oldUser = user.value(QStringLiteral("username")).toString();

    QDialog dlg(this);
    dlg.setWindowTitle(isEdit ? QStringLiteral("Cập Nhật Tài Khoản") : QStringLiteral("Tạo Tài Khoản Mới"));
    dlg.setFixedSize(360, 220);
    dlg.setStyleSheet(
        "QDialog { background-color: #0d0a26; color: #ecf2ff; font-family: sans-serif; } "
        "QLabel { color: #cbd5e1; font-weight: 700; font-size: 11px; } "
        "QLineEdit, QComboBox { background-color: #171338; color: #ffffff; border: 1px solid #2b235c; border-radius: 5px; padding: 4px 6px; font-size: 11px; font-weight: 600; } "
        "QLineEdit:focus, QComboBox:focus { border: 1px solid #38bdf8; background-color: #1f1a4a; }"
    );

    auto *mainLayout = new QVBoxLayout(&dlg);
    mainLayout->setContentsMargins(14, 12, 14, 12);
    mainLayout->setSpacing(8);

    auto *titleLbl = new QLabel(isEdit ? QStringLiteral("✏ Chỉnh Sửa: %1").arg(oldUser) : QStringLiteral("＋ Tạo Tài Khoản Mới"));
    titleLbl->setStyleSheet("color: #38bdf8; font-size: 12px; font-weight: 800;");
    mainLayout->addWidget(titleLbl);

    auto *form = new QFormLayout;
    form->setSpacing(6);

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
    enCheck->setStyleSheet("color: #cbd5e1; font-size: 10px; font-weight: 700;");
    form->addRow(QString(), enCheck);

    mainLayout->addLayout(form);
    mainLayout->addStretch();

    // Action Buttons
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();

    auto *cancelBtn = new QPushButton(QStringLiteral("Hủy"), &dlg);
    cancelBtn->setStyleSheet("QPushButton { background: #334155; color: #ffffff; border: none; border-radius: 4px; font-size: 10px; font-weight: 800; padding: 5px 12px; }");
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    auto *saveBtn = new QPushButton(isEdit ? QStringLiteral("💾 Lưu") : QStringLiteral("＋ Tạo Mới"), &dlg);
    saveBtn->setStyleSheet("QPushButton { background: #10b981; color: #ffffff; border: none; border-radius: 4px; font-size: 10px; font-weight: 800; padding: 5px 14px; } QPushButton:hover { background: #059669; }");
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
