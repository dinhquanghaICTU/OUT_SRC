#include "UserManagementPage.h"
#include "VirtualKeyboard.h"
#include "ui_UserManagementPage.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QTableWidgetItem>
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
}

UserManagementPage::UserManagementPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::UserManagementPage)
{
    ui->setupUi(this);
    setObjectName(QStringLiteral("UserManagementPage"));
    ui->titleLabel->setObjectName(QStringLiteral("usersPageTitle"));
    ui->addUserButton->setObjectName(QStringLiteral("usersAddButton"));

    setStyleSheet(
        "QWidget#UserManagementPage { background-color: #070d1e; color: #ecf2ff; font-family: sans-serif; } "
        "QLabel#usersPageTitle { color: #38bdf8; font-size: 14px; font-weight: 900; } "
        "QPushButton#usersAddButton { background: #10b981; color: #ffffff; border: none; border-radius: 6px; padding: 4px 12px; font-size: 11px; font-weight: 900; } "
        "QPushButton#usersAddButton:hover { background: #059669; } "
        "QTableWidget#usersTableModern { background-color: #0c1630; color: #ffffff; gridline-color: #1c2b54; border: 1px solid #1c2b54; border-radius: 8px; font-size: 11px; } "
        "QTableWidget#usersTableModern QHeaderView::section { background-color: #111d3d; color: #94a3b8; font-weight: 800; font-size: 10px; padding: 4px; border: none; } "
        "QTableWidget#usersTableModern::item { padding: 2px 6px; } "
        "QTableWidget#usersTableModern::item:selected { background-color: #1e3a8a; color: #ffffff; } "
        "QFrame#userDetailPanel { background-color: #0d1733; border: 1px solid #1c2b54; border-radius: 8px; }"
    );

    ui->verticalLayout->setContentsMargins(10, 8, 10, 8);
    ui->verticalLayout->setSpacing(8);
    ui->headerLayout->setSpacing(8);

    ui->verticalLayout->removeWidget(ui->usersTable);
    auto *contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(10);
    ui->verticalLayout->addLayout(contentLayout, 1);

    ui->usersTable->setObjectName(QStringLiteral("usersTableModern"));
    ui->usersTable->setColumnCount(4);
    ui->usersTable->setHorizontalHeaderLabels(
        {tr("Tài khoản"), tr("Quyền"), tr("Thiết bị"), tr("Trạng thái")});
    ui->usersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->usersTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->usersTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->usersTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->usersTable->horizontalHeader()->setMinimumHeight(28);
    ui->usersTable->verticalHeader()->hide();
    ui->usersTable->verticalHeader()->setDefaultSectionSize(36);
    ui->usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->usersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->usersTable->setAlternatingRowColors(false);
    contentLayout->addWidget(ui->usersTable, 1);

    m_detailPanel = new QFrame(this);
    m_detailPanel->setObjectName(QStringLiteral("userDetailPanel"));
    m_detailPanel->hide();

    connect(ui->usersTable, &QTableWidget::cellClicked, this, [this](int row, int) {
        if (row >= 0 && row < m_users.size())
            showUserDetails(m_users.at(row).toObject());
    });

    connect(ui->addUserButton, &QPushButton::clicked, this, [this] {
        QJsonObject empty;
        openEditDialog(empty);
    });
}

UserManagementPage::~UserManagementPage() { delete ui; }

void UserManagementPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    applyResponsiveLayout();
}

void UserManagementPage::applyResponsiveLayout()
{
    m_detailPanel->hide();
}

void UserManagementPage::clearUserDetails()
{
}

