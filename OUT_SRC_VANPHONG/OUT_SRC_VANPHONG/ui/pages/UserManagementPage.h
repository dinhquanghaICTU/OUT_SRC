#pragma once

#include <QJsonArray>
#include <QWidget>

namespace Ui { class UserManagementPage; }

class UserManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagementPage(QWidget *parent = nullptr);
    ~UserManagementPage() override;

    void setAdminEnabled(bool enabled);
    void setUsers(const QJsonArray &users);

signals:
    void createUserRequested(const QString &username, const QString &password, const QString &role);
    void updateUserRequested(const QString &oldUsername, const QString &newUsername, const QString &password, const QString &role, bool enabled);
    void deleteUserRequested(const QString &username);

private:
    void openAddUserDialog();
    void openEditUserDialog(int row);

    Ui::UserManagementPage *ui;
    QJsonArray m_users;
};
