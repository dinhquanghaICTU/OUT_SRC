#include "UserManagementPage.h"
#include "ui_UserManagementPage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QEvent>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QTableWidgetItem>
#include <utility>
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
    ui->titleLabel->setText(tr("Quản Lý Tài Khoản IoT & Thiết Bị"));
    ui->titleLabel->setObjectName(QStringLiteral("usersPageTitle"));
    ui->addUserButton->setText(tr("Thêm Thiết Bị Mới"));
    ui->addUserButton->setObjectName(QStringLiteral("usersAddButton"));

    ui->verticalLayout->setContentsMargins(28, 24, 28, 24);
    ui->verticalLayout->setSpacing(16);
    ui->headerLayout->setSpacing(12);

    auto *stats = new QHBoxLayout;
    stats->setContentsMargins(0, 0, 0, 0);
    stats->setSpacing(14);
    auto makeStat = [this](const QString &icon, const QString &title, QLabel **valueLabel, const char *name) {
        auto *card = new QFrame(this);
        card->setObjectName(QString::fromLatin1(name));
        auto *layout = new QHBoxLayout(card);
        layout->setContentsMargins(18, 14, 18, 14);
        layout->setSpacing(12);
        auto *iconLabel = new QLabel(icon, card);
        iconLabel->setObjectName(QStringLiteral("iotStatIcon"));
        iconLabel->setAlignment(Qt::AlignCenter);
        auto *texts = new QVBoxLayout;
        texts->setSpacing(2);
        auto *titleLabel = new QLabel(title, card);
        titleLabel->setObjectName(QStringLiteral("iotStatTitle"));
        *valueLabel = new QLabel(QStringLiteral("0"), card);
        (*valueLabel)->setObjectName(QStringLiteral("iotStatValue"));
        texts->addWidget(titleLabel);
        texts->addWidget(*valueLabel);
        layout->addWidget(iconLabel);
        layout->addLayout(texts, 1);
        return card;
    };
    stats->addWidget(makeStat(QStringLiteral("♙"), tr("Tổng Người Dùng"), &m_totalUsersLabel, "iotUserStatCard"));
    stats->addWidget(makeStat(QStringLiteral("▣"), tr("Thiết Bị Trực Tuyến"), &m_onlineDevicesLabel, "iotOnlineStatCard"));
    stats->addWidget(makeStat(QStringLiteral("⊘"), tr("Thiết Bị Ngoại Tuyến"), &m_offlineDevicesLabel, "iotOfflineStatCard"));
    ui->verticalLayout->insertLayout(1, stats);

    ui->verticalLayout->removeWidget(ui->usersTable);
    auto *contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(12);
    ui->verticalLayout->addLayout(contentLayout, 1);

    auto *leftPanel = new QFrame(this);
    leftPanel->setObjectName(QStringLiteral("iotRelationshipPanel"));
    leftPanel->setMinimumWidth(360);
    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(16, 16, 16, 16);
    leftLayout->setSpacing(12);
    auto *relationshipTitle = new QLabel(tr("User & Devices Relationship"), leftPanel);
    relationshipTitle->setObjectName(QStringLiteral("iotPanelTitle"));
    leftLayout->addWidget(relationshipTitle);

    ui->usersTable->setObjectName(QStringLiteral("usersTableModern"));
    ui->usersTable->setColumnCount(3);
    ui->usersTable->setHorizontalHeaderLabels({tr("Người dùng"), tr("Vai trò"), tr("Thiết bị")});
    ui->usersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->usersTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->usersTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->usersTable->horizontalHeader()->setMinimumHeight(40);
    ui->usersTable->verticalHeader()->hide();
    ui->usersTable->verticalHeader()->setDefaultSectionSize(74);
    ui->usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->usersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->usersTable->setAlternatingRowColors(false);
    leftLayout->addWidget(ui->usersTable, 1);
    contentLayout->addWidget(leftPanel, 1);

    auto *centerPanel = new QFrame(this);
    centerPanel->setObjectName(QStringLiteral("iotCenterPanel"));
    auto *centerLayout = new QVBoxLayout(centerPanel);
    centerLayout->setContentsMargins(16, 16, 16, 16);
    centerLayout->setSpacing(14);
    auto *kpiTitle = new QLabel(tr("KPI Summary"), centerPanel);
    kpiTitle->setObjectName(QStringLiteral("iotPanelTitle"));
    centerLayout->addWidget(kpiTitle);
    ui->verticalLayout->removeItem(stats);
    centerLayout->addLayout(stats);
    auto *monitor = new QFrame(centerPanel);
    monitor->setObjectName(QStringLiteral("iotMonitorPanel"));
    auto *monitorLayout = new QVBoxLayout(monitor);
    monitorLayout->setContentsMargins(14, 14, 14, 14);
    monitorLayout->setSpacing(12);
    auto *monitorTitle = new QLabel(tr("Device Status Monitor"), monitor);
    monitorTitle->setObjectName(QStringLiteral("iotPanelTitle"));
    monitorLayout->addWidget(monitorTitle);
    m_deviceMonitorGrid = new QGridLayout;
    m_deviceMonitorGrid->setHorizontalSpacing(10);
    m_deviceMonitorGrid->setVerticalSpacing(10);
    monitorLayout->addLayout(m_deviceMonitorGrid, 1);
    centerLayout->addWidget(monitor, 1);
    contentLayout->addWidget(centerPanel, 2);

    m_detailPanel = new QFrame(this);
    m_detailPanel->setObjectName(QStringLiteral("userDetailPanel"));
    m_detailPanel->setMinimumWidth(260);
    m_detailPanel->setMaximumWidth(320);
    m_detailLayout = new QVBoxLayout(m_detailPanel);
    m_detailLayout->setContentsMargins(16, 16, 16, 16);
    m_detailLayout->setSpacing(12);
    m_detailPanel->hide();
    clearUserDetails();

    connect(ui->addUserButton, &QPushButton::clicked, this, [this] {
        QJsonObject empty;
        openEditDialog(empty);
    });
}