void UserManagementPage::showUserDetails(const QJsonObject &user)
{
    m_selectedUser = user;
    const QString username = user.value(QStringLiteral("username")).toString();
    const QString role = user.value(QStringLiteral("role")).toString();
    const bool enabled = user.value(QStringLiteral("enabled")).toBool(true);
    const QStringList deviceIds = deviceIdsForUser(user);

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Chi tiết tài khoản"));
    dlg.setModal(true);
    dlg.setFixedSize(540, 420);
    dlg.setStyleSheet(
        "QDialog { background-color: #0b152d; color: #ffffff; font-family: sans-serif; } "
        "QLabel { color: #f1f5f9; font-size: 12px; font-weight: 700; } "
        "QLabel#dlgTitle { color: #38bdf8; font-size: 16px; font-weight: 900; } "
        "QLabel#dlgSubtitle { color: #cbd5e1; font-size: 11px; font-weight: 600; } "
        "QLabel#dlgSection { color: #38bdf8; font-size: 13px; font-weight: 800; } "
        "QFrame#deviceChip { background-color: #111d3d; border: 1px solid #233870; border-radius: 6px; } "
        "QPushButton#detachBtn { background-color: #7f1d1d; color: #fca5a5; border: 1px solid #991b1b; border-radius: 4px; font-size: 10px; font-weight: 800; padding: 4px 8px; } "
        "QPushButton#detachBtn:hover { background-color: #991b1b; } "
        "QPushButton#editBtn { background-color: #0284c7; color: #ffffff; border: none; border-radius: 6px; font-size: 11px; font-weight: 800; padding: 6px 14px; } "
        "QPushButton#editBtn:hover { background-color: #0369a1; } "
        "QPushButton#deleteBtn { background-color: #dc2626; color: #ffffff; border: none; border-radius: 6px; font-size: 11px; font-weight: 800; padding: 6px 14px; } "
        "QPushButton#deleteBtn:hover { background-color: #b91c1c; } "
        "QPushButton#closeBtn { background-color: #1e293b; color: #cbd5e1; border: 1px solid #334155; border-radius: 6px; font-size: 11px; font-weight: 800; padding: 6px 14px; } "
        "QPushButton#closeBtn:hover { background-color: #334155; }"
    );

    auto *root = new QVBoxLayout(&dlg);
    root->setContentsMargins(16, 14, 16, 14);
    root->setSpacing(10);

    // Header
    auto *head = new QHBoxLayout;
    auto *icon = new QLabel(role == QStringLiteral("admin") ? QStringLiteral("♛") : QStringLiteral("♟"), &dlg);
    icon->setStyleSheet("font-size: 24px; color: #38bdf8; background: #111d3d; border: 1px solid #233870; border-radius: 8px; padding: 6px 12px;");
    auto *info = new QVBoxLayout;
    info->setSpacing(2);
    auto *title = new QLabel(username, &dlg);
    title->setObjectName("dlgTitle");
    auto *sub = new QLabel(tr("Quyền: %1  |  Trạng thái: %2").arg(roleLabel(role), enabled ? tr("Đang hoạt động") : tr("Đã khóa")), &dlg);
    sub->setObjectName("dlgSubtitle");
    info->addWidget(title);
    info->addWidget(sub);
    head->addWidget(icon);
    head->addLayout(info, 1);
    root->addLayout(head);

    // Devices Section
    auto *sec = new QLabel(tr("Danh sách thiết bị liên kết (%1)").arg(deviceIds.size()), &dlg);
    sec->setObjectName("dlgSection");
    root->addWidget(sec);

    auto *scroll = new QScrollArea(&dlg);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background: transparent;");
    auto *devListWidget = new QWidget(scroll);
    devListWidget->setStyleSheet("background: transparent;");
    auto *devListLayout = new QVBoxLayout(devListWidget);
    devListLayout->setContentsMargins(0, 2, 0, 2);
    devListLayout->setSpacing(6);

    if (deviceIds.isEmpty()) {
        auto *empty = new QLabel(tr("Tài khoản này chưa gắn thiết bị nào."), devListWidget);
        empty->setStyleSheet("color: #64748b; font-size: 11px; padding: 8px;");
        devListLayout->addWidget(empty);
    } else {
        for (const QString &deviceId : deviceIds) {
            auto *chip = new QFrame(devListWidget);
            chip->setObjectName("deviceChip");
            auto *chipLayout = new QHBoxLayout(chip);
            chipLayout->setContentsMargins(10, 6, 10, 6);
            auto *dLabel = new QLabel(tr("ID: %1").arg(deviceId), chip);
            dLabel->setStyleSheet("color: #ffffff; font-size: 11px; font-weight: 700;");
            auto *detach = new QPushButton(tr("✕ Gỡ thiết bị"), chip);
            detach->setObjectName("detachBtn");
            chipLayout->addWidget(dLabel, 1);
            chipLayout->addWidget(detach);
            devListLayout->addWidget(chip);

            connect(detach, &QPushButton::clicked, &dlg, [this, username, deviceId, &dlg] {
                if (QMessageBox::question(&dlg, tr("Xác nhận gỡ"),
                        tr("Gỡ thiết bị '%1' khỏi tài khoản '%2'?\nThiết bị sẽ trở lại danh sách có thể thêm.").arg(deviceId, username)) == QMessageBox::Yes) {
                    emit releaseUserDeviceRequested(username, deviceId);
                    dlg.accept();
                }
            });
        }
    }
    devListLayout->addStretch();
    scroll->setWidget(devListWidget);
    root->addWidget(scroll, 1);

    // Actions
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(8);

    auto *deleteBtn = new QPushButton(tr("🗑 Xóa tài khoản"), &dlg);
    deleteBtn->setObjectName("deleteBtn");
    deleteBtn->setVisible(m_adminEnabled);
    connect(deleteBtn, &QPushButton::clicked, &dlg, [this, user, &dlg] {
        dlg.accept();
        confirmDeleteUser(user);
    });

    auto *editBtn = new QPushButton(tr("✏ Sửa tài khoản"), &dlg);
    editBtn->setObjectName("editBtn");
    editBtn->setVisible(m_adminEnabled);
    connect(editBtn, &QPushButton::clicked, &dlg, [this, user, &dlg] {
        dlg.accept();
        openEditDialog(user);
    });

    auto *closeBtn = new QPushButton(tr("Đóng"), &dlg);
    closeBtn->setObjectName("closeBtn");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    btnRow->addWidget(deleteBtn);
    btnRow->addWidget(editBtn);
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    root->addLayout(btnRow);

    dlg.exec();
}

