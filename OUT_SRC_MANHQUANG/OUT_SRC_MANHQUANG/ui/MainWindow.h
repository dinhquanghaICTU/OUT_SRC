#pragma once

#include <QMainWindow>
#include <QTimer>

namespace Ui { class MainWindow; }

class ApiClient;
class AuthService;
class SensorService;
class LoginPage;
class DashboardPage;
class DeviceManagementPage;
class HistoryPage;
class AlertPage;
class UserManagementPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onDoorCommandFromDashboard(const QString &action);

private:
    void setupNavigation();
    void updateClock();

    Ui::MainWindow *ui;
    ApiClient *m_apiClient;
    AuthService *m_authService;
    SensorService *m_sensorService;

    LoginPage *m_loginPage;
    DashboardPage *m_dashboardPage;
    DeviceManagementPage *m_deviceManagementPage;
    HistoryPage *m_historyPage;
    AlertPage *m_alertPage;
    UserManagementPage *m_userManagementPage;

    QTimer m_clockTimer;
    QString m_activeDeviceId = QStringLiteral("manhquang-190782");
};
