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
      m_deviceManagementPage(new DeviceManagementPage(this)),
      m_historyPage(new HistoryPage(this)),
      m_userManagementPage(new UserManagementPage(this))
{
    ui->setupUi(this);
    m_sidebarToggleButton = nullptr;
    setSidebarExpanded(true);

    ui->pages->addWidget(m_loginPage);
    ui->pages->addWidget(m_dashboardPage);
    ui->pages->addWidget(m_deviceManagementPage);
    ui->pages->addWidget(m_historyPage);
    ui->pages->addWidget(m_userManagementPage);
    ui->pages->setCurrentWidget(m_loginPage);
    ui->sideBar->hide();

    connect(m_loginPage, &LoginPage::loginRequested,
            m_authService, &AuthService::login);
    connect(m_authService, &AuthService::authenticated, this, [this] {
        ui->sideBar->show();
        m_dashboardPage->setUsername(m_authService->currentUsername());
        ui->usersButton->setVisible(m_authService->isAdmin());
        m_userManagementPage->setAdminEnabled(m_authService->isAdmin());
        ui->pages->setCurrentWidget(m_dashboardPage);
        ui->dashboardButton->setChecked(true);
        if (m_authService->isOfflineMode()) {
            m_deviceManagementPage->stopRealtime();
            statusBar()->showMessage(
                tr("Đang dùng admin offline để chỉnh giao diện. Server chưa kết nối."),
                8000);
            return;
        }
        m_apiClient->requestMyDevice();
        m_deviceManagementPage->startRealtime();
        // Dashboard/device data hiện lấy từ MQTT discovery. Không poll endpoint
        // legacy /api/readings/latest vì request 404 lặp lại gây nhiễu và tải thừa.
    });
    connect(m_authService, &AuthService::authenticationFailed, this,
            [this](const QString &message) {
                ErrorDialog::showLoginError(this, message);
            });
    connect(m_sensorService, &SensorService::readingUpdated,
            m_dashboardPage, &DashboardPage::updateReading);
    connect(m_apiClient, &ApiClient::networkError, this,
            [this](const QString &message) { statusBar()->showMessage(message, 5000); });
    connect(m_deviceManagementPage, &DeviceManagementPage::claimDeviceRequested,
            m_apiClient, &ApiClient::claimDevice);
    connect(m_deviceManagementPage, &DeviceManagementPage::refreshRequested,
            m_apiClient, &ApiClient::requestAvailableDevices);
    connect(m_deviceManagementPage, &DeviceManagementPage::refreshRequested,
            m_apiClient, &ApiClient::requestMyDevice);
    connect(m_deviceManagementPage, &DeviceManagementPage::relayControlRequested,
            m_apiClient, &ApiClient::setRelayState);
    connect(m_deviceManagementPage, &DeviceManagementPage::deviceConfigRequested,
            m_apiClient, &ApiClient::updatePerDeviceConfig);
    connect(m_deviceManagementPage, &DeviceManagementPage::releaseDeviceRequested,
            m_apiClient, &ApiClient::releaseDevice);
    connect(m_apiClient, &ApiClient::availableDevicesReceived,
            m_deviceManagementPage, &DeviceManagementPage::setAvailableDevices);
    connect(m_apiClient, &ApiClient::devicesReceived,
            m_deviceManagementPage, &DeviceManagementPage::setOwnedDevices);
    connect(m_apiClient, &ApiClient::devicesReceived,
            m_dashboardPage, &DashboardPage::setDevices);
    connect(m_apiClient, &ApiClient::devicesReceived,
            m_historyPage, &HistoryPage::setDevices);
    connect(m_historyPage, &HistoryPage::historyRequested,
            m_apiClient, &ApiClient::requestDeviceHistory);
    connect(m_apiClient, &ApiClient::deviceHistoryReceived,
            m_historyPage, &HistoryPage::setHistory);
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
    connect(m_apiClient, &ApiClient::deviceConfigSaved,
            m_deviceManagementPage, &DeviceManagementPage::configSaved);
    connect(m_apiClient, &ApiClient::deviceConfigSaved, this,
            [this](const QString &, bool) {
                if (!m_authService->isOfflineMode())
                    m_apiClient->requestMyDevice();
            });

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
        if (m_authService->isOfflineMode())
            return;
        statusBar()->showMessage(tr("Thêm tài khoản thành công"), 5000);
        m_apiClient->requestUsers();
    });
    connect(m_apiClient, &ApiClient::userUpdated, this, [this] {
        if (m_authService->isOfflineMode())
            return;
        statusBar()->showMessage(tr("Cập nhật tài khoản thành công"), 5000);
        m_apiClient->requestUsers();
    });
    connect(m_apiClient, &ApiClient::userDeleted, this, [this] {
        if (m_authService->isOfflineMode())
            return;
        statusBar()->showMessage(tr("Đã xóa tài khoản"), 5000);
        m_apiClient->requestUsers();
        m_apiClient->requestAvailableDevices();
    });
    connect(m_apiClient, &ApiClient::userDeviceReleased, this, [this](const QString &, const QString &deviceId) {
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
                if (message.contains(QStringLiteral("canceled"), Qt::CaseInsensitive)
                    || message.contains(QStringLiteral("cancelled"), Qt::CaseInsensitive)) {
                    return;
                }
                statusBar()->showMessage(message, 4000);
                m_apiClient->requestMyDevice();
            });

    connect(ui->dashboardButton, &QPushButton::clicked, this,
            [this] { ui->pages->setCurrentWidget(m_dashboardPage); });
    connect(ui->devicesButton, &QPushButton::clicked, this,
            [this] {
                ui->pages->setCurrentWidget(m_deviceManagementPage);
                if (m_authService->isOfflineMode())
                    return;
                m_apiClient->requestMyDevice();
                m_apiClient->requestAvailableDevices();
            });
    connect(ui->historyButton, &QPushButton::clicked, this,
            [this] {
                ui->pages->setCurrentWidget(m_historyPage);
                if (m_authService->isOfflineMode())
                    return;
                m_apiClient->requestMyDevice();
            });
    connect(ui->usersButton, &QPushButton::clicked, this,
            [this] {
                ui->pages->setCurrentWidget(m_userManagementPage);
                if (m_authService->isOfflineMode())
                    return;
                m_apiClient->requestUsers();
            });
    connect(ui->logoutButton, &QPushButton::clicked, this, [this] {
        m_sensorService->stop();
        m_deviceManagementPage->stopRealtime();
        m_authService->logout();
        ui->sideBar->hide();
        ui->pages->setCurrentWidget(m_loginPage);
    });
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    Q_UNUSED(event);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setSidebarExpanded(bool expanded)
{
    Q_UNUSED(expanded);
    m_sidebarExpanded = true;
    ui->sideBar->setMinimumHeight(76);
    ui->sideBar->setMaximumHeight(76);
    ui->sideBar->setMinimumWidth(0);
    ui->sideBar->setMaximumWidth(QWIDGETSIZE_MAX);
    ui->sideBar->setProperty("expanded", true);
    refreshSidebarButtonText();
    ui->sideBar->style()->unpolish(ui->sideBar);
    ui->sideBar->style()->polish(ui->sideBar);
}

void MainWindow::refreshSidebarButtonText()
{
    ui->dashboardButton->setText(QStringLiteral("⌂  ") + tr("Trang chủ"));
    ui->devicesButton->setText(QStringLiteral("◆  ") + tr("Thiết bị"));
    ui->historyButton->setText(QStringLiteral("▤  ") + tr("Lịch sử"));
    ui->usersButton->setText(QStringLiteral("♟  ") + tr("Tài khoản"));
    ui->logoutButton->setText(QStringLiteral("↪  ") + tr("Đăng xuất"));
}
