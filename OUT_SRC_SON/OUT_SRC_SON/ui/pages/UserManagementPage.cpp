#include "UserManagementPage.h"
#include "VirtualKeyboard.h"
#include "ui_UserManagementPage.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {

QString roleLabel(const QString &role)
{
    return role == QStringLiteral("admin") ? QObject::tr("Quản trị viên")
                                           : QObject::tr("Người dùng");
}

QStringList deviceIdsForUser(const QJsonObject &user)
{
    QStringList deviceIds;
    for (const QJsonValue &deviceId : user.value(QStringLiteral("device_ids")).toArray())
        deviceIds.append(deviceId.toString());
    return deviceIds;
}

QFrame *createGlassCard()
{
    auto *card = new QFrame;
    card->setStyleSheet(
        "QFrame { "
        "  background-color: #0d1733; "
        "  border: 1px solid #1c2b54; "
        "  border-radius: 10px; "
        "} "
        "QFrame:hover { "
        "  border-color: #2b3d75; "
        "}"
    );
    return card;
}

} // namespace

UserManagementPage::UserManagementPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::UserManagementPage)
{
    ui->setupUi(this);
    setStyleSheet("background-color: #070d1e; color: #ecf2ff; font-family: sans-serif;");

    setupCustomLayout();
}

UserManagementPage::~UserManagementPage()
{
    delete ui;
}

