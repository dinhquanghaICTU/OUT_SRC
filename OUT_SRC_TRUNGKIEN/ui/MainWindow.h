#pragma once

#include <QMainWindow>

class QResizeEvent;

namespace Ui { class MainWindow; }
class ApiClient;
class AuthService;
class DashboardPage;
class DeviceManagementPage;
class HistoryPage;
class LoginPage;
class QPushButton;
class SensorService;
class UserManagementPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setSidebarExpanded(bool expanded);
    void refreshSidebarButtonText();

    Ui::MainWindow *ui;
    ApiClient *m_apiClient;
    AuthService *m_authService;
    SensorService *m_sensorService;
    LoginPage *m_loginPage;
    DashboardPage *m_dashboardPage;
    DeviceManagementPage *m_deviceManagementPage;
    HistoryPage *m_historyPage;
    UserManagementPage *m_userManagementPage;
    QPushButton *m_sidebarToggleButton = nullptr;
    bool m_sidebarExpanded = false;
};
