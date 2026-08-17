#pragma once

#include <QMainWindow>

namespace Ui { class MainWindow; }

class ApiClient;
class AuthService;
class SensorService;
class LoginPage;
class DashboardPage;
class UserManagementPage;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    ApiClient *m_apiClient;
    AuthService *m_authService;
    SensorService *m_sensorService;
    LoginPage *m_loginPage;
    DashboardPage *m_dashboardPage;
    UserManagementPage *m_userManagementPage;
    QTimer *m_pollTimer = nullptr;
};
