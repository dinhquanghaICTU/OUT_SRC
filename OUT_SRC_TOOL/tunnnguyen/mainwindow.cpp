#include "mainwindow.h"
#include "ui_mainwindow.h"

// Core & Auth
#include "core/ChromeLauncher.h"
#include "core/SyncManager.h"
#include "auth/LicenseManager.h"

// Modular Pages in src/pages/
#include "dashboard/DashboardPage.h"
#include "account_parser/AccountParserPage.h"
#include "chrome_profiles/ChromeProfilesPage.h"
#include "proxy_pool/ProxyPoolPage.h"
#include "task_runner/TaskRunnerPage.h"
#include "license_key/LicenseKeyPage.h"
#include "cookie_manager/CookieManagerPage.h"
#include "reports_logs/ReportsLogsPage.h"
#include "settings/SettingsPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_launcher = new ChromeLauncher(this);
    m_syncManager = new SyncManager(this);

    setupCustomUi();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupCustomUi()
{
    setWindowTitle("TUNNGUYEN AUTOMATION - POS MMO ACCOUNT PARSER");
    resize(1360, 840);
    setMinimumSize(1100, 700);

    // Global SaaS theme stylesheet matching the exact original design
    setStyleSheet(
        "* { font-family: 'Google Sans', 'Product Sans', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; }"
        "QMainWindow { background-color: #e5e7eb; }"
        "QWidget#appContainer { background-color: #ffffff; border-radius: 20px; }"
        "QTableWidget { background-color: #ffffff; border: 1px solid #e2e8f0; "
        "border-radius: 12px; gridline-color: #f1f5f9; font-size: 13px; "
        "selection-background-color: #fff7ed; selection-color: #9a3412; "
        "alternate-background-color: #fbfcfe; }"
        "QTableWidget::item { padding: 4px 10px; border-bottom: 1px solid #f1f5f9; }"
        "QTableWidget::indicator { width: 18px; height: 18px; border-radius: 5px; "
        "border: 1.5px solid #cbd5e1; background: #ffffff; margin-right: 6px; }"
        "QTableWidget::indicator:checked { background-color: #f97316; border-color: #f97316; }"
        "QTableWidget::indicator:hover { border-color: #f97316; }"
        "QHeaderView::section { background-color: #f8fafc; color: #475569; "
        "font-weight: 700; font-size: 12px; border: none; border-bottom: 2px "
        "solid #e2e8f0; padding: 12px 10px; }"
        "QPlainTextEdit { background-color: #ffffff; border: 1px solid #e2e8f0; "
        "border-radius: 12px; padding: 12px; font-family: monospace; font-size: "
        "12px; color: #1e293b; }"
        "QPlainTextEdit:focus { border: 1.5px solid #f97316; }"
        "QLineEdit { background: #ffffff; border: 1px solid #e2e8f0; "
        "border-radius: 8px; padding: 7px 12px; font-size: 13px; color: #1e293b; "
        "min-height: 20px; }"
        "QLineEdit:focus { border: 1.5px solid #f97316; }"
        "QComboBox {"
        "   background-color: #ffffff;"
        "   border: 1px solid #e2e8f0;"
        "   border-radius: 8px;"
        "   padding: 7px 32px 7px 12px;"
        "   font-size: 13px;"
        "   color: #1e293b;"
        "   font-weight: 500;"
        "   min-height: 20px;"
        "}"
        "QComboBox:hover { border-color: #cbd5e1; background-color: #f8fafc; }"
        "QComboBox:focus, QComboBox:on { border: 1.5px solid #f97316; "
        "background-color: #ffffff; }"
        "QComboBox::drop-down {"
        "   subcontrol-origin: padding;"
        "   subcontrol-position: top right;"
        "   width: 28px;"
        "   border: none;"
        "}"
        "QComboBox::down-arrow {"
        "   width: 0; height: 0;"
        "   border-left: 4.5px solid transparent;"
        "   border-right: 4.5px solid transparent;"
        "   border-top: 5.5px solid #64748b;"
        "   margin-right: 8px;"
        "}"
        "QComboBox::down-arrow:on {"
        "   border-top: none;"
        "   border-bottom: 5.5px solid #f97316;"
        "}"
        "QComboBox QAbstractItemView {"
        "   background-color: #ffffff;"
        "   border: 1px solid #e2e8f0;"
        "   border-radius: 12px;"
        "   padding: 6px;"
        "   selection-background-color: #ffedd5;"
        "   selection-color: #c2410c;"
        "   outline: none;"
        "   font-size: 13px;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "   min-height: 32px;"
        "   padding-left: 10px;"
        "   padding-right: 10px;"
        "   border-radius: 6px;"
        "   color: #334155;"
        "   margin-bottom: 2px;"
        "}"
        "QComboBox QAbstractItemView::item:hover {"
        "   background-color: #f8fafc;"
        "   color: #0f172a;"
        "}"
        "QComboBox QAbstractItemView::item:selected {"
        "   background-color: #ffedd5;"
        "   color: #c2410c;"
        "   font-weight: 600;"
        "}"
        "QScrollBar:vertical { background: #f8fafc; width: 6px; margin: 0; "
        "border-radius: 3px; }"
        "QScrollBar::handle:vertical { background: #cbd5e1; min-height: 20px; "
        "border-radius: 3px; }"
        "QScrollBar::handle:vertical:hover { background: #94a3b8; }");

    auto *rootWidget = new QWidget(this);
    setCentralWidget(rootWidget);

    auto *rootLayout = new QVBoxLayout(rootWidget);
    rootLayout->setContentsMargins(16, 16, 16, 16);

    // Main App Container (Rounded white card like the template)
    auto *appContainer = new QWidget(rootWidget);
    appContainer->setObjectName("appContainer");

    auto *appLayout = new QHBoxLayout(appContainer);
    appLayout->setContentsMargins(0, 0, 0, 0);
    appLayout->setSpacing(0);

    // 1. LEFT SIDEBAR (Width: 250px)
    appLayout->addWidget(createSidebar());

    // Vertical Divider
    auto *line1 = new QFrame();
    line1->setFrameShape(QFrame::VLine);
    line1->setStyleSheet("color: #f1f5f9;");
    appLayout->addWidget(line1);

    // 2. MAIN CENTER CONTENT
    auto *centerWrapper = new QWidget(appContainer);
    auto *centerLayout = new QVBoxLayout(centerWrapper);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);

    // Top Bar (Search + Profile Avatar)
    centerLayout->addWidget(createTopBar());

    auto *line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setStyleSheet("color: #f1f5f9;");
    centerLayout->addWidget(line2);

    // Center Main Area Stack
    m_pageStack = new QStackedWidget();

    // Instantiate each modular page widget from src/pages/
    m_pageDashboard      = new DashboardPage(this);            // Index 0: src/pages/dashboard/
    m_pageAccountParser  = new AccountParserPage(this);         // Index 1: src/pages/account_parser/
    m_pageChromeProfiles = new ChromeProfilesPage(m_launcher, m_syncManager, this); // Index 2: src/pages/chrome_profiles/
    m_pageProxyPool      = new ProxyPoolPage(this);             // Index 3: src/pages/proxy_pool/
    m_pageTaskRunner     = new TaskRunnerPage(this);            // Index 4: src/pages/task_runner/
    m_pageLicenseKey     = new LicenseKeyPage(this);            // Index 5: src/pages/license_key/
    m_pageCookieManager  = new CookieManagerPage(this);         // Index 6: src/pages/cookie_manager/
    m_pageReportsLogs    = new ReportsLogsPage(this);           // Index 7: src/pages/reports_logs/
    m_pageSettings       = new SettingsPage(this);              // Index 8: src/pages/settings/

    m_pageStack->addWidget(m_pageDashboard);                            // 0: Dashboard
    m_pageStack->addWidget(m_pageAccountParser->getCenterWidget());     // 1: Tách & lọc tài khoản
    m_pageStack->addWidget(m_pageChromeProfiles);                       // 2: Chrome Profiles
    m_pageStack->addWidget(m_pageProxyPool);                            // 3: Proxy Pool
    m_pageStack->addWidget(m_pageTaskRunner);                           // 4: Task Runner
    m_pageStack->addWidget(m_pageLicenseKey);                           // 5: License Key
    m_pageStack->addWidget(m_pageCookieManager);                        // 6: Cookie Manager
    m_pageStack->addWidget(m_pageReportsLogs);                          // 7: Reports & Logs
    m_pageStack->addWidget(m_pageSettings);                             // 8: Settings

    // Cross-page sync
    connect(m_pageChromeProfiles, &ChromeProfilesPage::profilesChanged, this, [this](const QList<ChromeProfileItem> &profiles) {
        m_pageProxyPool->updateProfiles(profiles);
        m_pageTaskRunner->updateProfiles(profiles);
    });
    connect(m_pageDashboard, &DashboardPage::requestNavigate, this, &MainWindow::onNavButtonClicked);

    m_pageProxyPool->updateProfiles(m_pageChromeProfiles->getProfiles());
    m_pageTaskRunner->updateProfiles(m_pageChromeProfiles->getProfiles());

    centerLayout->addWidget(m_pageStack, 1);
    appLayout->addWidget(centerWrapper, 1);

    // Vertical Divider
    m_rightDivider = new QFrame();
    m_rightDivider->setFrameShape(QFrame::VLine);
    m_rightDivider->setStyleSheet("color: #f1f5f9;");
    appLayout->addWidget(m_rightDivider);

    // 3. RIGHT SUMMARY & EXPORT PANEL (Exact original width: 370px)
    m_rightPanel = m_pageAccountParser->getRightSummaryWidget();
    appLayout->addWidget(m_rightPanel);

    rootLayout->addWidget(appContainer);

    // Initial page: Dashboard (0)
    onNavButtonClicked(0);
}

