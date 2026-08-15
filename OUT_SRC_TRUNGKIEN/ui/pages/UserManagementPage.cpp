#include "UserManagementPage.h"
#include "ui_UserManagementPage.h"

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

    ui->verticalLayout->setContentsMargins(24, 22, 24, 22);
    ui->verticalLayout->setSpacing(16);
    ui->headerLayout->setSpacing(12);

    ui->verticalLayout->removeWidget(ui->usersTable);
    auto *contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(18);
    ui->verticalLayout->addLayout(contentLayout, 1);

    ui->usersTable->setObjectName(QStringLiteral("usersTableModern"));
    ui->usersTable->setColumnCount(4);
    ui->usersTable->setHorizontalHeaderLabels(
        {tr("Tài khoản"), tr("Quyền"), tr("Thiết bị"), tr("Trạng thái")});
    ui->usersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->usersTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->usersTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->usersTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->usersTable->horizontalHeader()->setMinimumHeight(44);
    ui->usersTable->verticalHeader()->hide();
    ui->usersTable->verticalHeader()->setDefaultSectionSize(64);
    ui->usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->usersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->usersTable->setAlternatingRowColors(false);
    contentLayout->addWidget(ui->usersTable, 1);

    m_detailPanel = new QFrame(this);
    m_detailPanel->setObjectName(QStringLiteral("userDetailPanel"));
    m_detailPanel->setMinimumWidth(330);
    m_detailPanel->setMaximumWidth(390);
    m_detailLayout = new QVBoxLayout(m_detailPanel);
    m_detailLayout->setContentsMargins(22, 22, 22, 22);
    m_detailLayout->setSpacing(12);
    contentLayout->addWidget(m_detailPanel);
    clearUserDetails();

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

void UserManagementPage::clearUserDetails()
{
    while (QLayoutItem *item = m_detailLayout->takeAt(0)) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    auto *icon = new QLabel(QStringLiteral("♟"), m_detailPanel);
    icon->setObjectName(QStringLiteral("userDetailIcon"));
    icon->setAlignment(Qt::AlignCenter);
    auto *title = new QLabel(tr("Chọn tài khoản"), m_detailPanel);
    title->setObjectName(QStringLiteral("userDetailTitle"));
    auto *hint = new QLabel(tr("Bấm vào một user ở bảng bên trái để xem thiết bị, sửa hoặc xóa tài khoản."), m_detailPanel);
    hint->setObjectName(QStringLiteral("userDetailHint"));
    hint->setWordWrap(true);
    m_detailLayout->addWidget(icon, 0, Qt::AlignLeft);
    m_detailLayout->addWidget(title);
    m_detailLayout->addWidget(hint);
    m_detailLayout->addStretch();
}

void UserManagementPage::showUserDetails(const QJsonObject &user)
{
    m_selectedUser = user;
    while (QLayoutItem *item = m_detailLayout->takeAt(0)) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    const QString username = user.value(QStringLiteral("username")).toString();
    const QString role = user.value(QStringLiteral("role")).toString();
    const bool enabled = user.value(QStringLiteral("enabled")).toBool();
    const QStringList deviceIds = deviceIdsForUser(user);

    auto *top = new QHBoxLayout;
    auto *icon = new QLabel(role == QStringLiteral("admin") ? QStringLiteral("♛") : QStringLiteral("♟"), m_detailPanel);
    icon->setObjectName(QStringLiteral("userDetailIcon"));
    icon->setAlignment(Qt::AlignCenter);
    auto *close = new QPushButton(QStringLiteral("×"), m_detailPanel);
    close->setObjectName(QStringLiteral("userDetailCloseButton"));
    close->setFixedSize(34, 34);
    top->addWidget(icon);
    top->addStretch();
    top->addWidget(close);
    m_detailLayout->addLayout(top);
    connect(close, &QPushButton::clicked, this, &UserManagementPage::clearUserDetails);

    auto *title = new QLabel(username, m_detailPanel);
    title->setObjectName(QStringLiteral("userDetailTitle"));
    auto *meta = new QLabel(tr("%1  •  %2").arg(roleLabel(role), enabled ? tr("Đang hoạt động") : tr("Đã khóa")), m_detailPanel);
    meta->setObjectName(QStringLiteral("userDetailHint"));
    m_detailLayout->addWidget(title);
    m_detailLayout->addWidget(meta);

    auto *stat = new QLabel(tr("%1 thiết bị đang sử dụng").arg(deviceIds.size()), m_detailPanel);
    stat->setObjectName(QStringLiteral("userDetailStat"));
    m_detailLayout->addWidget(stat);

    auto *edit = new QPushButton(tr("Sửa tài khoản"), m_detailPanel);
    auto *remove = new QPushButton(tr("Xóa tài khoản"), m_detailPanel);
    edit->setObjectName(QStringLiteral("userPanelPrimaryButton"));
    remove->setObjectName(QStringLiteral("userPanelDangerButton"));
    m_detailLayout->addWidget(edit);
    m_detailLayout->addWidget(remove);
    connect(edit, &QPushButton::clicked, this, [this, user] { openEditDialog(user); });
    connect(remove, &QPushButton::clicked, this, [this, user] { confirmDeleteUser(user); });

    auto *devicesTitle = new QLabel(tr("Thiết bị của user"), m_detailPanel);
    devicesTitle->setObjectName(QStringLiteral("userDetailSectionTitle"));
    m_detailLayout->addWidget(devicesTitle);

    if (deviceIds.isEmpty()) {
        auto *empty = new QLabel(tr("User này chưa gắn thiết bị nào."), m_detailPanel);
        empty->setObjectName(QStringLiteral("userDetailEmpty"));
        empty->setWordWrap(true);
        m_detailLayout->addWidget(empty);
    } else {
        for (const QString &deviceId : deviceIds) {
            auto *card = new QFrame(m_detailPanel);
            card->setObjectName(QStringLiteral("userDeviceChip"));
            auto *layout = new QHBoxLayout(card);
            layout->setContentsMargins(12, 10, 10, 10);
            layout->setSpacing(8);
            auto *label = new QLabel(deviceId, card);
            label->setObjectName(QStringLiteral("userDeviceIdLabel"));
            auto *detach = new QPushButton(tr("Gỡ"), card);
            detach->setObjectName(QStringLiteral("userDeviceDetachButton"));
            detach->setFixedWidth(62);
            layout->addWidget(label, 1);
            layout->addWidget(detach);
            m_detailLayout->addWidget(card);
            connect(detach, &QPushButton::clicked, this, [this, username, deviceId] {
                if (QMessageBox::question(this, tr("Gỡ thiết bị"),
                        tr("Gỡ thiết bị %1 khỏi tài khoản %2?\nThiết bị sẽ xuất hiện lại trong danh sách có thể thêm.")
                            .arg(deviceId, username)) != QMessageBox::Yes)
                    return;
                emit releaseUserDeviceRequested(username, deviceId);
            });
        }
    }
    m_detailLayout->addStretch();
}

