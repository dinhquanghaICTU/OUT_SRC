#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QWidget>

namespace Ui { class UserManagementPage; }

class QLabel;
class QPushButton;
class QGridLayout;
class QScrollArea;

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
    void releaseUserDeviceRequested(const QString &username, const QString &deviceId);
    void refreshRequested();

private:
    void setupCustomUI();
    void renderUserGrid();
    void openEditDialog(const QJsonObject &user = QJsonObject());
    void confirmDeleteUser(const QJsonObject &user);

    Ui::UserManagementPage *ui;

    QJsonArray m_users;
    bool m_adminEnabled = false;
    QString m_currentFilter = "all"; // all, admin, user

    // Filter Buttons
    QPushButton *m_filterAllBtn = nullptr;
    QPushButton *m_filterAdminBtn = nullptr;
    QPushButton *m_filterUserBtn = nullptr;

    // Grid Container
    QGridLayout *m_gridLayout = nullptr;
    QLabel *m_emptyLabel = nullptr;
};
