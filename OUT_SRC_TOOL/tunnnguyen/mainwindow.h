#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ChromeLauncher;
class SyncManager;

// Modular Pages
class DashboardPage;
class AccountParserPage;
class ChromeProfilesPage;
class ProxyPoolPage;
class TaskRunnerPage;
class LicenseKeyPage;
class CookieManagerPage;
class ReportsLogsPage;
class SettingsPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void openTaskRunnerPage(int subPage = 0, bool autoStart = false);

public slots:
    void onNavButtonClicked(int index);

private:
    void setupCustomUi();
    QWidget* createSidebar();
    QWidget* createTopBar();

    Ui::MainWindow *ui = nullptr;

    ChromeLauncher *m_launcher = nullptr;
    SyncManager *m_syncManager = nullptr;

    QStackedWidget *m_pageStack = nullptr;
    QWidget *m_rightPanel = nullptr;
    QFrame *m_rightDivider = nullptr;
    QList<QPushButton*> m_navButtons;

    // Modular Page Instances (Organized in src/pages/)
    DashboardPage *m_pageDashboard = nullptr;           // src/pages/dashboard/
    AccountParserPage *m_pageAccountParser = nullptr;   // src/pages/account_parser/
    ChromeProfilesPage *m_pageChromeProfiles = nullptr; // src/pages/chrome_profiles/
    ProxyPoolPage *m_pageProxyPool = nullptr;           // src/pages/proxy_pool/
    TaskRunnerPage *m_pageTaskRunner = nullptr;         // src/pages/task_runner/
    LicenseKeyPage *m_pageLicenseKey = nullptr;         // src/pages/license_key/
    CookieManagerPage *m_pageCookieManager = nullptr;   // src/pages/cookie_manager/
    ReportsLogsPage *m_pageReportsLogs = nullptr;       // src/pages/reports_logs/
    SettingsPage *m_pageSettings = nullptr;             // src/pages/settings/
};

#endif // MAINWINDOW_H