QWidget *MainWindow::createSidebar()
{
    auto *sidebar = new QWidget();
    sidebar->setFixedWidth(250);
    sidebar->setStyleSheet("background-color: #ffffff; border-top-left-radius: "
                           "20px; border-bottom-left-radius: 20px;");

    auto *layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(16, 20, 16, 20);
    layout->setSpacing(9);

    // Logo / Brand
    auto *logoRow = new QHBoxLayout();
    logoRow->setSpacing(10);
    auto *logoIcon = new QLabel("⚡");
    logoIcon->setStyleSheet(
        "background-color: #f97316; color: #ffffff; font-size: 16px; "
        "border-radius: 8px; padding: 4px 8px;");
    auto *logoText = new QLabel("TunnBit");
    logoText->setStyleSheet("font-size: 19px; font-weight: 800; color: #0f172a;");
    logoRow->addWidget(logoIcon);
    logoRow->addWidget(logoText);
    logoRow->addStretch();
    layout->addLayout(logoRow);

    layout->addSpacing(12);

    // User Profile Card
    auto *userCard = new QWidget();
    userCard->setStyleSheet(
        "background: #f8fafc; border-radius: 10px; padding: 6px 10px;");
    auto *userLayout = new QHBoxLayout(userCard);
    userLayout->setContentsMargins(4, 4, 4, 4);
    userLayout->setSpacing(8);
    auto *avatar = new QLabel("👨‍💻");
    avatar->setStyleSheet("font-size: 25px; background: #e2e8f0; border-radius: "
                          "16px; padding: 4px;");
    auto *userInfo = new QVBoxLayout();
    userInfo->setSpacing(1);
    auto *userName = new QLabel("Tunn Nguyen");
    userName->setStyleSheet("font-size: 15px; font-weight: 700; color: #0f172a;");
    auto *userRole = new QLabel("Quyền của user");
    userRole->setStyleSheet("font-size: 11px; color: #64748b;");
    userInfo->addWidget(userName);
    userInfo->addWidget(userRole);
    userLayout->addWidget(avatar);
    userLayout->addLayout(userInfo);
    userLayout->addStretch();
    layout->addWidget(userCard);

    layout->addSpacing(10);

    // Nav Item Helper
    m_navButtons.clear();
    auto addNavItem = [&](int navIndex, const QString &icon, const QString &text, bool isActive = false) {
        auto *btn = new QPushButton(QString("%1  %2").arg(icon, text));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(36);
        QString style =
            "QPushButton {"
            "   border: none;"
            "   text-align: left;"
            "   padding-left: 10px;"
            "   font-size: 13px;"
            "   border-radius: 8px;"
            "   color: %1;"
            "   background-color: %2;"
            "   font-weight: %3;"
            "}"
            "QPushButton:hover { background-color: #f1f5f9; color: #0f172a; }";

        if (isActive) {
            btn->setStyleSheet(style.arg("#0f172a", "#f1f5f9", "700"));
        } else {
            btn->setStyleSheet(style.arg("#64748b", "transparent", "500"));
        }

        connect(btn, &QPushButton::clicked, [this, navIndex]() {
            onNavButtonClicked(navIndex);
        });

        m_navButtons.append(btn);
        layout->addWidget(btn);
    };

    auto addSectionHeader = [&](const QString &title) {
        layout->addSpacing(8);
        auto *lbl = new QLabel(title);
        lbl->setStyleSheet("font-size: 11px; font-weight: 600; color: #94a3b8; "
                           "text-transform: uppercase; padding-left: 10px;");
        layout->addWidget(lbl);
    };

    addNavItem(0, "📊", "Dashboard");
    addNavItem(1, "⚡", "Tách & lọc tài khoản", true);
    addNavItem(2, "👥", "Chrome Profiles");
    addNavItem(3, "🌐", "Proxy Pool");

    addSectionHeader("Automation");
    addNavItem(4, "🚀", "Task Runner");
    addNavItem(5, "💳", "License Key");
    addNavItem(6, "📦", "Cookie Manager");

    addSectionHeader("System");
    addNavItem(7, "📈", "Reports & Logs");
    addNavItem(8, "⚙️", "Settings");

    layout->addStretch();

    // Logout button at bottom
    auto *btnLogout = new QPushButton("←  Logout");
    btnLogout->setCursor(Qt::PointingHandCursor);
    btnLogout->setStyleSheet(
        "QPushButton { border: none; text-align: left; padding-left: 10px; "
        "font-size: 13px; color: #94a3b8; font-weight: 600; } QPushButton:hover "
        "{ color: #ef4444; }");
    connect(btnLogout, &QPushButton::clicked, this, [this]() {
        close();
    });
    layout->addWidget(btnLogout);

    return sidebar;
}