void UserManagementPage::openEditDialog(const QJsonObject &user)
{
    const bool editing = !user.isEmpty();
    const QString oldUsername = user.value(QStringLiteral("username")).toString();

    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("addUserDialog"));
    dialog.setWindowTitle(editing ? tr("Sửa tài khoản") : tr("Thêm tài khoản"));
    dialog.setModal(true);
    dialog.setFixedSize(520, 380);
    dialog.setStyleSheet(
        "QDialog { background-color: #0b152d; color: #ffffff; font-family: sans-serif; } "
        "QLabel { color: #f1f5f9; font-size: 12px; font-weight: 700; } "
        "QLabel#dlgTitle { color: #38bdf8; font-size: 15px; font-weight: 900; } "
        "QLabel#dlgSubtitle { color: #94a3b8; font-size: 11px; font-weight: 600; } "
        "QLineEdit, QComboBox { background-color: #111d3d; color: #ffffff; border: 1.5px solid #233870; border-radius: 6px; padding: 6px 10px; font-size: 12px; font-weight: 700; min-height: 28px; } "
        "QLineEdit:focus, QComboBox:focus { border-color: #38bdf8; background-color: #172554; } "
        "QCheckBox { color: #ffffff; font-size: 12px; font-weight: 700; spacing: 8px; } "
        "QPushButton#saveBtn { background-color: #10b981; color: #ffffff; border: none; border-radius: 6px; font-size: 12px; font-weight: 900; padding: 8px 18px; } "
        "QPushButton#saveBtn:hover { background-color: #059669; } "
        "QPushButton#cancelBtn { background-color: #1e293b; color: #cbd5e1; border: 1px solid #334155; border-radius: 6px; font-size: 11px; font-weight: 800; padding: 6px 14px; } "
        "QPushButton#cancelBtn:hover { background-color: #334155; }"
    );

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(18, 14, 18, 14);
    root->setSpacing(12);

    auto *title = new QLabel(editing ? tr("Sửa thông tin tài khoản") : tr("Thêm tài khoản mới"), &dialog);
    title->setObjectName("dlgTitle");
    auto *hint = new QLabel(editing
        ? tr("Cập nhật quyền và mật khẩu. Để trống mật khẩu nếu không muốn đổi.")
        : tr("Tạo tài khoản đăng nhập cho người dùng mới."), &dialog);
    hint->setObjectName("dlgSubtitle");
    hint->setWordWrap(true);
    root->addWidget(title);
    root->addWidget(hint);

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(10);
    auto *username = new QLineEdit(oldUsername, &dialog);
    auto *password = new QLineEdit(&dialog);
    auto *role = new QComboBox(&dialog);
    password->setEchoMode(QLineEdit::Password);
    username->setPlaceholderText(tr("VD: user01"));
    password->setPlaceholderText(editing ? tr("Không nhập = giữ mật khẩu cũ") : tr("Tối thiểu 8 ký tự"));
    role->addItem(tr("Người dùng"), QStringLiteral("viewer"));
    role->addItem(tr("Quản trị viên"), QStringLiteral("admin"));
    role->setCurrentIndex(user.value(QStringLiteral("role")).toString() == QStringLiteral("admin") ? 1 : 0);

    auto *lblUser = new QLabel(tr("Tài khoản"), &dialog);
    auto *lblPass = new QLabel(editing ? tr("Mật khẩu mới") : tr("Mật khẩu"), &dialog);
    auto *lblRole = new QLabel(tr("Quyền"), &dialog);
    form->addRow(lblUser, username);
    form->addRow(lblPass, password);
    form->addRow(lblRole, role);

    QCheckBox *enabled = nullptr;
    if (editing) {
        enabled = new QCheckBox(tr("Đang hoạt động"), &dialog);
        enabled->setChecked(user.value(QStringLiteral("enabled")).toBool(true));
        auto *lblStatus = new QLabel(tr("Trạng thái"), &dialog);
        form->addRow(lblStatus, enabled);
    }
    root->addLayout(form);

    VirtualKeyboardDialog::attachToLineEdit(username, tr("Nhập tên tài khoản"));
    VirtualKeyboardDialog::attachToLineEdit(password, tr("Nhập mật khẩu"));

    auto *actions = new QHBoxLayout;
    actions->setSpacing(10);
    auto *cancel = new QPushButton(tr("Hủy"), &dialog);
    cancel->setObjectName("cancelBtn");
    auto *save = new QPushButton(editing ? tr("✔ Lưu thay đổi") : tr("✔ Tạo tài khoản"), &dialog);
    save->setObjectName("saveBtn");
    actions->addStretch();
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
            tr("Xóa tài khoản %1?\nCác thiết bị của user này sẽ được gỡ để có thể add lại.")
                .arg(username)) != QMessageBox::Yes)
        return;
    emit deleteUserRequested(username);
}

