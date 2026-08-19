#include "UserManagementPage.h"
#include "ui_UserManagementPage.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

UserManagementPage::UserManagementPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::UserManagementPage)
{
    ui->setupUi(this);

    ui->userTable->setColumnCount(5);
    ui->userTable->setHorizontalHeaderLabels({
        tr("ID"), tr("Tên đăng nhập"), tr("Vai trò"), tr("Trạng thái"), tr("Thao tác")
    });
    ui->userTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->userTable->verticalHeader()->hide();

    connect(ui->btnAddUser, &QPushButton::clicked, this, &UserManagementPage::openAddUserDialog);
}

UserManagementPage::~UserManagementPage()
{
    delete ui;
}

void UserManagementPage::setAdminEnabled(bool enabled)
{
    ui->btnAddUser->setVisible(enabled);
}

void UserManagementPage::setUsers(const QJsonArray &users)
{
    m_users = users;
    ui->userTable->setRowCount(0);

    for (int i = 0; i < users.size(); ++i) {
        const QJsonObject u = users.at(i).toObject();
        const int row = ui->userTable->rowCount();
        ui->userTable->insertRow(row);

        const QString username = u.value(QStringLiteral("username")).toString();
        const QString role = u.value(QStringLiteral("role")).toString();
        const bool enabled = u.value(QStringLiteral("enabled")).toBool(true);

        ui->userTable->setItem(row, 0, new QTableWidgetItem(QString::number(u.value(QStringLiteral("id")).toInt())));
        ui->userTable->setItem(row, 1, new QTableWidgetItem(username));
        ui->userTable->setItem(row, 2, new QTableWidgetItem(role == QStringLiteral("admin") ? tr("👑 Quản trị viên") : tr("👨‍🌾 Vận hành vườn")));
        ui->userTable->setItem(row, 3, new QTableWidgetItem(enabled ? tr("🟢 Hoạt động") : tr("⚪ Khóa")));

        auto *btnWidget = new QWidget;
        auto *btnLayout = new QHBoxLayout(btnWidget);
        btnLayout->setContentsMargins(2, 2, 2, 2);
        btnLayout->setSpacing(4);

        auto *editBtn = new QPushButton(tr("✏️ Sửa"), btnWidget);
        editBtn->setStyleSheet(QStringLiteral("background: #0284c7; color: white; border: none; border-radius: 4px; padding: 2px 6px; font-size: 10px;"));
        connect(editBtn, &QPushButton::clicked, this, [this, i] { openEditUserDialog(i); });
        btnLayout->addWidget(editBtn);

        if (username != QStringLiteral("admin")) {
            auto *delBtn = new QPushButton(tr("🗑️ Xóa"), btnWidget);
            delBtn->setStyleSheet(QStringLiteral("background: #dc2626; color: white; border: none; border-radius: 4px; padding: 2px 6px; font-size: 10px;"));
            connect(delBtn, &QPushButton::clicked, this, [this, username] {
                if (QMessageBox::question(this, tr("Xác nhận xóa"), tr("Xóa tài khoản %1?").arg(username)) == QMessageBox::Yes) {
                    emit deleteUserRequested(username);
                }
            });
            btnLayout->addWidget(delBtn);
        }

        ui->userTable->setCellWidget(row, 4, btnWidget);
    }
}

void UserManagementPage::openAddUserDialog()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Thêm tài khoản vườn"));
    dlg.setStyleSheet(QStringLiteral("background-color: #0c2317; color: #f1f5f9;"));

    auto *layout = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;

    auto *uEdit = new QLineEdit(&dlg);
    uEdit->setStyleSheet(QStringLiteral("background-color: #07170e; color: white; border: 1px solid #1b4332; border-radius: 4px; padding: 4px;"));
    form->addRow(tr("Tên đăng nhập:"), uEdit);

    auto *pEdit = new QLineEdit(&dlg);
    pEdit->setEchoMode(QLineEdit::Password);
    pEdit->setStyleSheet(QStringLiteral("background-color: #07170e; color: white; border: 1px solid #1b4332; border-radius: 4px; padding: 4px;"));
    form->addRow(tr("Mật khẩu:"), pEdit);

    auto *roleCombo = new QComboBox(&dlg);
    roleCombo->addItems({QStringLiteral("operator"), QStringLiteral("admin"), QStringLiteral("user")});
    roleCombo->setStyleSheet(QStringLiteral("background-color: #07170e; color: white; border: 1px solid #1b4332; border-radius: 4px; padding: 4px;"));
    form->addRow(tr("Vai trò:"), roleCombo);

    layout->addLayout(form);

    auto *bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(bbox);
    connect(bbox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bbox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        if (!uEdit->text().trimmed().isEmpty() && !pEdit->text().isEmpty()) {
            emit createUserRequested(uEdit->text().trimmed(), pEdit->text(), roleCombo->currentText());
        }
    }
}

void UserManagementPage::openEditUserDialog(int row)
{
    if (row < 0 || row >= m_users.size()) return;
    const QJsonObject u = m_users.at(row).toObject();
    const QString oldUser = u.value(QStringLiteral("username")).toString();

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Chỉnh sửa tài khoản"));
    dlg.setStyleSheet(QStringLiteral("background-color: #0c2317; color: #f1f5f9;"));

    auto *layout = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;

    auto *uEdit = new QLineEdit(oldUser, &dlg);
    uEdit->setStyleSheet(QStringLiteral("background-color: #07170e; color: white; border: 1px solid #1b4332; border-radius: 4px; padding: 4px;"));
    form->addRow(tr("Tên đăng nhập:"), uEdit);

    auto *pEdit = new QLineEdit(&dlg);
    pEdit->setEchoMode(QLineEdit::Password);
    pEdit->setPlaceholderText(tr("Để trống nếu không đổi"));
    pEdit->setStyleSheet(QStringLiteral("background-color: #07170e; color: white; border: 1px solid #1b4332; border-radius: 4px; padding: 4px;"));
    form->addRow(tr("Mật khẩu mới:"), pEdit);

    auto *roleCombo = new QComboBox(&dlg);
    roleCombo->addItems({QStringLiteral("operator"), QStringLiteral("admin"), QStringLiteral("user")});
    roleCombo->setCurrentText(u.value(QStringLiteral("role")).toString());
    roleCombo->setStyleSheet(QStringLiteral("background-color: #07170e; color: white; border: 1px solid #1b4332; border-radius: 4px; padding: 4px;"));
    form->addRow(tr("Vai trò:"), roleCombo);

    layout->addLayout(form);

    auto *bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(bbox);
    connect(bbox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bbox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        emit updateUserRequested(oldUser, uEdit->text().trimmed(), pEdit->text(), roleCombo->currentText(), true);
    }
}
