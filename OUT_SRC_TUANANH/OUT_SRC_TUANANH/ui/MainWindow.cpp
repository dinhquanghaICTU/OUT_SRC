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
    m_sidebarToggleButton = new QPushButton(ui->sideBar);
    m_sidebarToggleButton->setObjectName(QStringLiteral("sidebarToggleButton"));
    m_sidebarToggleButton->setToolTip(tr("Mở rộng/thu gọn thanh điều hướng"));
    ui->sideBarLayout->insertWidget(1, m_sidebarToggleButton);
    const bool savedSidebarExpanded = QSettings().value(
        QStringLiteral("ui/sidebarExpanded"), true).toBool();
    setSidebarExpanded(savedSidebarExpanded);

    ui->pages->addWidget(m_loginPage);
    ui->pages->addWidget(m_dashboardPage);
    ui->pages->addWidget(m_deviceManagementPage);
    ui->pages->addWidget(m_historyPage);
    ui->pages->addWidget(m_userManagementPage);
    ui->pages->setCurrentWidget(m_loginPage);
    ui->sideBar->hide();
    setCompactNavigation(width() <= 500);

    connect(m_loginPage, &LoginPage::loginRequested,
            m_authService, &AuthService::login);
    connect(m_sidebarToggleButton, &QPushButton::clicked, this, [this] {
        setSidebarExpanded(!m_sidebarExpanded);
        QSettings().setValue(QStringLiteral("ui/sidebarExpanded"), m_sidebarExpanded);
    });
    connect(m_authService, &AuthService::authenticated, this, [this] {
        ui->sideBar->show();
        m_dashboardPage->setUsername(m_authService->currentUsername());
        m_deviceManagementPage->setCurrentUser(m_authService->currentUsername(), m_authService->isAdmin());
        ui->devicesButton->setVisible(true);
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
                // Không hiện popup khi server chậm/mất kết nối vì app đang polling realtime.
                // Chỉ báo nhẹ ở status bar để không chặn giao diện.
                statusBar()->showMessage(message, 5000);
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
    const int pageWidth = event->size().width();
    setCompactNavigation(pageWidth <= 520);
    if (pageWidth < 520 && m_sidebarExpanded)
        setSidebarExpanded(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setSidebarExpanded(bool expanded)
{
    m_sidebarExpanded = expanded;
    const bool compact = ui->sideBar->property("compactNavigation").toBool();
    const int width = expanded ? 175 : 54;
    ui->sideBar->setMinimumWidth(compact ? 0 : width);
    ui->sideBar->setMaximumWidth(compact ? QWIDGETSIZE_MAX : width);
    ui->sideBar->setProperty("expanded", expanded);
    refreshSidebarButtonText();
    ui->sideBar->style()->unpolish(ui->sideBar);
    ui->sideBar->style()->polish(ui->sideBar);
    const QList<QPushButton *> buttons = ui->sideBar->findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        button->style()->unpolish(button);
        button->style()->polish(button);
    }
    ui->sideBar->update();
}

void MainWindow::setCompactNavigation(bool compact)
{
    if (ui->sideBar->property("compactNavigation").toBool() == compact)
        return;

    ui->sideBar->setProperty("compactNavigation", compact);
    ui->sideBarLayout->setContentsMargins(compact ? 4 : 8, compact ? 4 : 12,
                                           compact ? 4 : 8, compact ? 4 : 12);
    ui->sideBarLayout->setSpacing(compact ? 2 : 8);
    ui->sideLogo->setVisible(!compact);
    m_sidebarToggleButton->setVisible(!compact);
    ui->sideBar->setMinimumWidth(compact ? 0 : (m_sidebarExpanded ? 175 : 54));
    ui->sideBar->setMaximumWidth(compact ? QWIDGETSIZE_MAX : (m_sidebarExpanded ? 175 : 54));
    ui->sideBar->setMaximumHeight(compact ? 44 : QWIDGETSIZE_MAX);
    ui->sideBar->setMinimumHeight(compact ? 44 : 0);
    ui->mainLayout->setDirection(compact ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
    ui->sideBarLayout->setDirection(compact ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
    ui->navSpacer->changeSize(0, 0, QSizePolicy::Expanding,
                              compact ? QSizePolicy::Minimum : QSizePolicy::Expanding);
    ui->sideBarLayout->invalidate();
    ui->mainLayout->invalidate();
    ui->sideBar->style()->unpolish(ui->sideBar);
    ui->sideBar->style()->polish(ui->sideBar);
    refreshSidebarButtonText();
}

void MainWindow::refreshSidebarButtonText()
{
    const bool compact = ui->sideBar->property("compactNavigation").toBool();
    auto setNavText = [this, compact](QPushButton *button, const QString &icon, const QString &label) {
        if (compact) {
            button->setText(QStringLiteral("%1 %2").arg(icon, label));
        } else {
            button->setText(m_sidebarExpanded ? QStringLiteral("%1  %2").arg(icon, label) : icon);
        }
    };
    setNavText(m_sidebarToggleButton, m_sidebarExpanded ? QStringLiteral("‹")
                                                        : QStringLiteral("›"),
               m_sidebarExpanded ? tr("Thu gọn") : tr("Mở rộng"));
    setNavText(ui->dashboardButton, QStringLiteral("⌂"), tr("Trang chủ"));
    setNavText(ui->devicesButton, QStringLiteral("◆"), tr("Quản lý thiết bị"));
    setNavText(ui->historyButton, QStringLiteral("▤"), tr("Lịch sử dữ liệu"));
    setNavText(ui->usersButton, QStringLiteral("♟"), tr("Quản lý tài khoản"));
    setNavText(ui->logoutButton, QStringLiteral("↪"), tr("Đăng xuất"));
}