QWidget *MainWindow::createTopBar()
{
    auto *topBar = new QWidget();
    topBar->setFixedHeight(60);
    topBar->setStyleSheet("background-color: #ffffff;");

    auto *layout = new QHBoxLayout(topBar);
    layout->setContentsMargins(24, 10, 24, 10);
    layout->setSpacing(16);

    layout->addStretch();

    // Sun/Moon & Notification
    auto *themeBtn = new QPushButton("☀️");
    themeBtn->setFixedSize(34, 34);
    themeBtn->setCursor(Qt::PointingHandCursor);
    themeBtn->setStyleSheet("QPushButton { background: #f8fafc; border: 1px "
                            "solid #e2e8f0; border-radius: 17px; font-size: "
                            "14px; } QPushButton:hover { background: #e2e8f0; }");

    auto *avatarBadge = new QLabel("👤");
    avatarBadge->setFixedSize(34, 34);
    avatarBadge->setAlignment(Qt::AlignCenter);
    avatarBadge->setStyleSheet("background: #ffedd5; border: 1px solid #fed7aa; "
                               "border-radius: 17px; font-size: 15px;");

    layout->addWidget(themeBtn);
    layout->addWidget(avatarBadge);

    return topBar;
}
/*
    nó nhaỷ vào đây mỗi khi tao nhấn chuyển trang


*/
void MainWindow::onNavButtonClicked(int index)
{
    if (index < 0 || index >= m_pageStack->count()){
//        qDebug() << "Total page:" << m_pageStack->count() << "| Clicked index:" << index;
        return;
    }

    qDebug() << ">>> Da click chuyen sang trang:" << index << "| Tong so trang:" << m_pageStack->count();

    for (int i = 0; i < m_navButtons.size(); ++i) {
        if (i == index) {
            m_navButtons[i]->setStyleSheet(
                "QPushButton { border: none; text-align: left; padding-left: 10px; "
                "font-size: 13px; border-radius: 8px; color: #0f172a; "
                "background-color: #f1f5f9; font-weight: 700; }");
        } else {
            m_navButtons[i]->setStyleSheet(
                "QPushButton { border: none; text-align: left; padding-left: 10px; "
                "font-size: 13px; border-radius: 8px; color: #64748b; "
                "background-color: transparent; font-weight: 500; } "
                "QPushButton:hover { background-color: #f1f5f9; color: #0f172a; }");
        }
    }

    m_pageStack->setCurrentIndex(index);

    // Show right summary panel ONLY on Account Parser page (index 1)
    if (index == 1) {
        if (m_rightDivider) m_rightDivider->show();
        if (m_rightPanel) m_rightPanel->show();
    } else {
        if (m_rightDivider) m_rightDivider->hide();
        if (m_rightPanel) m_rightPanel->hide();
    }

    // Refresh data hooks
    if (index == 0 && m_pageDashboard) {
        int totalProfiles = m_pageChromeProfiles->getProfiles().size();
        int runningProfiles = 0;
        for (const auto &p : m_pageChromeProfiles->getProfiles()) {
            if (p.isRunning) runningProfiles++;
        }
        int totalProxies = m_pageProxyPool->getTotalProxies();
        int liveProxies = m_pageProxyPool->getLiveProxies();
        auto *lm = LicenseManager::instance();
        QString licStatus = lm->isActivated() ? "VIP Kích Hoạt" : "Hết Hạn";
        QString licDays = QString::number(lm->daysRemaining());
        m_pageDashboard->updateMetrics(totalProfiles, runningProfiles, totalProxies, liveProxies, licStatus, licDays);
    } else if (index == 2 && m_pageChromeProfiles) {
        m_pageChromeProfiles->refreshProfileTable();
    } else if (index == 3 && m_pageProxyPool) {
        m_pageProxyPool->refreshProxyPoolTable();
    } else if (index == 5 && m_pageLicenseKey) {
        m_pageLicenseKey->refreshLicenseStatus();
    }
}

void MainWindow::openTaskRunnerPage(int subPage, bool autoStart)
{
    onNavButtonClicked(4); // Task Runner index
    if (m_pageTaskRunner) {
        m_pageTaskRunner->setSubPage(subPage, autoStart);
    }
}