void UserManagementPage::setupCustomLayout()
{
    // Clear legacy layout widgets
    while (QLayoutItem *item = ui->verticalLayout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    auto *mainLayout = ui->verticalLayout;
    mainLayout->setContentsMargins(12, 8, 12, 8);
    mainLayout->setSpacing(8);

    // ==========================================
    // ROW 1: HEADER & STATS BAR
    // ==========================================
    auto *topHeaderCard = createGlassCard();
    auto *topLayout = new QHBoxLayout(topHeaderCard);
    topLayout->setContentsMargins(12, 6, 12, 6);
    topLayout->setSpacing(10);

    // Title & Icon
    auto *titleCol = new QVBoxLayout;
    titleCol->setSpacing(1);
    auto *titleRow = new QHBoxLayout;
    auto *icon = new QLabel(QStringLiteral("🛡️"));
    icon->setStyleSheet("font-size: 14px;");
    auto *titleText = new QLabel(QStringLiteral("TRUNG TÂM PHÂN QUYỀN & BẢO MẬT"));
    titleText->setStyleSheet("color: #38bdf8; font-size: 12px; font-weight: 900; letter-spacing: 0.5px;");
    titleRow->addWidget(icon);
    titleRow->addWidget(titleText);
    titleRow->addStretch();
    titleCol->addLayout(titleRow);

    auto *subText = new QLabel(QStringLiteral("Quản lý người dùng, tài khoản và cấp quyền điều khiển thiết bị"));
    subText->setStyleSheet("color: #94a3b8; font-size: 9px;");
    titleCol->addWidget(subText);
    topLayout->addLayout(titleCol, 3);

    // Mini Stats Pills
    auto makeStatPill = [](const QString &lbl, const QString &val, const QString &col) {
        auto *p = new QFrame;
        p->setStyleSheet("background-color: rgba(15, 23, 42, 0.7); border: 1px solid #1e293b; border-radius: 6px; padding: 2px 6px;");
        auto *l = new QVBoxLayout(p);
        l->setContentsMargins(4, 2, 4, 2);
        l->setSpacing(0);
        auto *t = new QLabel(lbl);
        t->setStyleSheet("color: #64748b; font-size: 8px; font-weight: 700;");
        auto *v = new QLabel(val);
        v->setStyleSheet(QStringLiteral("color: %1; font-size: 12px; font-weight: 900; font-family: monospace;").arg(col));
        l->addWidget(t);
        l->addWidget(v);
        return qMakePair(p, v);
    };

    auto s1 = makeStatPill("TỔNG USER", "0", "#38bdf8");
    auto s2 = makeStatPill("ADMIN", "0", "#fbbf24");
    auto s3 = makeStatPill("HOẠT ĐỘNG", "0", "#10b981");
    m_statTotalUsers = s1.second;
    m_statAdminUsers = s2.second;
    m_statActiveUsers = s3.second;

    topLayout->addWidget(s1.first);
    topLayout->addWidget(s2.first);
    topLayout->addWidget(s3.first);

    // Add User Button
    m_addUserBtn = new QPushButton(QStringLiteral("+ Thêm Tài Khoản"));
    m_addUserBtn->setCursor(Qt::PointingHandCursor);
    m_addUserBtn->setMinimumHeight(32);
    m_addUserBtn->setStyleSheet(
        "QPushButton { "
        "  background: #10b981; "
        "  color: #ffffff; "
        "  border: none; "
        "  border-radius: 6px; "
        "  font-size: 11px; "
        "  font-weight: 900; "
        "  padding: 0 12px; "
        "} "
        "QPushButton:hover { "
        "  background: #059669; "
        "} "
        "QPushButton:pressed { "
        "  background: #047857; "
        "}"
    );
    connect(m_addUserBtn, &QPushButton::clicked, this, [this] {
        QJsonObject empty;
        openEditDialog(empty);
    });
    topLayout->addWidget(m_addUserBtn);

    mainLayout->addWidget(topHeaderCard);

    // ==========================================
    // ROW 2: 2-COLUMN SPLIT (User Cards List + Inspector Panel)
    // ==========================================
    auto *bodyLayout = new QHBoxLayout;
    bodyLayout->setSpacing(8);

    // --- LEFT: User Cards Scroll View ---
    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("background: transparent; border: none;");

    auto *cardsContainer = new QWidget;
    cardsContainer->setStyleSheet("background: transparent;");
    m_cardsLayout = new QVBoxLayout(cardsContainer);
    m_cardsLayout->setContentsMargins(0, 0, 4, 0);
    m_cardsLayout->setSpacing(6);

    scrollArea->setWidget(cardsContainer);
    bodyLayout->addWidget(scrollArea, 6);

    // --- RIGHT: Inspector Profile Panel ---
    m_inspectorPanel = createGlassCard();
    m_inspectorPanel->setStyleSheet(
        "QFrame { "
        "  background-color: #0c1633; "
        "  border: 1.5px solid #1c2b54; "
        "  border-radius: 10px; "
        "}"
    );
    m_inspectorLayout = new QVBoxLayout(m_inspectorPanel);
    m_inspectorLayout->setContentsMargins(12, 10, 12, 10);
    m_inspectorLayout->setSpacing(8);

    clearUserDetails();
    bodyLayout->addWidget(m_inspectorPanel, 4);

    mainLayout->addLayout(bodyLayout, 1);
}

void UserManagementPage::renderUserCards()
{
    // Clear cards layout
    QLayoutItem *child;
    while ((child = m_cardsLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    const QString selectedUser = m_selectedUser.value(QStringLiteral("username")).toString();

    for (int i = 0; i < m_users.size(); ++i) {
        const QJsonObject u = m_users[i].toObject();
        const QString uname = u.value(QStringLiteral("username")).toString();
        const QString role = u.value(QStringLiteral("role")).toString();
        const bool enabled = u.value(QStringLiteral("enabled")).toBool(true);
        const QStringList devices = deviceIdsForUser(u);
        const bool isAdmin = (role == QStringLiteral("admin"));
        const bool isSelected = (uname.compare(selectedUser, Qt::CaseInsensitive) == 0);

        auto *card = new QFrame;
        const QString cardStyle = isSelected
            ? "QFrame { background-color: #11234c; border: 2px solid #38bdf8; border-radius: 8px; } QFrame:hover { background-color: #132754; }"
            : "QFrame { background-color: #0d1733; border: 1px solid #1c2b54; border-radius: 8px; } QFrame:hover { border-color: #38bdf8; background-color: #0f1c3f; }";
        card->setStyleSheet(cardStyle);
        card->setCursor(Qt::PointingHandCursor);

        auto *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(10, 8, 10, 8);
        cardLayout->setSpacing(10);

        // Avatar Icon
        auto *avatarLbl = new QLabel(isAdmin ? QStringLiteral("👑") : QStringLiteral("👤"));
        avatarLbl->setStyleSheet(isAdmin
            ? "background: rgba(251, 191, 36, 0.15); border: 1.5px solid #fbbf24; border-radius: 18px; font-size: 16px; padding: 4px;"
            : "background: rgba(56, 189, 248, 0.15); border: 1.5px solid #38bdf8; border-radius: 18px; font-size: 16px; padding: 4px;");
        avatarLbl->setFixedSize(36, 36);
        avatarLbl->setAlignment(Qt::AlignCenter);
        cardLayout->addWidget(avatarLbl);

        // Info details
        auto *infoCol = new QVBoxLayout;
        infoCol->setSpacing(2);

        auto *topRow = new QHBoxLayout;
        auto *nameLbl = new QLabel(uname);
        nameLbl->setStyleSheet("color: #ffffff; font-size: 12px; font-weight: 900; background: transparent; border: none;");
        topRow->addWidget(nameLbl);

        auto *roleBadge = new QLabel(roleLabel(role));
        roleBadge->setStyleSheet(isAdmin
            ? "background: #f59e0b; color: #ffffff; font-size: 8px; font-weight: 900; border-radius: 4px; padding: 1px 5px;"
            : "background: #334155; color: #94a3b8; font-size: 8px; font-weight: 800; border-radius: 4px; padding: 1px 5px;");
        topRow->addWidget(roleBadge);

        auto *statusBadge = new QLabel(enabled ? QStringLiteral("🟢 Hoạt động") : QStringLiteral("🔴 Khóa"));
        statusBadge->setStyleSheet(enabled
            ? "color: #10b981; font-size: 8px; font-weight: 800; background: rgba(16, 185, 129, 0.12); border-radius: 4px; padding: 1px 5px;"
            : "color: #ef4444; font-size: 8px; font-weight: 800; background: rgba(239, 68, 68, 0.12); border-radius: 4px; padding: 1px 5px;");
        topRow->addWidget(statusBadge);
        topRow->addStretch();
        infoCol->addLayout(topRow);

        // Device tags
        auto *deviceRow = new QHBoxLayout;
        auto *devIcon = new QLabel(QStringLiteral("🖲"));
        devIcon->setStyleSheet("font-size: 9px;");
        auto *devText = new QLabel(devices.isEmpty() ? QStringLiteral("Chưa liên kết thiết bị") : devices.join(QStringLiteral(", ")));
        devText->setStyleSheet("color: #94a3b8; font-size: 9px; font-style: italic;");
        deviceRow->addWidget(devIcon);
        deviceRow->addWidget(devText);
        deviceRow->addStretch();
        infoCol->addLayout(deviceRow);

        cardLayout->addLayout(infoCol, 1);

        // Quick edit click
        auto *selectBtn = new QPushButton(QStringLiteral("Chi tiết ➔"));
        selectBtn->setStyleSheet("background: #131f3f; color: #38bdf8; border: 1px solid #233565; border-radius: 4px; font-size: 9px; font-weight: 800; padding: 4px 8px;");
        connect(selectBtn, &QPushButton::clicked, this, [this, u] {
            showUserDetails(u);
            renderUserCards();
        });
        cardLayout->addWidget(selectBtn);

        m_cardsLayout->addWidget(card);
    }
    m_cardsLayout->addStretch();
}

void UserManagementPage::showUserDetails(const QJsonObject &user)
{
    m_selectedUser = user;
    clearUserDetails();

    const QString username = user.value(QStringLiteral("username")).toString();
    const QString role = user.value(QStringLiteral("role")).toString();
    const bool enabled = user.value(QStringLiteral("enabled")).toBool(true);
    const QStringList deviceIds = deviceIdsForUser(user);
    const bool isAdmin = (role == QStringLiteral("admin"));

    // Header Profile
    auto *headRow = new QHBoxLayout;
    auto *avatar = new QLabel(isAdmin ? QStringLiteral("👑") : QStringLiteral("👤"));
    avatar->setStyleSheet(isAdmin
        ? "background: rgba(251, 191, 36, 0.2); border: 2px solid #fbbf24; border-radius: 20px; font-size: 18px;"
        : "background: rgba(56, 189, 248, 0.2); border: 2px solid #38bdf8; border-radius: 20px; font-size: 18px;");
    avatar->setFixedSize(40, 40);
    avatar->setAlignment(Qt::AlignCenter);
    headRow->addWidget(avatar);

    auto *uCol = new QVBoxLayout;
    uCol->setSpacing(1);
    auto *uNameLbl = new QLabel(username);
    uNameLbl->setStyleSheet("color: #ffffff; font-size: 14px; font-weight: 900;");
    auto *rLbl = new QLabel(roleLabel(role));
    rLbl->setStyleSheet(isAdmin ? "color: #fbbf24; font-size: 10px; font-weight: 800;" : "color: #38bdf8; font-size: 10px; font-weight: 800;");
    uCol->addWidget(uNameLbl);
    uCol->addWidget(rLbl);
    headRow->addLayout(uCol, 1);
    m_inspectorLayout->addLayout(headRow);

    // Divider
    auto *div = new QFrame;
    div->setFrameShape(QFrame::HLine);
    div->setStyleSheet("color: #1e293b;");
    m_inspectorLayout->addWidget(div);

    // Properties Box
    auto *propBox = new QFrame;
    propBox->setStyleSheet("background: rgba(15, 23, 42, 0.6); border: 1px solid #1e293b; border-radius: 6px; padding: 4px;");
    auto *propLayout = new QVBoxLayout(propBox);
    propLayout->setContentsMargins(6, 4, 6, 4);
    propLayout->setSpacing(3);

    auto *stRow = new QHBoxLayout;
    auto *stHead = new QLabel(QStringLiteral("Trạng thái hoạt động:"));
    stHead->setStyleSheet("color: #64748b; font-size: 10px;");
    auto *stVal = new QLabel(enabled ? QStringLiteral("🟢 Hoạt động") : QStringLiteral("🔴 Đã khóa"));
    stVal->setStyleSheet(enabled ? "color: #10b981; font-weight: 900; font-size: 10px;" : "color: #ef4444; font-weight: 900; font-size: 10px;");
    stRow->addWidget(stHead);
    stRow->addWidget(stVal);
    stRow->addStretch();
    propLayout->addLayout(stRow);

    auto *permRow = new QHBoxLayout;
    auto *permHead = new QLabel(QStringLiteral("Cấp quyền:"));
    permHead->setStyleSheet("color: #64748b; font-size: 10px;");
    auto *permVal = new QLabel(isAdmin ? QStringLiteral("Full Admin (Bơm + Ngưỡng + User)") : QStringLiteral("Viewer (Xem & Bật Bơm)"));
    permVal->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: 700;");
    permRow->addWidget(permHead);
    permRow->addWidget(permVal);
    permRow->addStretch();
    propLayout->addLayout(permRow);

    m_inspectorLayout->addWidget(propBox);

    // Devices Section
    auto *devHead = new QLabel(QStringLiteral("Thiết bị được phân quyền:"));
    devHead->setStyleSheet("color: #38bdf8; font-size: 11px; font-weight: 800; margin-top: 4px;");
    m_inspectorLayout->addWidget(devHead);

    if (deviceIds.isEmpty()) {
        auto *noDev = new QLabel(QStringLiteral("Chưa có thiết bị nào gán cho tài khoản này"));
        noDev->setStyleSheet("color: #64748b; font-size: 10px; font-style: italic;");
        m_inspectorLayout->addWidget(noDev);
    } else {
        for (const QString &devId : deviceIds) {
            auto *dRow = new QHBoxLayout;
            auto *dTag = new QLabel(QStringLiteral("🖲 %1").arg(devId));
            dTag->setStyleSheet("background: #111e42; color: #ffffff; border: 1px solid #1c2b54; border-radius: 4px; padding: 2px 6px; font-size: 10px; font-weight: 800;");
            dRow->addWidget(dTag);
            dRow->addStretch();

            auto *unbindBtn = new QPushButton(QStringLiteral("Gỡ"));
            unbindBtn->setCursor(Qt::PointingHandCursor);
            unbindBtn->setStyleSheet("background: #7f1d1d; color: #fecaca; border: none; border-radius: 3px; font-size: 9px; font-weight: 800; padding: 1px 6px;");
            connect(unbindBtn, &QPushButton::clicked, this, [this, username, devId] {
                emit releaseUserDeviceRequested(username, devId);
            });
            dRow->addWidget(unbindBtn);
            m_inspectorLayout->addLayout(dRow);
        }
    }

    m_inspectorLayout->addStretch();

    // Action Buttons
    auto *btnCol = new QVBoxLayout;
    btnCol->setSpacing(6);

    auto *editBtn = new QPushButton(QStringLiteral("✏️ Chỉnh Sửa & Đổi Mật Khẩu"));
    editBtn->setCursor(Qt::PointingHandCursor);
    editBtn->setMinimumHeight(30);
    editBtn->setStyleSheet("QPushButton { background: #1e3a8a; color: #ffffff; border: 1px solid #38bdf8; border-radius: 6px; font-size: 11px; font-weight: 800; } QPushButton:hover { background: #2563eb; }");
    connect(editBtn, &QPushButton::clicked, this, [this, user] {
        openEditDialog(user);
    });
    btnCol->addWidget(editBtn);

    if (username.compare(QStringLiteral("admin"), Qt::CaseInsensitive) != 0) {
        auto *delBtn = new QPushButton(QStringLiteral("🗑️ Xóa Tài Khoản Này"));
        delBtn->setCursor(Qt::PointingHandCursor);
        delBtn->setMinimumHeight(28);
        delBtn->setStyleSheet("QPushButton { background: rgba(239, 68, 68, 0.2); color: #ef4444; border: 1px solid #ef4444; border-radius: 6px; font-size: 10px; font-weight: 800; } QPushButton:hover { background: #ef4444; color: #ffffff; }");
        connect(delBtn, &QPushButton::clicked, this, [this, user] {
            confirmDeleteUser(user);
        });
        btnCol->addWidget(delBtn);
    }

    m_inspectorLayout->addLayout(btnCol);
}

void UserManagementPage::clearUserDetails()
{
    while (QLayoutItem *item = m_inspectorLayout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    auto *emptyLbl = new QLabel(QStringLiteral("👈 Chọn một tài khoản ở danh sách bên trái để xem hồ sơ và phân quyền thiết bị."));
    emptyLbl->setStyleSheet("color: #64748b; font-size: 11px; font-style: italic; padding: 20px;");
    emptyLbl->setWordWrap(true);
    emptyLbl->setAlignment(Qt::AlignCenter);
    m_inspectorLayout->addWidget(emptyLbl);
}

void UserManagementPage::openEditDialog(const QJsonObject &user)
{
    const bool editing = !user.isEmpty();
    const QString oldUsername = user.value(QStringLiteral("username")).toString();

    QDialog dialog(this);
    dialog.setWindowTitle(editing ? tr("Chỉnh sửa tài khoản") : tr("Tạo tài khoản mới"));
    dialog.resize(380, 280);
    dialog.setStyleSheet(
        "QDialog { background-color: #070d1e; color: #ecf2ff; font-family: sans-serif; } "
        "QLabel { color: #f1f5f9; font-size: 11px; font-weight: 700; } "
        "QLineEdit, QComboBox { background-color: #0f1c3f; color: #ffffff; border: 1.5px solid #233870; border-radius: 6px; font-size: 12px; padding: 4px 8px; min-height: 28px; } "
        "QLineEdit:focus, QComboBox:focus { border: 2px solid #38bdf8; background-color: #162447; } "
        "QCheckBox { color: #cbd5e1; font-size: 11px; font-weight: 700; }"
    );

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(16, 14, 16, 14);
    root->setSpacing(10);

    auto *title = new QLabel(editing ? tr("CẬP NHẬT TÀI KHOẢN: %1").arg(oldUsername) : tr("TẠO TÀI KHOẢN MỚI"), &dialog);
    title->setStyleSheet("color: #38bdf8; font-size: 13px; font-weight: 900;");
    root->addWidget(title);

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(8);

    auto *username = new QLineEdit(oldUsername, &dialog);
    auto *password = new QLineEdit(&dialog);
    auto *role = new QComboBox(&dialog);
    password->setEchoMode(QLineEdit::Password);
    username->setPlaceholderText(tr("VD: user_son"));
    password->setPlaceholderText(editing ? tr("Để trống nếu không đổi") : tr("Tối thiểu 8 ký tự"));
    role->addItem(tr("Người dùng (Viewer)"), QStringLiteral("viewer"));
    role->addItem(tr("Quản trị viên (Admin)"), QStringLiteral("admin"));
    role->setCurrentIndex(user.value(QStringLiteral("role")).toString() == QStringLiteral("admin") ? 1 : 0);

    form->addRow(tr("Tên đăng nhập:"), username);
    form->addRow(editing ? tr("Mật khẩu mới:") : tr("Mật khẩu:"), password);
    form->addRow(tr("Phân quyền:"), role);

    QCheckBox *enabled = nullptr;
    if (editing) {
        enabled = new QCheckBox(tr("Kích hoạt tài khoản"), &dialog);
        enabled->setChecked(user.value(QStringLiteral("enabled")).toBool(true));
        form->addRow(tr("Trạng thái:"), enabled);
    }
    root->addLayout(form);

    VirtualKeyboardDialog::attachToLineEdit(username, tr("Nhập tên tài khoản"));
    VirtualKeyboardDialog::attachToLineEdit(password, tr("Nhập mật khẩu"));

    auto *actions = new QHBoxLayout;
    actions->setSpacing(8);
    auto *cancel = new QPushButton(tr("Hủy"), &dialog);
    auto *save = new QPushButton(editing ? tr("Lưu Thay Đổi") : tr("Tạo Tài Khoản"), &dialog);
    cancel->setStyleSheet("background: #1e293b; color: #94a3b8; border: 1px solid #334155; border-radius: 6px; padding: 6px 14px; font-weight: 800; font-size: 11px;");
    save->setStyleSheet("background: #10b981; color: #ffffff; border: none; border-radius: 6px; padding: 6px 16px; font-weight: 900; font-size: 11px;");
    actions->addWidget(cancel);
    actions->addWidget(save);
    root->addLayout(actions);

    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(save, &QPushButton::clicked, &dialog, &QDialog::accept);

    username->setFocus();
    if (dialog.exec() != QDialog::Accepted)
        return;

    if (username->text().trimmed().size() < 3
        || (!editing && password->text().size() < 8)
        || (editing && !password->text().isEmpty() && password->text().size() < 8)) {
        QMessageBox::warning(this, tr("Dữ liệu không hợp lệ"),
                             tr("Tài khoản tối thiểu 3 ký tự. Mật khẩu tối thiểu 8 ký tự nếu có nhập."));
        return;
    }

    if (editing) {
        emit updateUserRequested(oldUsername, username->text(), password->text(),
                                 role->currentData().toString(), enabled ? enabled->isChecked() : true);
    } else {
        emit createUserRequested(username->text(), password->text(), role->currentData().toString());
    }
}

void UserManagementPage::confirmDeleteUser(const QJsonObject &user)
{
    const QString username = user.value(QStringLiteral("username")).toString();
    if (QMessageBox::question(this, tr("Xóa tài khoản"),
            tr("Bạn có chắc chắn muốn xóa tài khoản %1?\nThiết bị của tài khoản này sẽ được giải phóng để gắn lại.")
                .arg(username)) != QMessageBox::Yes)
        return;
    emit deleteUserRequested(username);
}

void UserManagementPage::setUsers(const QJsonArray &users)
{
    m_users = users;

    int totalCount = users.size();
    int adminCount = 0;
    int activeCount = 0;

    for (const QJsonValue &v : users) {
        const QJsonObject u = v.toObject();
        if (u.value(QStringLiteral("role")).toString() == QStringLiteral("admin"))
            adminCount++;
        if (u.value(QStringLiteral("enabled")).toBool(true))
            activeCount++;
    }

    if (m_statTotalUsers) m_statTotalUsers->setText(QString::number(totalCount));
    if (m_statAdminUsers) m_statAdminUsers->setText(QString::number(adminCount));
    if (m_statActiveUsers) m_statActiveUsers->setText(QString::number(activeCount));

    renderUserCards();

    // Select first user if none selected
    if (!users.isEmpty()) {
        const QString selName = m_selectedUser.value(QStringLiteral("username")).toString();
        bool found = false;
        for (const QJsonValue &v : users) {
            if (v.toObject().value(QStringLiteral("username")).toString().compare(selName, Qt::CaseInsensitive) == 0) {
                showUserDetails(v.toObject());
                found = true;
                break;
            }
        }
        if (!found) {
            showUserDetails(users.first().toObject());
        }
    } else {
        clearUserDetails();
    }
}

void UserManagementPage::setAdminEnabled(bool enabled)
{
    m_adminEnabled = enabled;
    if (m_addUserBtn)
        m_addUserBtn->setVisible(enabled);
}
