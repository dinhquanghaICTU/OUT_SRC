#include "MainWindow.h"

#include "api/ApiClient.h"
#include "services/AuthService.h"
#include "services/SensorService.h"
#include "ui/dialogs/ErrorDialog.h"
#include "ui/dialogs/SelectOnlineDeviceDialog.h"
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
        m_deviceManagementPage->setCurrentUser(m_authService->currentUsername(), m_authService->isAdmin());
        ui->usersButton->setVisible(m_authService->isAdmin());
        m_userManagementPage->setAdminEnabled(m_authService->isAdmin());
        ui->pages->setCurrentWidget(m_dashboardPage);
        ui->dashboardButton->setChecked(true);
        setSidebarExpanded(true);
        if (m_authService->isOfflineMode()) {
            m_deviceManagementPage->stopRealtime();
            statusBar()->showMessage(
                tr("Đang dùng admin offline để chỉnh giao diện. Server chưa kết nối."),
                8000);
            return;
        }
        m_apiClient->requestMyDevice();
        m_apiClient->requestAvailableDevices();
        m_deviceManagementPage->startRealtime();
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
    connect(m_dashboardPage, &DashboardPage::relayControlRequested,
            m_apiClient, &ApiClient::setRelayState);
    connect(m_dashboardPage, &DashboardPage::addDeviceRequested,
            this, &MainWindow::openSelectDeviceDialog);

    connect(m_deviceManagementPage, &DeviceManagementPage::deviceConfigRequested,
            m_apiClient, &ApiClient::updatePerDeviceConfig);
    connect(m_dashboardPage, &DashboardPage::deviceConfigRequested,
            m_apiClient, &ApiClient::updatePerDeviceConfig);
    connect(m_deviceManagementPage, &DeviceManagementPage::releaseDeviceRequested,
            m_apiClient, &ApiClient::releaseDevice);

    connect(m_apiClient, &ApiClient::availableDevicesReceived,
            m_deviceManagementPage, &DeviceManagementPage::setAvailableDevices);
    connect(m_apiClient, &ApiClient::availableDevicesReceived, this, [this](const QJsonArray &devices) {
        m_availableDevices = devices;
    });

    connect(m_apiClient, &ApiClient::devicesReceived,
            m_deviceManagementPage, &DeviceManagementPage::setOwnedDevices);
    connect(m_apiClient, &ApiClient::devicesReceived, this, [this](const QJsonArray &devices) {
        if (!devices.isEmpty()) {
            const auto firstDev = devices.first().toObject();
            const QString devId = firstDev.value(QStringLiteral("device_id")).toString();
            const QString name = firstDev.value(QStringLiteral("name")).toString(devId);
            m_dashboardPage->setHasDevice(true, devId, name);
        } else {
            m_dashboardPage->setHasDevice(false);
        }
    });

    auto forwardMetrics = [this](const QJsonArray &devices) {
        for (const auto &val : devices) {
            const auto dev = val.toObject();
            const QString devId = dev.value(QStringLiteral("device_id")).toString();
            if (!devId.isEmpty()) {
                m_dashboardPage->setDeviceId(devId);
            }
            const auto metrics = dev.value(QStringLiteral("metrics")).toObject();
            if (!metrics.isEmpty()) {
                m_dashboardPage->updateDeviceMetrics(metrics);
                break;
            }
        }
    };
    connect(m_apiClient, &ApiClient::devicesReceived, this, forwardMetrics);
    connect(m_apiClient, &ApiClient::availableDevicesReceived, this, forwardMetrics);

    auto *realtimeTimer = new QTimer(this);
    realtimeTimer->setInterval(800);
    connect(realtimeTimer, &QTimer::timeout, this, [this] {
        if (m_authService->isAuthenticated() && !m_authService->isOfflineMode()) {
            m_apiClient->requestMyDevice();
            m_apiClient->requestAvailableDevices();
        }
    });
    realtimeTimer->start();

    connect(m_apiClient, &ApiClient::deviceClaimed, this,
            [this](const QJsonObject &device) {
                if (m_authService->isOfflineMode())
                    return;
                const QString devId = device.value(QStringLiteral("device_id")).toString();
                const QString name = device.value(QStringLiteral("name")).toString(devId);
                m_dashboardPage->setHasDevice(true, devId, name);
                m_apiClient->requestMyDevice();
                m_apiClient->requestAvailableDevices();
                statusBar()->showMessage(tr("Thêm thiết bị thành công"), 5000);
            });
    connect(m_apiClient, &ApiClient::deviceReleased, this,
            [this](const QString &) {
                if (m_authService->isOfflineMode())
                    return;
                m_dashboardPage->setHasDevice(false);
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
                statusBar()->showMessage(message, 5000);
                m_apiClient->requestMyDevice();
            });

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
        m_deviceManagementPage->stopRealtime();
        m_authService->logout();
        ui->sideBar->hide();
        ui->pages->setCurrentWidget(m_loginPage);
    });

    setSidebarExpanded(true);
    refreshSidebarButtonText();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openSelectDeviceDialog()
{
    SelectOnlineDeviceDialog dlg(m_availableDevices, this);
    connect(&dlg, &SelectOnlineDeviceDialog::deviceSelected, this, [this](const QString &deviceId, const QString &name) {
        if (!m_authService->isOfflineMode()) {
            m_apiClient->claimDevice(deviceId, name);
        } else {
            m_dashboardPage->setHasDevice(true, deviceId, name);
        }
    });
    dlg.exec();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    const int w = width();
    const int h = height();
    setCompactNavigation(w < 700);
    setShortHeight(h < 420);
}

void MainWindow::setSidebarExpanded(bool expanded)
{
    m_sidebarExpanded = expanded;
    refreshSidebarButtonText();
}

void MainWindow::setCompactNavigation(bool compact)
{
    const QString textDashboard = compact ? tr("⌂") : tr("⌂ Trang chủ");
    const QString textUsers = compact ? tr("♟") : tr("♟ Tài khoản");
    ui->dashboardButton->setText(textDashboard);
    ui->usersButton->setText(textUsers);
}

void MainWindow::setShortHeight(bool shortHeight)
{
    ui->sideBar->setMaximumHeight(shortHeight ? 38 : 44);
}

void MainWindow::refreshSidebarButtonText()
{
    const int w = width();
    setCompactNavigation(w < 700);
}
