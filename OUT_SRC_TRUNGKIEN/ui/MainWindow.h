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

    void triggerLogin(const QString &username, const QString &password, int targetPageIndex = 0);
    HistoryPage *historyPage() const { return m_historyPage; }

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setSidebarExpanded(bool expanded);
    void setCompactNavigation(bool compact);
    void setShortHeight(bool shortHeight);
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
