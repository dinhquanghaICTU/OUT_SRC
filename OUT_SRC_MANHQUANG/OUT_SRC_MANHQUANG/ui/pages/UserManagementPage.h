#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QWidget>

namespace Ui { class UserManagementPage; }

class UserManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagementPage(QWidget *parent = nullptr);
    ~UserManagementPage() override;

    void setUsers(const QJsonArray &users);
    void setAdminEnabled(bool enabled);

signals:
    void createUserRequested(const QString &username, const QString &password, const QString &role);
    void updateUserRequested(const QString &oldUsername, const QString &username,
                             const QString &password, const QString &role, bool enabled);
    void deleteUserRequested(const QString &username);
    void refreshRequested();

private:
    void openAddUserDialog();
    void openEditUserDialog(const QJsonObject &user);

    Ui::UserManagementPage *ui;
    QJsonArray m_users;
    bool m_adminEnabled = false;
};
