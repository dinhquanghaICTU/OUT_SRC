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

#include <QButtonGroup>
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

    setWindowTitle(tr("Hệ Thống Giám Sát Điện Năng (ACS712 & ZMPT101B) - Thế Anh (ICTU)"));

    ui->pages->addWidget(m_loginPage);
    ui->pages->addWidget(m_dashboardPage);
    ui->pages->addWidget(m_deviceManagementPage);
    ui->pages->addWidget(m_historyPage);
    ui->pages->addWidget(m_userManagementPage);
    ui->pages->setCurrentWidget(m_loginPage);
    ui->topConsoleBar->hide();

    // Top Navigation Tabs
    auto *topNavGroup = new QButtonGroup(this);
    topNavGroup->addButton(ui->topNavDashboard);
    topNavGroup->addButton(ui->topNavHistory);
    topNavGroup->addButton(ui->topNavDevices);
    topNavGroup->addButton(ui->topNavUsers);
    topNavGroup->setExclusive(true);

    connect(ui->topNavDashboard, &QPushButton::clicked, this, [this] {
        ui->pages->setCurrentWidget(m_dashboardPage);
    });
    connect(ui->topNavHistory, &QPushButton::clicked, this, [this] {
        ui->pages->setCurrentWidget(m_historyPage);
        if (!m_authService->isOfflineMode()) {
            m_apiClient->requestMyDevice();
        }
    });
    connect(ui->topNavDevices, &QPushButton::clicked, this, [this] {
        ui->pages->setCurrentWidget(m_deviceManagementPage);
        if (!m_authService->isOfflineMode()) {
            m_apiClient->requestMyDevice();
            m_apiClient->requestAvailableDevices();
        }
    });
    connect(ui->topNavUsers, &QPushButton::clicked, this, [this] {
        ui->pages->setCurrentWidget(m_userManagementPage);
        if (!m_authService->isOfflineMode()) {
            m_apiClient->requestUsers();
        }
    });

    connect(ui->pages, &QStackedWidget::currentChanged, this, [this](int) {
        QWidget *cur = ui->pages->currentWidget();
        if (cur == m_dashboardPage) ui->topNavDashboard->setChecked(true);
        else if (cur == m_historyPage) ui->topNavHistory->setChecked(true);
        else if (cur == m_deviceManagementPage) ui->topNavDevices->setChecked(true);
        else if (cur == m_userManagementPage) ui->topNavUsers->setChecked(true);
    });

    connect(m_deviceManagementPage, &DeviceManagementPage::backToDashboardRequested, this, [this] {
        ui->pages->setCurrentWidget(m_dashboardPage);
    });
    connect(m_historyPage, &HistoryPage::backToDashboardRequested, this, [this] {
        ui->pages->setCurrentWidget(m_dashboardPage);
    });
    connect(m_userManagementPage, &UserManagementPage::backToDashboardRequested, this, [this] {
        ui->pages->setCurrentWidget(m_dashboardPage);
    });

    connect(m_loginPage, &LoginPage::loginRequested,
            m_authService, &AuthService::login);

    connect(m_authService, &AuthService::authenticated, this, [this] {
        ui->topConsoleBar->show();
        ui->topNavDashboard->setChecked(true);
        ui->topNavUsers->setVisible(m_authService->isAdmin());
        m_dashboardPage->setUsername(m_authService->currentUsername());
        m_deviceManagementPage->setCurrentUser(m_authService->currentUsername(), m_authService->isAdmin());
        m_userManagementPage->setAdminEnabled(m_authService->isAdmin());
        ui->pages->setCurrentWidget(m_dashboardPage);

        m_sensorService->start();

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
            [this](const QString &message) {
                if (!message.contains(QStringLiteral("readings/latest"), Qt::CaseInsensitive)) {
                    statusBar()->showMessage(message, 5000);
                }
            });

    // Dashboard Device Claims & Control
    connect(m_dashboardPage, &DashboardPage::claimDeviceRequested,
            m_apiClient, &ApiClient::claimDevice);
    connect(m_dashboardPage, &DashboardPage::releaseDeviceRequested,
            m_apiClient, &ApiClient::releaseDevice);
    connect(m_dashboardPage, &DashboardPage::relayControlRequested,
            m_apiClient, &ApiClient::setRelayState);
    connect(m_dashboardPage, &DashboardPage::refreshDevicesRequested,
            m_apiClient, &ApiClient::requestAvailableDevices);
    connect(m_dashboardPage, &DashboardPage::historyPageRequested, this, [this] {
        ui->pages->setCurrentWidget(m_historyPage);
        if (!m_authService->isOfflineMode()) {
            m_apiClient->requestMyDevice();
        }
    });
    connect(m_dashboardPage, &DashboardPage::devicesPageRequested, this, [this] {
        ui->pages->setCurrentWidget(m_deviceManagementPage);
        if (!m_authService->isOfflineMode()) {
            m_apiClient->requestMyDevice();
            m_apiClient->requestAvailableDevices();
        }
    });

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

    // Device Management Page Connections
    connect(m_deviceManagementPage, &DeviceManagementPage::claimDeviceRequested,
            m_apiClient, &ApiClient::claimDevice);
    connect(m_deviceManagementPage, &DeviceManagementPage::releaseDeviceRequested,
            m_apiClient, &ApiClient::releaseDevice);
    connect(m_deviceManagementPage, &DeviceManagementPage::relayControlRequested,
            m_apiClient, &ApiClient::setRelayState);
    connect(m_deviceManagementPage, &DeviceManagementPage::deviceConfigRequested,
            m_apiClient, &ApiClient::updatePerDeviceConfig);
    connect(m_deviceManagementPage, &DeviceManagementPage::refreshRequested, this,
            [this] {
                if (m_authService->isOfflineMode())
                    return;
                m_apiClient->requestMyDevice();
                m_apiClient->requestAvailableDevices();
            });
    connect(m_apiClient, &ApiClient::deviceConfigSaved,
            m_deviceManagementPage, &DeviceManagementPage::configSaved);
    connect(m_apiClient, &ApiClient::operationFailed,
            m_deviceManagementPage, &DeviceManagementPage::configSaveFailed);

    // History Page Connections
    connect(m_historyPage, &HistoryPage::historyRequested,
            m_apiClient, &ApiClient::requestDeviceHistory);
    connect(m_apiClient, &ApiClient::deviceHistoryReceived,
            m_historyPage, &HistoryPage::setHistory);

    // User Management Connections
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

    connect(ui->topLogoutButton, &QPushButton::clicked, this, [this] {
        m_sensorService->stop();
        m_authService->logout();
        ui->topConsoleBar->hide();
        ui->pages->setCurrentWidget(m_loginPage);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loginAdminDirectly()
{
    m_authService->login(QStringLiteral("admin"), QStringLiteral("1"));
}

void MainWindow::showHistoryTable()
{
    loginAdminDirectly();
    QTimer::singleShot(800, this, [this] {
        ui->pages->setCurrentWidget(m_historyPage);
        m_historyPage->showTableView();

        QJsonObject sample;
        sample[QStringLiteral("total")] = 8;
        QJsonArray keys;
        keys.append(QStringLiteral("current_a"));
        keys.append(QStringLiteral("power_w"));
        keys.append(QStringLiteral("voltage_v"));
        sample[QStringLiteral("metric_keys")] = keys;

        QJsonArray data;
        for (int i = 0; i < 8; ++i) {
            QJsonObject row;
            row[QStringLiteral("recorded_at")] = QStringLiteral("2026-09-22 15:35:%1").arg(50 - i * 2, 2, 10, QLatin1Char('0'));
            QJsonObject metrics;
            metrics[QStringLiteral("current_a")] = 2.35 + (i * 0.05);
            metrics[QStringLiteral("power_w")] = 518.2 + (i * 3.4);
            metrics[QStringLiteral("voltage_v")] = 220.5 + (i % 3);
            row[QStringLiteral("metrics")] = metrics;
            data.append(row);
        }
        sample[QStringLiteral("data")] = data;
        m_historyPage->setHistory(sample);
    });
}

void MainWindow::showHistoryChart()
{
    loginAdminDirectly();
    QTimer::singleShot(800, this, [this] {
        ui->pages->setCurrentWidget(m_historyPage);
        ui->topNavHistory->setChecked(true);

        QJsonObject sample;
        sample[QStringLiteral("total")] = 12;
        sample[QStringLiteral("period")] = QStringLiteral("day");
        sample[QStringLiteral("selected_date")] = QStringLiteral("2026-09-22");
        QJsonArray keys;
        keys.append(QStringLiteral("current_a"));
        keys.append(QStringLiteral("power_w"));
        keys.append(QStringLiteral("voltage_v"));
        sample[QStringLiteral("metric_keys")] = keys;

        QJsonArray data;
        for (int i = 0; i < 12; ++i) {
            QJsonObject row;
            row[QStringLiteral("recorded_at")] = QStringLiteral("2026-09-22 15:%1:00").arg(10 + i * 4, 2, 10, QLatin1Char('0'));
            QJsonObject metrics;
            metrics[QStringLiteral("current_a")] = 2.20 + (i % 5) * 0.25;
            metrics[QStringLiteral("power_w")] = 480.0 + (i % 5) * 55.0;
            metrics[QStringLiteral("voltage_v")] = 219.0 + (i % 4) * 1.5;
            row[QStringLiteral("metrics")] = metrics;
            data.append(row);
        }
        sample[QStringLiteral("data")] = data;
        m_historyPage->setHistory(sample);
    });
}

void MainWindow::showUserManagement()
{
    loginAdminDirectly();
    QTimer::singleShot(800, this, [this] {
        ui->pages->setCurrentWidget(m_userManagementPage);
        ui->topNavUsers->setChecked(true);
    });
}

void MainWindow::showDeviceDrawer()
{
    loginAdminDirectly();
    QTimer::singleShot(800, this, [this] {
        ui->pages->setCurrentWidget(m_deviceManagementPage);
        QJsonObject dev;
        dev[QStringLiteral("device_id")] = QStringLiteral("Theanh-190782");
        dev[QStringLiteral("name")] = QStringLiteral("Bộ Đo AC RMS & Công Suất Tải THEANH");
        dev[QStringLiteral("device_type")] = QStringLiteral("power_monitor");
        dev[QStringLiteral("added_by")] = QStringLiteral("admin");
        dev[QStringLiteral("created_at")] = QStringLiteral("2026-09-22 15:34:00");
        QJsonObject metrics;
        metrics[QStringLiteral("voltage_v")] = 221.8;
        metrics[QStringLiteral("current_a")] = 2.35;
        metrics[QStringLiteral("power_w")] = 518.0;
        dev[QStringLiteral("metrics")] = metrics;
        m_deviceManagementPage->openDeviceDrawer(dev);
    });
}
