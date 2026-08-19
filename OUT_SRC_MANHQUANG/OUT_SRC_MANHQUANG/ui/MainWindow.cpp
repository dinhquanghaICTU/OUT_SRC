#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "api/ApiClient.h"
#include "services/AuthService.h"
#include "services/SensorService.h"
#include "ui/dialogs/ErrorDialog.h"
#include "ui/pages/AlertPage.h"
#include "ui/pages/DashboardPage.h"
#include "ui/pages/DeviceManagementPage.h"
#include "ui/pages/HistoryPage.h"
#include "ui/pages/LoginPage.h"
#include "ui/pages/UserManagementPage.h"

#include <QDateTime>
#include <QMessageBox>
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
      m_alertPage(new AlertPage(this)),
      m_userManagementPage(new UserManagementPage(this))
{
    ui->setupUi(this);

    ui->pages->addWidget(m_loginPage);
    ui->pages->addWidget(m_dashboardPage);
    ui->pages->addWidget(m_deviceManagementPage);
    ui->pages->addWidget(m_historyPage);
    ui->pages->addWidget(m_alertPage);
    ui->pages->addWidget(m_userManagementPage);

    ui->pages->setCurrentWidget(m_loginPage);
    ui->topHeaderRibbon->hide();
    ui->leftNavRail->hide();

    setupNavigation();

    connect(&m_clockTimer, &QTimer::timeout, this, &MainWindow::updateClock);
    m_clockTimer.start(1000);
    updateClock();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateClock()
{
    ui->clockLabel->setText(QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy HH:mm:ss")));
}

void MainWindow::setupNavigation()
{
    // Auth connections
    connect(m_loginPage, &LoginPage::loginRequested, m_authService, &AuthService::login);

    connect(m_authService, &AuthService::authenticated, this, [this] {
        ui->topHeaderRibbon->show();
        ui->leftNavRail->show();

        const QString user = m_authService->currentUsername();
        const bool isAdmin = m_authService->isAdmin();

        ui->currentUserBadge->setText(isAdmin ? tr("👑 Admin (%1)").arg(user) : tr("👤 %1").arg(user));
        ui->btnNavUsers->setVisible(isAdmin);
        m_userManagementPage->setAdminEnabled(isAdmin);
        m_deviceManagementPage->setCurrentUser(user, isAdmin);

        ui->pages->setCurrentWidget(m_dashboardPage);
        ui->btnNavDashboard->setChecked(true);

        m_sensorService->start();

        if (m_authService->isOfflineMode()) {
            statusBar()->showMessage(tr("Chế độ Offline Admin - Đã kết nối cục bộ"), 6000);
        } else {
            m_apiClient->requestMyDevice();
            m_deviceManagementPage->startRealtime();
        }
    });

    connect(m_authService, &AuthService::authenticationFailed, this, [this](const QString &msg) {
        ErrorDialog::showLoginError(this, msg);
    });

    connect(ui->logoutButton, &QPushButton::clicked, this, [this] {
        m_authService->logout();
        m_sensorService->stop();
        m_deviceManagementPage->stopRealtime();
        ui->topHeaderRibbon->hide();
        ui->leftNavRail->hide();
        ui->pages->setCurrentWidget(m_loginPage);
    });

    // Navigation buttons
    connect(ui->btnNavDashboard, &QPushButton::clicked, this, [this] {
        ui->pages->setCurrentWidget(m_dashboardPage);
    });
    connect(ui->btnNavDevices, &QPushButton::clicked, this, [this] {
        ui->pages->setCurrentWidget(m_deviceManagementPage);
        if (!m_authService->isOfflineMode()) {
            m_apiClient->requestMyDevice();
            m_apiClient->requestAvailableDevices();
        }
    });
    connect(ui->btnNavHistory, &QPushButton::clicked, this, [this] {
        ui->pages->setCurrentWidget(m_historyPage);
    });
    connect(ui->btnNavAlerts, &QPushButton::clicked, this, [this] {
        ui->pages->setCurrentWidget(m_alertPage);
    });
    connect(ui->btnNavUsers, &QPushButton::clicked, this, [this] {
        ui->pages->setCurrentWidget(m_userManagementPage);
        if (!m_authService->isOfflineMode()) {
            m_apiClient->requestUsers();
        }
    });

    // Sensor service to Dashboard & Header Ribbon
    connect(m_sensorService, &SensorService::readingUpdated, m_dashboardPage, &DashboardPage::updateReading);
    connect(m_sensorService, &SensorService::readingUpdated, this, [this](const SensorReading &r) {
        if (r.irBlocked) {
            ui->doorStatusPill->setText(tr("⚠️ KẸT VẬT CẢN IR"));
            ui->doorStatusPill->setStyleSheet(QStringLiteral("color: #ef4444; background: rgba(239, 68, 68, 0.15); border: 1px solid #ef4444; border-radius: 6px; font-size: 10px; font-weight: 800; padding: 3px 8px;"));
        } else if (r.doorPositionPct >= 95.0) {
            ui->doorStatusPill->setText(tr("🟢 CỬA ĐANG MỞ"));
            ui->doorStatusPill->setStyleSheet(QStringLiteral("color: #10b981; background: rgba(16, 185, 129, 0.15); border: 1px solid #10b981; border-radius: 6px; font-size: 10px; font-weight: 800; padding: 3px 8px;"));
        } else if (r.doorPositionPct <= 5.0) {
            ui->doorStatusPill->setText(tr("⚪ CỬA ĐÃ ĐÓNG"));
            ui->doorStatusPill->setStyleSheet(QStringLiteral("color: #94a3b8; background: rgba(148, 163, 184, 0.12); border: 1px solid #334155; border-radius: 6px; font-size: 10px; font-weight: 800; padding: 3px 8px;"));
        } else {
            ui->doorStatusPill->setText(tr("🟡 ĐANG DI CHUYỂN (%1%)").arg(QString::number(r.doorPositionPct, 'f', 0)));
            ui->doorStatusPill->setStyleSheet(QStringLiteral("color: #f59e0b; background: rgba(245, 158, 11, 0.15); border: 1px solid #f59e0b; border-radius: 6px; font-size: 10px; font-weight: 800; padding: 3px 8px;"));
        }
    });

    connect(m_sensorService, &SensorService::safetyAlert, this, [this](const QString &source, const QString &msg) {
        m_alertPage->addAlert(source, QStringLiteral("warning"), msg);
    });

    // Dashboard Actions
    connect(m_dashboardPage, &DashboardPage::doorCommandRequested, this, &MainWindow::onDoorCommandFromDashboard);
    connect(m_dashboardPage, &DashboardPage::simMotionRequested, m_sensorService, &SensorService::triggerManualMotion);
    connect(m_dashboardPage, &DashboardPage::simObstacleRequested, m_sensorService, &SensorService::triggerObstacle);

    // Device Management connections
    connect(m_deviceManagementPage, &DeviceManagementPage::claimDeviceRequested, m_apiClient, &ApiClient::claimDevice);
    connect(m_deviceManagementPage, &DeviceManagementPage::releaseDeviceRequested, m_apiClient, &ApiClient::releaseDevice);
    connect(m_deviceManagementPage, &DeviceManagementPage::relayControlRequested, m_apiClient, &ApiClient::setRelayState);
    connect(m_deviceManagementPage, &DeviceManagementPage::deviceConfigRequested, m_apiClient, &ApiClient::updatePerDeviceConfig);
    connect(m_deviceManagementPage, &DeviceManagementPage::refreshRequested, this, [this] {
        m_apiClient->requestMyDevice();
        m_apiClient->requestAvailableDevices();
    });

    connect(m_apiClient, &ApiClient::devicesReceived, m_deviceManagementPage, &DeviceManagementPage::setOwnedDevices);
    connect(m_apiClient, &ApiClient::devicesReceived, m_historyPage, &HistoryPage::setDevices);
    connect(m_apiClient, &ApiClient::availableDevicesReceived, m_deviceManagementPage, &DeviceManagementPage::setAvailableDevices);
    connect(m_apiClient, &ApiClient::deviceClaimed, this, [this] {
        m_apiClient->requestMyDevice();
        m_apiClient->requestAvailableDevices();
        statusBar()->showMessage(tr("Đã thêm thiết bị cửa thành công"), 4000);
    });
    connect(m_apiClient, &ApiClient::deviceReleased, this, [this] {
        m_apiClient->requestMyDevice();
        m_apiClient->requestAvailableDevices();
        statusBar()->showMessage(tr("Đã gỡ thiết bị khỏi tài khoản"), 4000);
    });

    // History
    connect(m_historyPage, &HistoryPage::historyRequested, m_apiClient, &ApiClient::requestDeviceHistory);
    connect(m_apiClient, &ApiClient::deviceHistoryReceived, m_historyPage, &HistoryPage::setHistory);

    // Users
    connect(m_userManagementPage, &UserManagementPage::createUserRequested, m_apiClient, &ApiClient::createUser);
    connect(m_userManagementPage, &UserManagementPage::updateUserRequested, m_apiClient, &ApiClient::updateUser);
    connect(m_userManagementPage, &UserManagementPage::deleteUserRequested, m_apiClient, &ApiClient::deleteUser);
    connect(m_apiClient, &ApiClient::usersReceived, m_userManagementPage, &UserManagementPage::setUsers);
    connect(m_apiClient, &ApiClient::userCreated, this, [this] { m_apiClient->requestUsers(); statusBar()->showMessage(tr("Tạo tài khoản thành công"), 3000); });
    connect(m_apiClient, &ApiClient::userUpdated, this, [this] { m_apiClient->requestUsers(); statusBar()->showMessage(tr("Cập nhật tài khoản thành công"), 3000); });
    connect(m_apiClient, &ApiClient::userDeleted, this, [this] { m_apiClient->requestUsers(); statusBar()->showMessage(tr("Đã xóa tài khoản"), 3000); });

    // Error handler
    connect(m_apiClient, &ApiClient::networkError, this, [this](const QString &err) {
        statusBar()->showMessage(err, 5000);
    });
}

void MainWindow::onDoorCommandFromDashboard(const QString &action)
{
    if (action == QStringLiteral("open")) {
        m_sensorService->triggerManualMotion();
        if (!m_authService->isOfflineMode()) {
            m_apiClient->setRelayState(m_activeDeviceId, true);
        }
        statusBar()->showMessage(tr("Lệnh mở cửa đã gửi"), 3000);
    } else if (action == QStringLiteral("close")) {
        if (!m_authService->isOfflineMode()) {
            m_apiClient->setRelayState(m_activeDeviceId, false);
        }
        statusBar()->showMessage(tr("Lệnh đóng cửa đã gửi"), 3000);
    } else if (action == QStringLiteral("stop")) {
        statusBar()->showMessage(tr("🛑 DỪNG KHẨN CẤP ĐỘNG CƠ CỬA"), 4000);
        m_alertPage->addAlert(QStringLiteral("MANUAL_STOP"), QStringLiteral("critical"), tr("Người vận hành kích hoạt dừng khẩn cấp"));
    }
}
