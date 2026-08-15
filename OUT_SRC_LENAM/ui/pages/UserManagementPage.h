#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QWidget>

namespace Ui { class UserManagementPage; }
class QFrame;
class QVBoxLayout;
class QLabel;
class QGridLayout;

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
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showUserDetails(const QJsonObject &user);
    void clearUserDetails();
    void openEditDialog(const QJsonObject &user);
    void confirmDeleteUser(const QJsonObject &user);
    bool confirmReleaseDevice(const QString &username, const QString &deviceId);

    Ui::UserManagementPage *ui;
    QJsonArray m_users;
    QJsonObject m_selectedUser;
    QLabel *m_totalUsersLabel = nullptr;
    QLabel *m_onlineDevicesLabel = nullptr;
    QLabel *m_offlineDevicesLabel = nullptr;
    QFrame *m_detailPanel = nullptr;
    QGridLayout *m_deviceMonitorGrid = nullptr;
    QVBoxLayout *m_detailLayout = nullptr;
};
