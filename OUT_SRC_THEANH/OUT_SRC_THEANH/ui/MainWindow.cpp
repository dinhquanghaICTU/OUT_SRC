#include "MainWindow.h"

#include "api/ApiClient.h"
#include "services/AuthService.h"
#include "services/SensorService.h"
#include "ui/dialogs/ErrorDialog.h"
#include "ui_MainWindow.h"
#include "ui/pages/DashboardPage.h"
#include "ui/pages/LoginPage.h"
#include "ui/pages/UserManagementPage.h"

#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QSettings>
#include <QStyle>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_apiClient(new ApiClient(this)),
      m_authService(new AuthService(m_apiClient, this)),
      m_sensorService(new SensorService(m_apiClient, this)),
      m_loginPage(new LoginPage(this)),
      m_dashboardPage(new DashboardPage(this)),
      m_userManagementPage(new UserManagementPage(this))
{
    ui->setupUi(this);

    ui->pages->addWidget(m_loginPage);
    ui->pages->addWidget(m_dashboardPage);
    ui->pages->addWidget(m_userManagementPage);
    ui->pages->setCurrentWidget(m_loginPage);
    ui->topHeaderBar->hide();

    connect(m_loginPage, &LoginPage::loginRequested,
            m_authService, &AuthService::login);

    connect(m_authService, &AuthService::authenticated, this, [this] {
        ui->topHeaderBar->show();
        ui->currentUserBadge->setText(
            m_authService->isAdmin()
                ? QStringLiteral("👑 %1 (Admin)").arg(m_authService->currentUsername())
                : QStringLiteral("👤 %1").arg(m_authService->currentUsername()));
        m_dashboardPage->setUsername(m_authService->currentUsername());
        ui->usersButton->setVisible(m_authService->isAdmin());
        m_userManagementPage->setAdminEnabled(m_authService->isAdmin());
        ui->pages->setCurrentWidget(m_dashboardPage);
        ui->dashboardButton->setChecked(true);

        if (m_authService->isOfflineMode()) {
            statusBar()->showMessage(
                tr("Đang dùng admin offline để chỉnh giao diện. Server chưa kết nối."),
                8000);
            return;
        }
        m_apiClient->requestMyDevice();
        m_apiClient->requestAvailableDevices();
    });

    connect(m_authService, &AuthService::authenticationFailed, this,
            [this](const QString &message) {
                ErrorDialog::showLoginError(this, message);
            });

    connect(m_sensorService, &SensorService::readingUpdated,
            m_dashboardPage, &DashboardPage::updateReading);
    connect(m_apiClient, &ApiClient::networkError, this,
            [this](const QString &message) { statusBar()->showMessage(message, 5000); });

    // Dashboard Device Claims & Control
    connect(m_dashboardPage, &DashboardPage::claimDeviceRequested,
            m_apiClient, &ApiClient::claimDevice);
    connect(m_dashboardPage, &DashboardPage::releaseDeviceRequested,
            m_apiClient, &ApiClient::releaseDevice);
    connect(m_dashboardPage, &DashboardPage::relayControlRequested,
            m_apiClient, &ApiClient::setRelayState);
    connect(m_dashboardPage, &DashboardPage::refreshDevicesRequested,
            m_apiClient, &ApiClient::requestAvailableDevices);

    connect(m_apiClient, &ApiClient::availableDevicesReceived,
            m_dashboardPage, &DashboardPage::setAvailableDevices);
    connect(m_apiClient, &ApiClient::devicesReceived,
            m_dashboardPage, &DashboardPage::setOwnedDevices);

    connect(m_apiClient, &ApiClient::deviceClaimed, this,
            [this](const QJsonObject &) {
                if (m_authService->isOfflineMode())
                    return;
                m_apiClient->requestMyDevice();
                m_apiClient->requestAvailableDevices();
                statusBar()->showMessage(tr("Thêm thiết bị thành công"), 5000);
            });
    connect(m_apiClient, &ApiClient::deviceReleased, this,
            [this](const QString &) {
                if (m_authService->isOfflineMode())
                    return;
                m_apiClient->requestMyDevice();
                m_apiClient->requestAvailableDevices();
                statusBar()->showMessage(tr("Đã xóa thiết bị khỏi tài khoản"), 5000);
            });
    connect(m_apiClient, &ApiClient::relayCommandAccepted, this,
            [this](const QString &) {
                if (m_authService->isOfflineMode())
                    return;
                statusBar()->showMessage(tr("Đã gửi lệnh relay, đang chờ thiết bị xác nhận"), 3000);
                QTimer::singleShot(450, m_apiClient, &ApiClient::requestMyDevice);
            });

    // User Management
    connect(m_userManagementPage, &UserManagementPage::createUserRequested,
            m_apiClient, &ApiClient::createUser);
    connect(m_userManagementPage, &UserManagementPage::updateUserRequested,
            m_apiClient, &ApiClient::updateUser);
    connect(m_userManagementPage, &UserManagementPage::deleteUserRequested,
            m_apiClient, &ApiClient::deleteUser);
    connect(m_userManagementPage, &UserManagementPage::releaseUserDeviceRequested,
            m_apiClient, &ApiClient::releaseUserDevice);
    connect(m_userManagementPage, &UserManagementPage::refreshRequested,
            m_apiClient, &ApiClient::requestUsers);
    connect(m_apiClient, &ApiClient::usersReceived,
            m_userManagementPage, &UserManagementPage::setUsers);
    connect(m_apiClient, &ApiClient::userCreated, this,
            [this] {
                if (m_authService->isOfflineMode())
                    return;
                statusBar()->showMessage(tr("Tạo tài khoản thành công"), 5000);
                m_apiClient->requestUsers();
            });
    connect(m_apiClient, &ApiClient::userUpdated, this,
            [this] {
                if (m_authService->isOfflineMode())
                    return;
                statusBar()->showMessage(tr("Cập nhật tài khoản thành công"), 5000);
                m_apiClient->requestUsers();
            });
    connect(m_apiClient, &ApiClient::userDeleted, this,
            [this] {
                if (m_authService->isOfflineMode())
                    return;
                statusBar()->showMessage(tr("Xóa tài khoản thành công"), 5000);
                m_apiClient->requestUsers();
                m_apiClient->requestAvailableDevices();
            });
    connect(m_apiClient, &ApiClient::userDeviceReleased, this,
            [this](const QString &, const QString &deviceId) {
                if (m_authService->isOfflineMode())
                    return;
                statusBar()->showMessage(tr("Đã gỡ thiết bị %1 khỏi tài khoản").arg(deviceId), 5000);
                m_apiClient->requestUsers();
                m_apiClient->requestAvailableDevices();
            });
    connect(m_apiClient, &ApiClient::operationFailed, this,
            [this](const QString &message) {
                if (m_authService->isOfflineMode()) {
                    statusBar()->showMessage(tr("Server chưa kết nối trong chế độ offline UI"), 3000);
                    return;
                }
                statusBar()->showMessage(message, 5000);
                m_apiClient->requestMyDevice();
            });

    // Top Header Navigation Tab Buttons
    connect(ui->dashboardButton, &QPushButton::clicked, this,
            [this] { ui->pages->setCurrentWidget(m_dashboardPage); });
    connect(ui->usersButton, &QPushButton::clicked, this,
            [this] {
                ui->pages->setCurrentWidget(m_userManagementPage);
                if (m_authService->isOfflineMode())
                    return;
                m_apiClient->requestUsers();
            });
    connect(ui->logoutButton, &QPushButton::clicked, this, [this] {
        m_sensorService->stop();
        m_authService->logout();
        ui->topHeaderBar->hide();
        ui->pages->setCurrentWidget(m_loginPage);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
