#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QWidget>

namespace Ui { class UserManagementPage; }
class QFrame;
class QVBoxLayout;

class UserManagementPage : public QWidget
{
    Q_OBJECT
public:
    explicit UserManagementPage(QWidget *parent = nullptr);
    ~UserManagementPage() override;
    void setUsers(const QJsonArray &users);
    void setAdminEnabled(bool enabled);

signals:
    void createUserRequested(const QString &username, const QString &password,
                             const QString &role);
    void updateUserRequested(const QString &oldUsername, const QString &username,
                             const QString &password, const QString &role, bool enabled);
    void deleteUserRequested(const QString &username);
    void releaseUserDeviceRequested(const QString &username, const QString &deviceId);
    void refreshRequested();

private:
    void showUserDetails(const QJsonObject &user);
    void clearUserDetails();
    void openEditDialog(const QJsonObject &user);
    void confirmDeleteUser(const QJsonObject &user);

    Ui::UserManagementPage *ui;
    QJsonArray m_users;
    QJsonObject m_selectedUser;
    QFrame *m_detailPanel = nullptr;
    QVBoxLayout *m_detailLayout = nullptr;
};
