#include "MainWindow.h"

#include "api/ApiClient.h"
#include "services/AuthService.h"
#include "services/SensorService.h"
#include "ui/dialogs/ErrorDialog.h"
#include "ui_MainWindow.h"
#include "ui/pages/DashboardPage.h"
#include "ui/pages/DeviceManagementPage.h"
#include "ui/pages/HistoryPage.h"
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
      m_deviceManagementPage(new DeviceManagementPage(this)),
      m_historyPage(new HistoryPage(this)),
      m_userManagementPage(new UserManagementPage(this)),
      m_pollTimer(new QTimer(this))
{
    ui->setupUi(this);

    ui->pages->addWidget(m_loginPage);            // Index 0
    ui->pages->addWidget(m_dashboardPage);        // Index 1
    ui->pages->addWidget(m_deviceManagementPage); // Index 2
    ui->pages->addWidget(m_historyPage);          // Index 3
    ui->pages->addWidget(m_userManagementPage);   // Index 4
    ui->pages->setCurrentWidget(m_loginPage);
    ui->topHeaderBar->hide();
    statusBar()->hide();

    // --- Navigation Tabs ---
    auto updateNavButtons = [this](QPushButton *active) {
        ui->dashboardButton->setChecked(active == ui->dashboardButton);
        ui->devicesButton->setChecked(active == ui->devicesButton);
        ui->historyButton->setChecked(active == ui->historyButton);
        ui->usersButton->setChecked(active == ui->usersButton);
    };

    connect(ui->dashboardButton, &QPushButton::clicked, this, [this, updateNavButtons] {
        ui->pages->setCurrentWidget(m_dashboardPage);
        updateNavButtons(ui->dashboardButton);
    });

    connect(ui->devicesButton, &QPushButton::clicked, this, [this, updateNavButtons] {
        ui->pages->setCurrentWidget(m_deviceManagementPage);
        updateNavButtons(ui->devicesButton);
        m_apiClient->requestMyDevice();
        m_apiClient->requestAvailableDevices();
    });

    connect(ui->historyButton, &QPushButton::clicked, this, [this, updateNavButtons] {
        ui->pages->setCurrentWidget(m_historyPage);
        updateNavButtons(ui->historyButton);
        m_apiClient->requestMyDevice();
    });

    connect(ui->usersButton, &QPushButton::clicked, this, [this, updateNavButtons] {
        ui->pages->setCurrentWidget(m_userManagementPage);
        updateNavButtons(ui->usersButton);
        if (m_authService->isAdmin()) {
            m_apiClient->requestUsers();
            m_apiClient->requestLoginHistory();
        }
        m_apiClient->requestAuditLogs();
    });

    connect(ui->logoutButton, &QPushButton::clicked, this, [this] {
        m_pollTimer->stop();
        m_deviceManagementPage->stopRealtime();
        m_authService->logout();
        ui->topHeaderBar->hide();
        ui->pages->setCurrentWidget(m_loginPage);
    });

    // --- Authentication ---
    connect(m_loginPage, &LoginPage::loginRequested,
            m_authService, &AuthService::login);

    connect(m_authService, &AuthService::authenticated, this, [this, updateNavButtons] {
        ui->topHeaderBar->show();
        m_dashboardPage->setUsername(m_authService->currentUsername());
        m_deviceManagementPage->setCurrentUser(m_authService->currentUsername(), m_authService->isAdmin());
        ui->roleBadgeLabel->setText(m_authService->isAdmin() ? tr("ADMIN") : tr("USER"));
        ui->usersButton->setVisible(true);
        if (m_authService->isAdmin()) {
            ui->usersButton->setText(tr("Quản trị & Nhật ký"));
            ui->usersButton->setToolTip(tr("Quản lý tài khoản, lịch sử đăng nhập & điều khiển hệ thống"));
        } else {
            ui->usersButton->setText(tr("Nhật ký thao tác"));
            ui->usersButton->setToolTip(tr("Xem lịch sử các thao tác của bạn trong hệ thống"));
        }
        m_userManagementPage->setCurrentUsername(m_authService->currentUsername());
        m_userManagementPage->setAdminEnabled(m_authService->isAdmin());

        ui->pages->setCurrentWidget(m_dashboardPage);
        updateNavButtons(ui->dashboardButton);

        m_apiClient->requestMyDevice();
        m_apiClient->requestAvailableDevices();
        m_deviceManagementPage->startRealtime();
        m_pollTimer->start(1500);
    });

    connect(m_authService, &AuthService::authenticationFailed, this,
            [this](const QString &message) {
                ErrorDialog::showLoginError(this, message);
            });

    // --- Dashboard Telemetry & Device Controls ---
    connect(m_dashboardPage, &DashboardPage::claimDeviceRequested,
            m_apiClient, &ApiClient::claimDevice);
    connect(m_dashboardPage, &DashboardPage::releaseDeviceRequested,
            m_apiClient, &ApiClient::releaseDevice);
    connect(m_dashboardPage, &DashboardPage::relayControlRequested,
            m_apiClient, &ApiClient::setRelayState);
    connect(m_dashboardPage, &DashboardPage::deviceConfigRequested,
            m_apiClient, &ApiClient::updatePerDeviceConfig);
    connect(m_dashboardPage, &DashboardPage::refreshDevicesRequested, this, [this] {
        m_apiClient->requestAvailableDevices();
        m_apiClient->requestMyDevice();
    });
    connect(m_dashboardPage, &DashboardPage::navigateToPageRequested, this, [this](int pageIdx) {
        navigateToPage(pageIdx);
    });

    // --- Device Management Page ---
    connect(m_deviceManagementPage, &DeviceManagementPage::claimDeviceRequested,
            m_apiClient, &ApiClient::claimDevice);
    connect(m_deviceManagementPage, &DeviceManagementPage::releaseDeviceRequested,
            m_apiClient, &ApiClient::releaseDevice);
    connect(m_deviceManagementPage, &DeviceManagementPage::relayControlRequested,
            m_apiClient, &ApiClient::setRelayState);
    connect(m_deviceManagementPage, &DeviceManagementPage::deviceConfigRequested,
            m_apiClient, &ApiClient::updatePerDeviceConfig);
    connect(m_deviceManagementPage, &DeviceManagementPage::refreshRequested, this, [this] {
        m_apiClient->requestAvailableDevices();
        m_apiClient->requestMyDevice();
    });

    // --- History Page ---
    connect(m_historyPage, &HistoryPage::historyRequested,
            m_apiClient, &ApiClient::requestDeviceHistory);
    connect(m_apiClient, &ApiClient::deviceHistoryReceived,
            m_historyPage, &HistoryPage::setHistory);

    // --- ApiClient Broadcasts ---
    connect(m_apiClient, &ApiClient::availableDevicesReceived,
            m_dashboardPage, &DashboardPage::setAvailableDevices);
    connect(m_apiClient, &ApiClient::availableDevicesReceived,
            m_deviceManagementPage, &DeviceManagementPage::setAvailableDevices);

    connect(m_apiClient, &ApiClient::devicesReceived,
            m_dashboardPage, &DashboardPage::setOwnedDevices);
    connect(m_apiClient, &ApiClient::devicesReceived,
            m_deviceManagementPage, &DeviceManagementPage::setOwnedDevices);
    connect(m_apiClient, &ApiClient::devicesReceived,
            m_historyPage, &HistoryPage::setDevices);

    connect(m_apiClient, &ApiClient::deviceClaimed, this, [this] {
        m_apiClient->requestMyDevice();
        m_apiClient->requestAvailableDevices();
        statusBar()->showMessage(tr("Đã thêm thiết bị thành công"), 3000);
    });
    connect(m_apiClient, &ApiClient::deviceReleased, this, [this] {
        m_apiClient->requestMyDevice();
        m_apiClient->requestAvailableDevices();
        statusBar()->showMessage(tr("Đã xóa thiết bị khỏi tài khoản"), 3000);
    });
    connect(m_apiClient, &ApiClient::relayCommandAccepted, this, [this] {
        statusBar()->showMessage(tr("Lệnh điều khiển đèn đã được gửi"), 2000);
        QTimer::singleShot(400, m_apiClient, &ApiClient::requestMyDevice);
    });
    connect(m_apiClient, &ApiClient::deviceConfigSaved,
            m_deviceManagementPage, &DeviceManagementPage::configSaved);
    connect(m_apiClient, &ApiClient::deviceConfigSaved, this, [this] {
        statusBar()->showMessage(tr("Đã lưu cấu hình ngưỡng chiếu sáng"), 3000);
        m_apiClient->requestMyDevice();
    });

    // Periodic telemetry polling
    connect(m_pollTimer, &QTimer::timeout, this, [this] {
        m_apiClient->requestMyDevice();
    });

    // --- User Management ---
    connect(m_userManagementPage, &UserManagementPage::createUserRequested,
            m_apiClient, &ApiClient::createUser);
    connect(m_userManagementPage, &UserManagementPage::updateUserRequested,
            m_apiClient, &ApiClient::updateUser);
    connect(m_userManagementPage, &UserManagementPage::deleteUserRequested,
            m_apiClient, &ApiClient::deleteUser);
    connect(m_userManagementPage, &UserManagementPage::releaseUserDeviceRequested,
            m_apiClient, &ApiClient::releaseUserDevice);
    connect(m_userManagementPage, &UserManagementPage::backToDashboardRequested, this, [this, updateNavButtons] {
        ui->pages->setCurrentWidget(m_dashboardPage);
        updateNavButtons(ui->dashboardButton);
    });
    connect(m_userManagementPage, &UserManagementPage::refreshRequested,
            m_apiClient, &ApiClient::requestUsers);
    connect(m_userManagementPage, &UserManagementPage::requestLoginHistoryRequested,
            m_apiClient, [this] {
        m_apiClient->requestLoginHistory();
    });
    connect(m_userManagementPage, &UserManagementPage::requestAuditLogsRequested,
            m_apiClient, [this] {
        m_apiClient->requestAuditLogs();
    });

    connect(m_apiClient, &ApiClient::usersReceived,
            m_userManagementPage, &UserManagementPage::setUsers);
    connect(m_apiClient, &ApiClient::loginHistoryReceived,
            m_userManagementPage, &UserManagementPage::setLoginHistory);
    connect(m_apiClient, &ApiClient::auditLogsReceived,
            m_userManagementPage, &UserManagementPage::setAuditLogs);
    connect(m_apiClient, &ApiClient::userCreated, this, [this] {
        m_apiClient->requestUsers();
        statusBar()->showMessage(tr("Tạo tài khoản thành công"), 3000);
    });
    connect(m_apiClient, &ApiClient::userUpdated, this, [this] {
        m_apiClient->requestUsers();
        statusBar()->showMessage(tr("Cập nhật tài khoản thành công"), 3000);
    });
    connect(m_apiClient, &ApiClient::userDeleted, this, [this] {
        m_apiClient->requestUsers();
        statusBar()->showMessage(tr("Đã xóa tài khoản"), 3000);
    });
    connect(m_apiClient, &ApiClient::userDeviceReleased, this, [this] {
        m_apiClient->requestUsers();
        statusBar()->showMessage(tr("Đã gỡ thiết bị khỏi tài khoản"), 3000);
    });

    connect(m_apiClient, &ApiClient::networkError, this, [this](const QString &msg) {
        statusBar()->showMessage(msg, 4000);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::autoLogin(const QString &username, const QString &password)
{
    m_authService->login(username, password);
}

void MainWindow::navigateToPage(int index)
{
    ui->topHeaderBar->show();
    switch (index) {
    case 0:
        ui->dashboardButton->click();
        break;
    case 1:
        ui->devicesButton->click();
        break;
    case 2:
        ui->historyButton->click();
        break;
    case 3:
        ui->usersButton->click();
        break;
    default:
        ui->dashboardButton->click();
        break;
    }
}