void UserManagementPage::setUsers(const QJsonArray &users)
{
    m_users = users;
    const QString selectedUsername = m_selectedUser.value(QStringLiteral("username")).toString();
    ui->usersTable->setRowCount(0);
    int selectedRow = -1;

    for (const QJsonValue &value : users) {
        const QJsonObject user = value.toObject();
        const int row = ui->usersTable->rowCount();
        ui->usersTable->insertRow(row);
        ui->usersTable->setRowHeight(row, 64);
        const QString username = user.value(QStringLiteral("username")).toString();
        const QString role = user.value(QStringLiteral("role")).toString();
        const bool enabled = user.value(QStringLiteral("enabled")).toBool();
        const QStringList deviceIds = deviceIdsForUser(user);

        ui->usersTable->setItem(row, 0, new QTableWidgetItem(QStringLiteral("  %1").arg(username)));
        ui->usersTable->setItem(row, 1, new QTableWidgetItem(roleLabel(role)));
        ui->usersTable->setItem(row, 2, new QTableWidgetItem(
            deviceIds.isEmpty() ? tr("Chưa có") : deviceIds.join(QStringLiteral(", "))));
        ui->usersTable->setItem(row, 3, new QTableWidgetItem(
            enabled ? tr("Đang hoạt động") : tr("Đã khóa")));
        if (username.compare(selectedUsername, Qt::CaseInsensitive) == 0)
            selectedRow = row;
    }

    if (selectedRow >= 0) {
        ui->usersTable->selectRow(selectedRow);
        showUserDetails(users.at(selectedRow).toObject());
    } else if (users.isEmpty()) {
        m_selectedUser = {};
        clearUserDetails();
    }
}

void UserManagementPage::setAdminEnabled(bool enabled)
{
    m_adminEnabled = enabled;
    ui->addUserButton->setVisible(enabled);
    applyResponsiveLayout();
    if (enabled)
        emit refreshRequested();
}
