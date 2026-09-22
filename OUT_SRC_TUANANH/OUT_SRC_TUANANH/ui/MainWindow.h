#pragma once

#include <QMainWindow>

namespace Ui { class MainWindow; }

class ApiClient;
class AuthService;
class SensorService;
class LoginPage;
class DashboardPage;
class DeviceManagementPage;
class HistoryPage;
class UserManagementPage;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void autoLogin(const QString &username = QStringLiteral("admin"),
                   const QString &password = QStringLiteral("admin123"));
    void navigateToPage(int index);

    DashboardPage *dashboardPage() const { return m_dashboardPage; }
    DeviceManagementPage *deviceManagementPage() const { return m_deviceManagementPage; }
    HistoryPage *historyPage() const { return m_historyPage; }
    UserManagementPage *userManagementPage() const { return m_userManagementPage; }

private:
    Ui::MainWindow *ui;
    ApiClient *m_apiClient;
    AuthService *m_authService;
    SensorService *m_sensorService;
    LoginPage *m_loginPage;
    DashboardPage *m_dashboardPage;
    DeviceManagementPage *m_deviceManagementPage;
    HistoryPage *m_historyPage;
    UserManagementPage *m_userManagementPage;
    QTimer *m_pollTimer = nullptr;
};