UserManagementPage::~UserManagementPage() { delete ui; }

bool UserManagementPage::eventFilter(QObject *watched, QEvent *event)
{
    auto *card = qobject_cast<QFrame *>(watched);
    if (!card || card->objectName() != QStringLiteral("iotMonitorItem"))
        return QWidget::eventFilter(watched, event);

    if (event->type() == QEvent::Enter || event->type() == QEvent::HoverEnter) {
        if (auto *hint = card->findChild<QLabel *>(QStringLiteral("iotMonitorDeleteHint")))
            hint->setVisible(true);
        return false;
    }

    if (event->type() == QEvent::Leave || event->type() == QEvent::HoverLeave) {
        if (auto *hint = card->findChild<QLabel *>(QStringLiteral("iotMonitorDeleteHint")))
            hint->setVisible(false);
        return false;
    }

    if (event->type() == QEvent::MouseButtonRelease) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton)
            return false;

        const QString username = card->property("username").toString();
        const QString deviceId = card->property("deviceId").toString();
        const QString kind = card->property("kind").toString();

        for (const QJsonValue &value : std::as_const(m_users)) {
            const QJsonObject user = value.toObject();
            const QString candidateUsername = user.value(QStringLiteral("username")).toString();
            if ((kind == QStringLiteral("user") || kind == QStringLiteral("empty"))
                && candidateUsername == username) {
                showUserDetails(user);
                return true;
            }
            if (kind == QStringLiteral("device")
                && candidateUsername == username
                && deviceIdsForUser(user).contains(deviceId)) {
                if (!confirmReleaseDevice(username, deviceId))
                    return true;
                emit releaseUserDeviceRequested(username, deviceId);
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

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
    auto *title = new QLabel(tr("Quản lý thiết bị"), m_detailPanel);
    title->setObjectName(QStringLiteral("userDetailTitle"));
    auto *hint = new QLabel(tr("Chọn một người dùng trong bảng để quản lý tài khoản và thiết bị đã liên kết."), m_detailPanel);
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

    const QString username = user.value(QStringLiteral("username")).toString();
    const QString role = user.value(QStringLiteral("role")).toString();
    const bool enabled = user.value(QStringLiteral("enabled")).toBool();
    const QStringList deviceIds = deviceIdsForUser(user);

    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("userActionDialog"));
    dialog.setWindowTitle(tr("Thông tin chi tiết"));
    dialog.setModal(true);
    dialog.setWindowFlags(dialog.windowFlags() | Qt::FramelessWindowHint);
    dialog.setAttribute(Qt::WA_TranslucentBackground);
    dialog.setFixedSize(900, 570);

    auto *root = new QHBoxLayout(&dialog);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *profile = new QFrame(&dialog);
    profile->setObjectName(QStringLiteral("userActionProfilePanel"));
    auto *profileLayout = new QVBoxLayout(profile);
    profileLayout->setContentsMargins(28, 34, 28, 28);
    profileLayout->setSpacing(12);
    auto *avatar = new QLabel(role == QStringLiteral("admin") ? QStringLiteral("♛") : QStringLiteral("♙"), profile);
    avatar->setObjectName(QStringLiteral("userActionAvatar"));
    avatar->setAlignment(Qt::AlignCenter);
    auto *profileTitle = new QLabel(tr("Chỉnh sửa tài khoản"), profile);
    profileTitle->setObjectName(QStringLiteral("userActionProfileTitle"));
    profileTitle->setAlignment(Qt::AlignCenter);
    profileTitle->setWordWrap(true);
    auto *profileHint = new QLabel(tr("Cập nhật thông tin profile của %1").arg(username), profile);
    profileHint->setObjectName(QStringLiteral("userActionProfileHint"));
    profileHint->setAlignment(Qt::AlignCenter);
    profileHint->setWordWrap(true);
    auto *statusTitle = new QLabel(tr("Trạng thái:"), profile);
    statusTitle->setObjectName(QStringLiteral("userActionStatusTitle"));
    auto *statusLine = new QLabel(enabled ? tr("● Đang hoạt động") : tr("⚠ Tạm khóa"), profile);
    statusLine->setObjectName(enabled ? QStringLiteral("userActionStatusOnline")
                                      : QStringLiteral("userActionStatusLocked"));
    auto *deviceLine = new QLabel(tr("● %1 thiết bị đang dùng").arg(deviceIds.size()), profile);
    deviceLine->setObjectName(QStringLiteral("userActionStatusDevice"));
    profileLayout->addWidget(avatar, 0, Qt::AlignHCenter);
    profileLayout->addSpacing(10);
    profileLayout->addWidget(profileTitle);
    profileLayout->addWidget(profileHint);
    profileLayout->addSpacing(18);
    profileLayout->addWidget(statusTitle);
    profileLayout->addWidget(statusLine);
    profileLayout->addWidget(deviceLine);
    profileLayout->addStretch();
    root->addWidget(profile, 0);

    auto *formPanel = new QFrame(&dialog);
    formPanel->setObjectName(QStringLiteral("userActionFormPanel"));
    auto *formRoot = new QVBoxLayout(formPanel);
    formRoot->setContentsMargins(40, 28, 40, 28);
    formRoot->setSpacing(14);

    auto *top = new QHBoxLayout;
    top->setContentsMargins(0, 0, 0, 0);
    auto *titleBlock = new QVBoxLayout;
    titleBlock->setSpacing(4);
    auto *title = new QLabel(tr("Thông tin chi tiết"), formPanel);
    title->setObjectName(QStringLiteral("userActionTitle"));
    auto *hint = new QLabel(tr("Cập nhật thông tin chi tiết cho tài khoản hiện có."), formPanel);
    hint->setObjectName(QStringLiteral("userActionHint"));
    hint->setWordWrap(true);
    titleBlock->addWidget(title);
    titleBlock->addWidget(hint);
    auto *close = new QPushButton(QStringLiteral("×"), formPanel);
    close->setObjectName(QStringLiteral("userActionCloseButton"));
    close->setFixedSize(38, 38);
    top->addLayout(titleBlock, 1);
    top->addWidget(close, 0, Qt::AlignTop);
    formRoot->addLayout(top);
    connect(close, &QPushButton::clicked, &dialog, &QDialog::reject);

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(18);
    form->setVerticalSpacing(12);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    auto *usernameInput = new QLineEdit(username, formPanel);
    auto *passwordInput = new QLineEdit(formPanel);
    auto *roleInput = new QComboBox(formPanel);
    auto *enabledInput = new QCheckBox(tr("Tài khoản đang hoạt động"), formPanel);
    usernameInput->setObjectName(QStringLiteral("userActionInput"));
    passwordInput->setObjectName(QStringLiteral("userActionInput"));
    roleInput->setObjectName(QStringLiteral("userActionCombo"));
    enabledInput->setObjectName(QStringLiteral("userActionCheck"));
    usernameInput->setMinimumWidth(390);
    passwordInput->setMinimumWidth(390);
    roleInput->setMinimumWidth(390);
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setPlaceholderText(tr("Bỏ trống nếu không muốn đổi mật khẩu"));
    roleInput->addItem(tr("Người dùng"), QStringLiteral("viewer"));
    roleInput->addItem(tr("Quản trị viên"), QStringLiteral("admin"));
    roleInput->setCurrentIndex(role == QStringLiteral("admin") ? 1 : 0);
    enabledInput->setChecked(enabled);
    form->addRow(tr("Tài khoản"), usernameInput);
    form->addRow(tr("Mật khẩu"), passwordInput);
    form->addRow(QString(), new QLabel(tr("Bỏ trống nếu không muốn đổi mật khẩu."), formPanel));
    form->addRow(tr("Quyền"), roleInput);
    form->addRow(tr("Trạng thái"), enabledInput);
    formRoot->addLayout(form);

    auto *actions = new QHBoxLayout;
    actions->setSpacing(12);
    auto *deleteAccount = new QPushButton(tr("Xóa tài khoản"), formPanel);
    auto *save = new QPushButton(tr("Cập nhật"), formPanel);
    deleteAccount->setObjectName(QStringLiteral("userPanelDangerButton"));
    save->setObjectName(QStringLiteral("userPanelPrimaryButton"));
    actions->addWidget(deleteAccount);
    actions->addWidget(save);
    formRoot->addStretch();
    formRoot->addLayout(actions);
    root->addWidget(formPanel, 1);

    connect(deleteAccount, &QPushButton::clicked, &dialog, [this, user, &dialog] {
        dialog.accept();
        confirmDeleteUser(user);
    });
    connect(save, &QPushButton::clicked, &dialog, [this, username, usernameInput, passwordInput, roleInput, enabledInput, &dialog] {
        if (usernameInput->text().trimmed().size() < 3
            || (!passwordInput->text().isEmpty() && passwordInput->text().size() < 8)) {
            QMessageBox::warning(this, tr("Dữ liệu không hợp lệ"),
                                 tr("Tài khoản tối thiểu 3 ký tự. Mật khẩu tối thiểu 8 ký tự nếu có nhập."));
            return;
        }
        dialog.accept();
        emit updateUserRequested(username, usernameInput->text().trimmed(), passwordInput->text(),
                                 roleInput->currentData().toString(), enabledInput->isChecked());
    });

    dialog.exec();
}

