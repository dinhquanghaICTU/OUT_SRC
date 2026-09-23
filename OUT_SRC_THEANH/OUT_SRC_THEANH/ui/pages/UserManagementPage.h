#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QWidget>

namespace Ui { class UserManagementPage; }

class QLabel;
class QPushButton;
class QGridLayout;
class QScrollArea;
class QTableWidget;
class QStackedWidget;
class QLineEdit;

class UserManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagementPage(QWidget *parent = nullptr);
    ~UserManagementPage() override;
    void setUsers(const QJsonArray &users);
    void setLoginHistory(const QJsonArray &history);
    void setAuditLogs(const QJsonArray &logs);
    void setCurrentUsername(const QString &username);
    void setAdminEnabled(bool enabled);
    void openEditDialog(const QJsonObject &user = QJsonObject());

signals:
    void backToDashboardRequested();
    void createUserRequested(const QString &username, const QString &password, const QString &role);
    void updateUserRequested(const QString &oldUsername, const QString &username,
                             const QString &password, const QString &role, bool enabled);
    void deleteUserRequested(const QString &username);
    void releaseUserDeviceRequested(const QString &username, const QString &deviceId);
    void refreshRequested();
    void requestLoginHistoryRequested();
    void requestAuditLogsRequested();

private:
    void setupCustomUI();
    void renderUserGrid();
    void renderLoginHistory();
    void renderAuditLogs();
    void confirmDeleteUser(const QJsonObject &user);

    Ui::UserManagementPage *ui;

    QJsonArray m_users;
    QJsonArray m_loginHistory;
    QJsonArray m_auditLogs;
    bool m_adminEnabled = false;
    QString m_currentUsername;
    QString m_currentFilter = "all"; // all, admin, user
    QString m_auditFilterText;

    // Subtab Buttons
    QPushButton *m_tabUsersBtn = nullptr;
    QPushButton *m_tabLoginBtn = nullptr;
    QPushButton *m_tabAuditBtn = nullptr;
    QPushButton *m_addUserBtn = nullptr;
    QPushButton *m_refreshBtn = nullptr;

    QStackedWidget *m_stack = nullptr;

    // Page 0: Users
    QPushButton *m_filterAllBtn = nullptr;
    QPushButton *m_filterAdminBtn = nullptr;
    QPushButton *m_filterUserBtn = nullptr;
    QGridLayout *m_gridLayout = nullptr;
    QLabel *m_emptyLabel = nullptr;

    // Page 1: Login History
    QLabel *m_loginSummaryLabel = nullptr;
    QTableWidget *m_loginTable = nullptr;

    // Page 2: Audit Logs
    QLabel *m_auditSummaryLabel = nullptr;
    QLineEdit *m_auditSearchEdit = nullptr;
    QTableWidget *m_auditTable = nullptr;
};
