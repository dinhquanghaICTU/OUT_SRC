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
#include <QStatusBar>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_apiClient(new ApiClient(this)),
      m_authService(new AuthService(m_apiClient, this)),
      m_sensorService(new SensorService(m_apiClient, this)),
      m_loginPage(new LoginPage(this)),
      m_dashboardPage(new DashboardPage(this)),
      m_userManagementPage(new UserManagementPage(this)),
      m_pollTimer(new QTimer(this))
{
    ui->setupUi(this);

    ui->pages->addWidget(m_loginPage);
    ui->pages->addWidget(m_dashboardPage);
    ui->pages->addWidget(m_userManagementPage);
    ui->pages->setCurrentWidget(m_loginPage);
    ui->topHeaderBar->hide();

    // Navigation
    connect(ui->dashboardButton, &QPushButton::clicked, this, [this] {
        ui->pages->setCurrentWidget(m_dashboardPage);
        ui->dashboardButton->setChecked(true);
        ui->usersButton->setChecked(false);
    });

    connect(ui->usersButton, &QPushButton::clicked, this, [this] {
        ui->pages->setCurrentWidget(m_userManagementPage);
        ui->dashboardButton->setChecked(false);
        ui->usersButton->setChecked(true);
        m_apiClient->requestUsers();
    });

    connect(ui->logoutButton, &QPushButton::clicked, this, [this] {
        m_pollTimer->stop();
        m_authService->logout();
        ui->topHeaderBar->hide();
        ui->pages->setCurrentWidget(m_loginPage);
    });

    // Auth
    connect(m_loginPage, &LoginPage::loginRequested,
            m_authService, &AuthService::login);

    connect(m_authService, &AuthService::authenticated, this, [this] {
        ui->topHeaderBar->show();
        m_dashboardPage->setUsername(m_authService->currentUsername());
        ui->roleBadgeLabel->setText(m_authService->isAdmin() ? QStringLiteral("👑 Admin") : QStringLiteral("👤 User"));
        ui->usersButton->setVisible(m_authService->isAdmin());
        m_userManagementPage->setAdminEnabled(m_authService->isAdmin());
        ui->pages->setCurrentWidget(m_dashboardPage);
        ui->dashboardButton->setChecked(true);
        ui->usersButton->setChecked(false);

        m_apiClient->requestMyDevice();
        m_apiClient->requestAvailableDevices();
        m_pollTimer->start(1500);
    });

    connect(m_authService, &AuthService::authenticationFailed, this,
            [this](const QString &message) {
                ErrorDialog::showLoginError(this, message);
            });

    // Dashboard Telemetry & Device controls
    connect(m_dashboardPage, &DashboardPage::claimDeviceRequested,
            m_apiClient, &ApiClient::claimDevice);
    connect(m_dashboardPage, &DashboardPage::releaseDeviceRequested,
            m_apiClient, &ApiClient::releaseDevice);
    connect(m_dashboardPage, &DashboardPage::relayControlRequested,
            m_apiClient, &ApiClient::setRelayState);
    connect(m_dashboardPage, &DashboardPage::refreshDevicesRequested, this, [this] {
        m_apiClient->requestAvailableDevices();
        m_apiClient->requestMyDevice();
    });

    connect(m_apiClient, &ApiClient::availableDevicesReceived,
            m_dashboardPage, &DashboardPage::setAvailableDevices);
    connect(m_apiClient, &ApiClient::devicesReceived,
            m_dashboardPage, &DashboardPage::setOwnedDevices);
    connect(m_apiClient, &ApiClient::deviceClaimed, this, [this] {
        m_apiClient->requestMyDevice();
    });
    connect(m_apiClient, &ApiClient::deviceReleased, this, [this] {
        m_apiClient->requestMyDevice();
    });
    connect(m_apiClient, &ApiClient::relayCommandAccepted, this, [this] {
        m_apiClient->requestMyDevice();
    });

    // Periodic telemetry polling
    connect(m_pollTimer, &QTimer::timeout, this, [this] {
        m_apiClient->requestMyDevice();
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
    connect(m_apiClient, &ApiClient::userCreated, this, [this] {
        m_apiClient->requestUsers();
    });
    connect(m_apiClient, &ApiClient::userUpdated, this, [this] {
        m_apiClient->requestUsers();
    });
    connect(m_apiClient, &ApiClient::userDeleted, this, [this] {
        m_apiClient->requestUsers();
    });
    connect(m_apiClient, &ApiClient::userDeviceReleased, this, [this] {
        m_apiClient->requestUsers();
    });

    connect(m_apiClient, &ApiClient::networkError, this, [this](const QString &msg) {
        statusBar()->showMessage(msg, 4000);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