void UserManagementPage::openEditDialog(const QJsonObject &user)
{
    const bool editing = !user.isEmpty();
    const QString oldUsername = user.value(QStringLiteral("username")).toString();
    const QString oldRole = user.value(QStringLiteral("role")).toString();
    const bool oldEnabled = user.value(QStringLiteral("enabled")).toBool(true);

    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("userActionDialog"));
    dialog.setWindowTitle(editing ? tr("Sửa tài khoản") : tr("Thêm tài khoản"));
    dialog.setModal(true);
    dialog.setWindowFlags(dialog.windowFlags() | Qt::FramelessWindowHint);
    dialog.setAttribute(Qt::WA_TranslucentBackground);
    dialog.setFixedSize(900, 520);

    auto *root = new QHBoxLayout(&dialog);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *profile = new QFrame(&dialog);
    profile->setObjectName(QStringLiteral("userActionProfilePanel"));
    auto *profileLayout = new QVBoxLayout(profile);
    profileLayout->setContentsMargins(28, 34, 28, 28);
    profileLayout->setSpacing(12);

    auto *avatar = new QLabel(editing ? QStringLiteral("♙") : QStringLiteral("●"), profile);
    avatar->setObjectName(QStringLiteral("userActionAvatar"));
    avatar->setAlignment(Qt::AlignCenter);
    auto *profileTitle = new QLabel(editing ? tr("Chỉnh sửa tài khoản") : tr("Tài khoản mới"), profile);
    profileTitle->setObjectName(QStringLiteral("userActionProfileTitle"));
    profileTitle->setAlignment(Qt::AlignCenter);
    profileTitle->setWordWrap(true);
    auto *profileHint = new QLabel(editing
        ? tr("Cập nhật thông tin profile của %1").arg(oldUsername)
        : tr("Đang cấu hình hồ sơ đăng nhập mới."), profile);
    profileHint->setObjectName(QStringLiteral("userActionProfileHint"));
    profileHint->setAlignment(Qt::AlignCenter);
    profileHint->setWordWrap(true);
    auto *statusTitle = new QLabel(tr("Trạng thái:"), profile);
    statusTitle->setObjectName(QStringLiteral("userActionStatusTitle"));
    auto *statusLine = new QLabel(oldEnabled ? tr("✓ Đang hoạt động") : tr("⚠ Tạm khóa"), profile);
    statusLine->setObjectName(oldEnabled ? QStringLiteral("userActionStatusOnline")
                                         : QStringLiteral("userActionStatusLocked"));
    profileLayout->addWidget(avatar, 0, Qt::AlignHCenter);
    profileLayout->addSpacing(10);
    profileLayout->addWidget(profileTitle);
    profileLayout->addWidget(profileHint);
    profileLayout->addSpacing(24);
    profileLayout->addWidget(statusTitle);
    profileLayout->addWidget(statusLine);
    profileLayout->addStretch();
    root->addWidget(profile, 0);

    auto *formPanel = new QFrame(&dialog);
    formPanel->setObjectName(QStringLiteral("userActionFormPanel"));
    auto *formRoot = new QVBoxLayout(formPanel);
    formRoot->setContentsMargins(40, 34, 40, 34);
    formRoot->setSpacing(16);

    auto *top = new QHBoxLayout;
    top->setContentsMargins(0, 0, 0, 0);
    auto *titleBlock = new QVBoxLayout;
    titleBlock->setSpacing(6);
    auto *title = new QLabel(tr("Thông tin chi tiết"), formPanel);
    title->setObjectName(QStringLiteral("userActionTitle"));
    auto *hint = new QLabel(editing
        ? tr("Cập nhật thông tin chi tiết cho tài khoản hiện có.")
        : tr("Tạo tài khoản đăng nhập cho người dùng mới."), formPanel);
    hint->setObjectName(QStringLiteral("userActionHint"));
    hint->setWordWrap(true);
    titleBlock->addWidget(title);
    titleBlock->addWidget(hint);
    auto *close = new QPushButton(QStringLiteral("×"), formPanel);
    close->setObjectName(QStringLiteral("userActionCloseButton"));
    close->setFixedSize(38, 38);
    top->addLayout(titleBlock, 1);
    top->addWidget(close, 0, Qt::AlignTop);
    formRoot->addLayout(top);
    connect(close, &QPushButton::clicked, &dialog, &QDialog::reject);

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(18);
    form->setVerticalSpacing(14);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    auto *username = new QLineEdit(oldUsername, formPanel);
    auto *password = new QLineEdit(formPanel);
    auto *role = new QComboBox(formPanel);
    auto *enabled = new QCheckBox(tr("Tài khoản đang hoạt động"), formPanel);
    username->setObjectName(QStringLiteral("userActionInput"));
    password->setObjectName(QStringLiteral("userActionInput"));
    role->setObjectName(QStringLiteral("userActionCombo"));
    enabled->setObjectName(QStringLiteral("userActionCheck"));
    username->setMinimumWidth(390);
    password->setMinimumWidth(390);
    role->setMinimumWidth(390);
    password->setEchoMode(QLineEdit::Password);
    username->setPlaceholderText(tr("VD: user01"));
    password->setPlaceholderText(editing ? tr("Không nhập = giữ mật khẩu cũ") : tr("Tối thiểu 8 ký tự"));
    role->addItem(tr("Người dùng"), QStringLiteral("viewer"));
    role->addItem(tr("Quản trị viên"), QStringLiteral("admin"));
    role->setCurrentIndex(oldRole == QStringLiteral("admin") ? 1 : 0);
    enabled->setChecked(oldEnabled);

    form->addRow(tr("Tài khoản"), username);
    form->addRow(editing ? tr("Mật khẩu mới") : tr("Mật khẩu"), password);
    auto *passwordHint = new QLabel(editing ? tr("Bỏ trống nếu không muốn đổi mật khẩu.") : tr("Mật khẩu tối thiểu 8 ký tự."), formPanel);
    passwordHint->setObjectName(QStringLiteral("userActionSmallHint"));
    form->addRow(QString(), passwordHint);
    form->addRow(tr("Quyền"), role);
    if (editing)
        form->addRow(tr("Trạng thái"), enabled);
    formRoot->addLayout(form);

    auto *actions = new QHBoxLayout;
    actions->setSpacing(14);
    auto *cancel = new QPushButton(tr("Hủy"), formPanel);
    auto *save = new QPushButton(editing ? tr("Cập nhật") : tr("Tạo tài khoản"), formPanel);
    cancel->setObjectName(QStringLiteral("userPanelSecondaryButton"));
    save->setObjectName(QStringLiteral("userPanelPrimaryButton"));
    actions->addWidget(cancel);
    actions->addWidget(save);
    formRoot->addStretch();
    formRoot->addLayout(actions);
    root->addWidget(formPanel, 1);

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
        emit updateUserRequested(oldUsername, username->text().trimmed(), password->text(),
                                 role->currentData().toString(), enabled->isChecked());
    } else {
        emit createUserRequested(username->text().trimmed(), password->text(), role->currentData().toString());
    }
}