void UserManagementPage::openEditDialog(const QJsonObject &user)
{
    const bool editing = !user.isEmpty();
    const QString oldUsername = user.value(QStringLiteral("username")).toString();

    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("addUserDialog"));
    dialog.setWindowTitle(editing ? tr("Sửa tài khoản") : tr("Thêm tài khoản"));
    dialog.setModal(true);
    dialog.setFixedWidth(420);

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(26, 24, 26, 24);
    root->setSpacing(16);
    auto *title = new QLabel(editing ? tr("Sửa tài khoản") : tr("Thêm tài khoản"), &dialog);
    title->setObjectName(QStringLiteral("addUserDialogTitle"));
    auto *hint = new QLabel(editing
        ? tr("Đổi thông tin tài khoản. Để trống mật khẩu nếu không muốn đổi.")
        : tr("Tạo tài khoản đăng nhập cho người dùng mới."), &dialog);
    hint->setObjectName(QStringLiteral("addUserDialogHint"));
    hint->setWordWrap(true);
    root->addWidget(title);
    root->addWidget(hint);

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(12);
    auto *username = new QLineEdit(oldUsername, &dialog);
    auto *password = new QLineEdit(&dialog);
    auto *role = new QComboBox(&dialog);
    auto *enabled = new QCheckBox(tr("Đang hoạt động"), &dialog);
    username->setObjectName(QStringLiteral("addUserInput"));
    password->setObjectName(QStringLiteral("addUserInput"));
    role->setObjectName(QStringLiteral("addUserRole"));
    password->setEchoMode(QLineEdit::Password);
    username->setPlaceholderText(tr("VD: user01"));
    password->setPlaceholderText(editing ? tr("Không nhập = giữ mật khẩu cũ") : tr("Tối thiểu 8 ký tự"));
    role->addItem(tr("Người dùng"), QStringLiteral("viewer"));
    role->addItem(tr("Quản trị viên"), QStringLiteral("admin"));
    role->setCurrentIndex(user.value(QStringLiteral("role")).toString() == QStringLiteral("admin") ? 1 : 0);
    enabled->setChecked(user.value(QStringLiteral("enabled")).toBool(true));
    form->addRow(tr("Tài khoản"), username);
    form->addRow(editing ? tr("Mật khẩu mới") : tr("Mật khẩu"), password);
    form->addRow(tr("Quyền"), role);
    if (editing)
        form->addRow(tr("Trạng thái"), enabled);
    root->addLayout(form);

    auto *actions = new QHBoxLayout;
    actions->setSpacing(10);
    auto *cancel = new QPushButton(tr("Hủy"), &dialog);
    auto *save = new QPushButton(editing ? tr("Lưu thay đổi") : tr("Tạo tài khoản"), &dialog);
    cancel->setObjectName(QStringLiteral("addUserCancelButton"));
    save->setObjectName(QStringLiteral("addUserSaveButton"));
    actions->addWidget(cancel);
    actions->addWidget(save);
    root->addLayout(actions);
    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(save, &QPushButton::clicked, &dialog, &QDialog::accept);
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
                                 role->currentData().toString(), enabled->isChecked());
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
    ui->addUserButton->setVisible(enabled);
    m_detailPanel->setVisible(enabled);
    if (enabled)
        emit refreshRequested();
}
