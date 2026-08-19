#include "UserManagementPage.h"
#include "ui_UserManagementPage.h"

#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

UserManagementPage::UserManagementPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::UserManagementPage)
{
    ui->setupUi(this);
    ui->usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->usersTable->verticalHeader()->hide();

    connect(ui->addUserButton, &QPushButton::clicked, this, &UserManagementPage::openAddUserDialog);
}

UserManagementPage::~UserManagementPage()
{
    delete ui;
}

void UserManagementPage::setAdminEnabled(bool enabled)
{
    m_adminEnabled = enabled;
    ui->addUserButton->setEnabled(enabled);
}

void UserManagementPage::setUsers(const QJsonArray &users)
{
    m_users = users;
    ui->usersTable->setRowCount(0);

    for (const auto &val : users) {
        const QJsonObject user = val.toObject();
        const QString username = user.value(QStringLiteral("username")).toString();
        const QString role = user.value(QStringLiteral("role")).toString();
        const QString devId = user.value(QStringLiteral("device_id")).toString(QStringLiteral("--"));
        const bool enabled = user.value(QStringLiteral("enabled")).toBool(true);

        const int row = ui->usersTable->rowCount();
        ui->usersTable->insertRow(row);

        ui->usersTable->setItem(row, 0, new QTableWidgetItem(username));
        ui->usersTable->setItem(row, 1, new QTableWidgetItem(role.toUpper()));
        ui->usersTable->setItem(row, 2, new QTableWidgetItem(devId));

        auto *statusItem = new QTableWidgetItem(enabled ? tr("🟢 Hoạt động") : tr("🔴 Bị khóa"));
        statusItem->setForeground(enabled ? QColor(QStringLiteral("#10b981")) : QColor(QStringLiteral("#ef4444")));
        ui->usersTable->setItem(row, 3, statusItem);

        auto *actionWidget = new QWidget;
        auto *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(2, 2, 2, 2);
        actionLayout->setSpacing(4);

        auto *editBtn = new QPushButton(tr("✏️ Sửa"), actionWidget);
        editBtn->setStyleSheet(QStringLiteral("background: #0284c7; color: white; border: none; border-radius: 4px; padding: 2px 8px; font-size: 10px; font-weight: 600;"));
        connect(editBtn, &QPushButton::clicked, this, [this, user] { openEditUserDialog(user); });
        actionLayout->addWidget(editBtn);

        if (username != QStringLiteral("admin")) {
            auto *delBtn = new QPushButton(tr("🗑️ Xóa"), actionWidget);
            delBtn->setStyleSheet(QStringLiteral("background: #ef4444; color: white; border: none; border-radius: 4px; padding: 2px 8px; font-size: 10px; font-weight: 600;"));
            connect(delBtn, &QPushButton::clicked, this, [this, username] {
                if (QMessageBox::question(this, tr("Xác nhận xóa"), tr("Xóa tài khoản %1?").arg(username)) == QMessageBox::Yes) {
                    emit deleteUserRequested(username);
                }
            });
            actionLayout->addWidget(delBtn);
        }

        ui->usersTable->setCellWidget(row, 4, actionWidget);
    }
}

void UserManagementPage::openAddUserDialog()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Thêm tài khoản mới"));
    dlg.setStyleSheet(QStringLiteral("background-color: #0f172a; color: white;"));
    dlg.resize(340, 240);

    auto *layout = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;

    auto *userEdit = new QLineEdit(&dlg);
    userEdit->setStyleSheet(QStringLiteral("background: #1e293b; color: white; padding: 6px; border-radius: 6px;"));
    form->addRow(tr("Tên đăng nhập:"), userEdit);

    auto *passEdit = new QLineEdit(&dlg);
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setStyleSheet(QStringLiteral("background: #1e293b; color: white; padding: 6px; border-radius: 6px;"));
    form->addRow(tr("Mật khẩu:"), passEdit);

    auto *roleCombo = new QComboBox(&dlg);
    roleCombo->addItems({QStringLiteral("admin"), QStringLiteral("operator"), QStringLiteral("user")});
    roleCombo->setStyleSheet(QStringLiteral("background: #1e293b; color: white; padding: 6px; border-radius: 6px;"));
    form->addRow(tr("Vai trò:"), roleCombo);

    layout->addLayout(form);

    auto *btnRow = new QHBoxLayout;
    auto *cancelBtn = new QPushButton(tr("Hủy"), &dlg);
    cancelBtn->setStyleSheet(QStringLiteral("background: #334155; color: white; padding: 6px 14px; border-radius: 6px;"));
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    auto *saveBtn = new QPushButton(tr("Tạo tài khoản"), &dlg);
    saveBtn->setStyleSheet(QStringLiteral("background: #10b981; color: white; padding: 6px 14px; border-radius: 6px; font-weight: 700;"));
    connect(saveBtn, &QPushButton::clicked, [&] {
        if (userEdit->text().trimmed().isEmpty() || passEdit->text().isEmpty()) {
            QMessageBox::warning(&dlg, tr("Lỗi"), tr("Vui lòng điền đủ thông tin"));
            return;
        }
        emit createUserRequested(userEdit->text().trimmed(), passEdit->text(), roleCombo->currentText());
        dlg.accept();
    });
    btnRow->addWidget(saveBtn);
    layout->addLayout(btnRow);

    dlg.exec();
}

void UserManagementPage::openEditUserDialog(const QJsonObject &user)
{
    const QString oldUsername = user.value(QStringLiteral("username")).toString();

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Chỉnh sửa tài khoản"));
    dlg.setStyleSheet(QStringLiteral("background-color: #0f172a; color: white;"));
    dlg.resize(340, 260);

    auto *layout = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;

    auto *userEdit = new QLineEdit(oldUsername, &dlg);
    userEdit->setStyleSheet(QStringLiteral("background: #1e293b; color: white; padding: 6px; border-radius: 6px;"));
    form->addRow(tr("Tên đăng nhập:"), userEdit);

    auto *passEdit = new QLineEdit(&dlg);
    passEdit->setPlaceholderText(tr("Để trống nếu không đổi"));
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setStyleSheet(QStringLiteral("background: #1e293b; color: white; padding: 6px; border-radius: 6px;"));
    form->addRow(tr("Mật khẩu mới:"), passEdit);

    auto *roleCombo = new QComboBox(&dlg);
    roleCombo->addItems({QStringLiteral("admin"), QStringLiteral("operator"), QStringLiteral("user")});
    roleCombo->setCurrentText(user.value(QStringLiteral("role")).toString());
    roleCombo->setStyleSheet(QStringLiteral("background: #1e293b; color: white; padding: 6px; border-radius: 6px;"));
    form->addRow(tr("Vai trò:"), roleCombo);

    layout->addLayout(form);

    auto *btnRow = new QHBoxLayout;
    auto *cancelBtn = new QPushButton(tr("Hủy"), &dlg);
    cancelBtn->setStyleSheet(QStringLiteral("background: #334155; color: white; padding: 6px 14px; border-radius: 6px;"));
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    auto *saveBtn = new QPushButton(tr("Lưu thay đổi"), &dlg);
    saveBtn->setStyleSheet(QStringLiteral("background: #0284c7; color: white; padding: 6px 14px; border-radius: 6px; font-weight: 700;"));
    connect(saveBtn, &QPushButton::clicked, [&] {
        emit updateUserRequested(oldUsername, userEdit->text().trimmed(), passEdit->text(), roleCombo->currentText(), true);
        dlg.accept();
    });
    btnRow->addWidget(saveBtn);
    layout->addLayout(btnRow);

    dlg.exec();
}