bool UserManagementPage::confirmReleaseDevice(const QString &username, const QString &deviceId)
{
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("confirmDeviceDialog"));
    dialog.setWindowTitle(tr("Xác nhận gỡ thiết bị"));
    dialog.setModal(true);
    dialog.setWindowFlags(dialog.windowFlags() | Qt::FramelessWindowHint);
    dialog.setAttribute(Qt::WA_TranslucentBackground);
    dialog.setFixedSize(860, 520);

    auto *root = new QHBoxLayout(&dialog);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *profile = new QFrame(&dialog);
    profile->setObjectName(QStringLiteral("confirmDeviceProfilePanel"));
    auto *profileLayout = new QVBoxLayout(profile);
    profileLayout->setContentsMargins(30, 34, 30, 28);
    profileLayout->setSpacing(12);

    auto *deviceIcon = new QLabel(QStringLiteral("▣"), profile);
    deviceIcon->setObjectName(QStringLiteral("confirmDeviceIcon"));
    deviceIcon->setAlignment(Qt::AlignCenter);
    auto *profileTitle = new QLabel(tr("Xác nhận gỡ"), profile);
    profileTitle->setObjectName(QStringLiteral("confirmDeviceProfileTitle"));
    profileTitle->setAlignment(Qt::AlignCenter);
    auto *profileHint = new QLabel(tr("Bạn đang gỡ thiết bị khỏi tài khoản..."), profile);
    profileHint->setObjectName(QStringLiteral("confirmDeviceProfileHint"));
    profileHint->setAlignment(Qt::AlignCenter);
    profileHint->setWordWrap(true);
    auto *info = new QLabel(tr("Thiết bị: %1\nTài khoản: %2\nTrạng thái:\n🛡 Thiết bị khả dụng lại")
                                .arg(deviceId, username), profile);
    info->setObjectName(QStringLiteral("confirmDeviceInfo"));
    info->setWordWrap(true);
    auto *warning = new QLabel(tr("Thông tin quan trọng:\n1 Thiết bị sẽ xuất hiện lại trong danh sách có thể thêm.\n2 Mọi dữ liệu lịch sử của thiết bị vẫn được giữ trên server nếu server đang lưu."), profile);
    warning->setObjectName(QStringLiteral("confirmDeviceWarning"));
    warning->setWordWrap(true);
    profileLayout->addWidget(deviceIcon, 0, Qt::AlignHCenter);
    profileLayout->addSpacing(8);
    profileLayout->addWidget(profileTitle);
    profileLayout->addWidget(profileHint);
    profileLayout->addSpacing(16);
    profileLayout->addWidget(info);
    profileLayout->addWidget(warning);
    profileLayout->addStretch();
    root->addWidget(profile);

    auto *formPanel = new QFrame(&dialog);
    formPanel->setObjectName(QStringLiteral("confirmDeviceFormPanel"));
    auto *formRoot = new QVBoxLayout(formPanel);
    formRoot->setContentsMargins(40, 32, 40, 32);
    formRoot->setSpacing(18);

    auto *top = new QHBoxLayout;
    auto *titleBlock = new QVBoxLayout;
    titleBlock->setSpacing(6);
    auto *title = new QLabel(tr("Thông tin chi tiết xác nhận"), formPanel);
    title->setObjectName(QStringLiteral("confirmDeviceTitle"));
    auto *hint = new QLabel(tr("Vui lòng xác nhận hành động gỡ thiết bị bên dưới."), formPanel);
    hint->setObjectName(QStringLiteral("confirmDeviceHint"));
    hint->setWordWrap(true);
    titleBlock->addWidget(title);
    titleBlock->addWidget(hint);
    auto *close = new QPushButton(QStringLiteral("×"), formPanel);
    close->setObjectName(QStringLiteral("confirmDeviceCloseButton"));
    close->setFixedSize(40, 40);
    top->addLayout(titleBlock, 1);
    top->addWidget(close, 0, Qt::AlignTop);
    formRoot->addLayout(top);
    connect(close, &QPushButton::clicked, &dialog, &QDialog::reject);

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(14);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    auto *account = new QLineEdit(username, formPanel);
    account->setObjectName(QStringLiteral("confirmDeviceInput"));
    account->setReadOnly(true);
    auto *reason = new QComboBox(formPanel);
    reason->setObjectName(QStringLiteral("confirmDeviceCombo"));
    reason->addItem(tr("Gỡ thiết bị thông thường"));
    reason->addItem(tr("Chuyển thiết bị cho user khác"));
    reason->addItem(tr("Thiết bị bị thay thế"));
    form->addRow(tr("Tài khoản"), account);
    form->addRow(tr("Lý do gỡ"), reason);
    formRoot->addLayout(form);

    auto *question = new QLabel(tr("Gỡ thiết bị %1 khỏi tài khoản %2?").arg(deviceId, username), formPanel);
    question->setObjectName(QStringLiteral("confirmDeviceQuestion"));
    question->setWordWrap(true);
    formRoot->addWidget(question);

    auto *actions = new QHBoxLayout;
    actions->setSpacing(14);
    auto *confirm = new QPushButton(tr("Xác nhận Gỡ"), formPanel);
    auto *cancel = new QPushButton(tr("Hủy"), formPanel);
    confirm->setObjectName(QStringLiteral("confirmDevicePrimaryButton"));
    cancel->setObjectName(QStringLiteral("confirmDeviceCancelButton"));
    actions->addWidget(confirm);
    actions->addWidget(cancel);
    actions->addStretch();
    formRoot->addStretch();
    formRoot->addLayout(actions);
    root->addWidget(formPanel, 1);

    connect(confirm, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    return dialog.exec() == QDialog::Accepted;
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
    int totalDevices = 0;

    for (const QJsonValue &value : users) {
        const QJsonObject user = value.toObject();
        const int row = ui->usersTable->rowCount();
        ui->usersTable->insertRow(row);
        ui->usersTable->setRowHeight(row, 64);
        const QString username = user.value(QStringLiteral("username")).toString();
        const QString role = user.value(QStringLiteral("role")).toString();
        const bool enabled = user.value(QStringLiteral("enabled")).toBool();
        const QStringList deviceIds = deviceIdsForUser(user);
        totalDevices += deviceIds.size();

        ui->usersTable->setItem(row, 0, new QTableWidgetItem(QStringLiteral("%1\n%2@lenam.local").arg(username, username)));
        ui->usersTable->setItem(row, 1, new QTableWidgetItem(roleLabel(role)));
        ui->usersTable->setItem(row, 2, new QTableWidgetItem(tr("%1 thiết bị").arg(deviceIds.size())));
        if (username.compare(selectedUsername, Qt::CaseInsensitive) == 0)
            selectedRow = row;
    }

    if (m_totalUsersLabel)
        m_totalUsersLabel->setText(QString::number(users.size()));
    if (m_onlineDevicesLabel)
        m_onlineDevicesLabel->setText(QString::number(totalDevices));
    if (m_offlineDevicesLabel)
        m_offlineDevicesLabel->setText(QString::number(0));

    if (m_deviceMonitorGrid) {
        while (QLayoutItem *item = m_deviceMonitorGrid->takeAt(0)) {
            if (item->widget())
                item->widget()->deleteLater();
            delete item;
        }

        auto makeMonitorItem = [this](const QString &iconText,
                                      const QString &title,
                                      const QString &subtitle,
                                      const QString &status,
                                      const QString &kind,
                                      const QString &username,
                                      const QString &deviceId = {}) {
            auto *card = new QFrame(this);
            card->setObjectName(QStringLiteral("iotMonitorItem"));
            card->setProperty("kind", kind);
            card->setProperty("username", username);
            card->setProperty("deviceId", deviceId);
            card->setFixedSize(360, 94);
            card->setCursor(Qt::PointingHandCursor);
            card->installEventFilter(this);

            auto *layout = new QHBoxLayout(card);
            layout->setContentsMargins(14, 12, 12, 12);
            layout->setSpacing(12);

            auto *icon = new QLabel(iconText, card);
            icon->setObjectName(QStringLiteral("iotMonitorItemIcon"));
            icon->setProperty("kind", kind);
            icon->setAlignment(Qt::AlignCenter);
            icon->setFixedSize(52, 52);
            icon->setAttribute(Qt::WA_TransparentForMouseEvents);
            layout->addWidget(icon, 0, Qt::AlignVCenter);

            auto *texts = new QVBoxLayout;
            texts->setContentsMargins(0, 0, 0, 0);
            texts->setSpacing(2);
            auto *titleLabel = new QLabel(title, card);
            titleLabel->setObjectName(QStringLiteral("iotMonitorItemTitle"));
            titleLabel->setWordWrap(false);
            titleLabel->setMinimumWidth(0);
            titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
            auto *subtitleLabel = new QLabel(subtitle, card);
            subtitleLabel->setObjectName(QStringLiteral("iotMonitorItemSubtitle"));
            subtitleLabel->setWordWrap(false);
            subtitleLabel->setMinimumWidth(0);
            subtitleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
            texts->addStretch();
            texts->addWidget(titleLabel);
            texts->addWidget(subtitleLabel);
            texts->addStretch();
            layout->addLayout(texts, 1);

            auto *actions = new QVBoxLayout;
            actions->setContentsMargins(0, 0, 0, 0);
            actions->setSpacing(6);
            auto *removeHint = new QLabel(QStringLiteral("×"), card);
            removeHint->setObjectName(QStringLiteral("iotMonitorDeleteHint"));
            removeHint->setAlignment(Qt::AlignCenter);
            removeHint->setVisible(false);
            removeHint->setAttribute(Qt::WA_TransparentForMouseEvents);
            auto *statusLabel = new QLabel(status, card);
            statusLabel->setObjectName(QStringLiteral("iotMonitorItemState"));
            statusLabel->setProperty("kind", kind);
            statusLabel->setAlignment(Qt::AlignCenter);
            statusLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
            actions->addWidget(removeHint, 0, Qt::AlignRight);
            actions->addStretch();
            actions->addWidget(statusLabel, 0, Qt::AlignRight);
            layout->addLayout(actions);

            return card;
        };

        int rowIndex = 0;
        for (const QJsonValue &value : users) {
            const QJsonObject user = value.toObject();
            const QString username = user.value(QStringLiteral("username")).toString();
            const QStringList deviceIds = deviceIdsForUser(user);
            const bool hasDevices = !deviceIds.isEmpty();
            auto *userCard = makeMonitorItem(QStringLiteral("♙"),
                                             username,
                                             hasDevices ? tr("%1 thiết bị đang liên kết").arg(deviceIds.size())
                                                        : tr("Chưa có thiết bị"),
                                             hasDevices ? tr("User") : tr("Trống"),
                                             hasDevices ? QStringLiteral("user") : QStringLiteral("empty"),
                                             username);
            m_deviceMonitorGrid->addWidget(userCard, rowIndex, 0,
                                           Qt::AlignLeft | Qt::AlignTop);
            int deviceColumn = 1;
            for (const QString &deviceId : deviceIds) {
                auto *deviceCard = makeMonitorItem(QStringLiteral("▣"),
                                                   deviceId,
                                                   tr("Thuộc user %1").arg(username),
                                                   tr("Gỡ"),
                                                   QStringLiteral("device"),
                                                   username,
                                                   deviceId);
                m_deviceMonitorGrid->addWidget(deviceCard, rowIndex, deviceColumn,
                                               Qt::AlignLeft | Qt::AlignTop);
                ++deviceColumn;
            }
            ++rowIndex;
        }
        m_deviceMonitorGrid->setColumnStretch(0, 0);
        m_deviceMonitorGrid->setColumnStretch(1, 0);
        m_deviceMonitorGrid->setColumnStretch(2, 0);
        m_deviceMonitorGrid->setColumnStretch(3, 1);
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
    if (m_detailPanel)
        m_detailPanel->hide();
    if (enabled)
        emit refreshRequested();
}
