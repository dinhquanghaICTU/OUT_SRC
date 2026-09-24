#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QTabWidget>
#include <QStackedWidget>
#include <QFrame>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ChromeLauncher;
class SyncManager;
class QCheckBox;
class QSpinBox;
class QNetworkAccessManager;

struct MmoAccount {
    QStringList fields;
    bool isValid = true;
    QString statusMsg;
};

#include "core/ChromeProfileItem.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Navigation
    void onNavButtonClicked(int index);

    // Account Parser Slots
    void onImportFileClicked();
    void onProcessLinesClicked();
    void onExportExcelClicked();
    void onCopyValidClicked();
    void onClearAllClicked();
    void onSearchFilterChanged(const QString &text);
    void onPillFilterClicked(int filterIndex);
    void onFormatChanged(int index);

    // Chrome Profiles Slots
    void onCreateProfileClicked();
    void onImportProfilesClicked();
    void onExportProfilesTemplateClicked();
    void onLaunchProfile(int index, int slotOrder = -1);
    void onStopProfile(int index);
    void onStopAllProfilesClicked();
    void onLaunchSelectedProfilesClicked();
    void onStopSelectedProfilesClicked();
    void onToggleSelectAllProfiles();
    void onOpenProfilesFolderClicked();
    void onDeleteProfile(int index);
    void onEditProfile(int index);
    void onQuickEditProxy(int index);
    void onProfileSearchFilterChanged(const QString &text);
    void onToggleSyncBarClicked();
    void onStartOrStopSyncClicked();
    void updateSyncMasterCombo();

    // Proxy Pool Slots
    void onCheckAllProxiesClicked();
    void onFilterProxyStatusChanged(int index);
    void onProxyPoolSearchChanged(const QString &text);
    void onSyncProxiesFromProfiles();

    // License & Renewal Slots
    void onSelectLicensePlan(int planIndex);
    void onActivateLicenseKeyClicked();
    void onCheckPaymentOnlineClicked();
    void onCopyTextToClipboard(const QString &text, const QString &desc);

private:
    void setupCustomUi();
    QWidget* createSidebar();
    QWidget* createTopBar();
    QWidget* createDashboardPanel();
    QWidget* createCenterPanel();
    QWidget* createRightSummaryPanel();
    QWidget* createChromeProfilesPanel();
    QWidget* createProxyPoolPanel();
    QWidget* createLicenseKeyPanel();

    // Dashboard Helpers
    void refreshDashboardPage();

    // License Helpers
    void refreshLicensePage();
    void updateVietQrCode();

    // Proxy Pool Helpers
    void refreshProxyPoolTable();
    void checkSingleProxy(int itemIndex);
    void updateProxyPoolKpis();

    // Account Parser helpers
    void updateStats(int total, int valid, int error, int duplicates);
    void renderTable(const QList<MmoAccount> &accounts);
    QString getDelimiter() const;
    QStringList getCurrentHeaders() const;

    // Chrome Profiles helpers
    void refreshProfileTable();
    void loadProfiles();
    void saveProfiles();
    QString getProfilesDir() const;
    QPair<int, int> getActiveWindowSize(const ChromeProfileItem &item) const;
    QPoint calculateTilePosition(int width, int height, int slotIndex) const;

    Ui::MainWindow *ui;

    // Navigation & Page Stack
    QStackedWidget *m_pageStack = nullptr;
    QWidget *m_rightPanel = nullptr;
    QFrame *m_rightDivider = nullptr;
    QList<QPushButton*> m_navButtons;

    // Parser Controls
    QPlainTextEdit *m_rawInputEdit = nullptr;
    QTableWidget *m_tableWidget = nullptr;
    QComboBox *m_delimiterCombo = nullptr;
    QComboBox *m_formatCombo = nullptr;
    QComboBox *m_exportFormatCombo = nullptr;
    QLineEdit *m_searchEdit = nullptr;

    // Filter Buttons (Pills)
    QList<QPushButton*> m_pillButtons;
    int m_currentFilter = 0; // 0: All, 1: Valid, 2: Error, 3: Duplicates

    // Summary Labels
    QLabel *m_lblTotal = nullptr;
    QLabel *m_lblValid = nullptr;
    QLabel *m_lblError = nullptr;
    QLabel *m_lblDuplicates = nullptr;
    QLabel *m_lblTotalExport = nullptr;

    QList<MmoAccount> m_allAccounts;

    // Chrome Profiles Controls
    QTableWidget *m_profileTable = nullptr;
    QLineEdit *m_profileSearchEdit = nullptr;
    QComboBox *m_profileWindowSizeCombo = nullptr;
    QLabel *m_lblTotalProfiles = nullptr;
    QLabel *m_lblActiveProfiles = nullptr;
    QLabel *m_lblProxyProfiles = nullptr;
    QList<ChromeProfileItem> m_profiles;

    // Master-Worker Synchronizer
    SyncManager *m_syncManager = nullptr;
    QWidget *m_syncBarWidget = nullptr;
    QPushButton *m_btnToggleSyncBar = nullptr;
    QComboBox *m_syncMasterCombo = nullptr;
    QCheckBox *m_syncMouseCheck = nullptr;
    QCheckBox *m_syncKeyCheck = nullptr;
    QCheckBox *m_syncScrollCheck = nullptr;
    QCheckBox *m_syncNavCheck = nullptr;
    QSpinBox *m_syncDelaySpin = nullptr;
    QPushButton *m_btnStartSync = nullptr;

    // Proxy Pool Controls & Data
    struct ProxyPoolItem {
        QString rawProxy;
        QStringList associatedProfileNames;
        bool isChecking = false;
        bool checked = false;
        bool isLive = false;
        int pingMs = -1;
        QString country;
        QString countryCode;
        QString publicIp;
        QString flagEmoji;
        QString errorMsg;
    };
    QList<ProxyPoolItem> m_proxyPoolItems;
    QTableWidget *m_proxyPoolTable = nullptr;
    QLineEdit *m_proxyPoolSearchEdit = nullptr;
    QComboBox *m_proxyPoolFilterCombo = nullptr;
    QLabel *m_lblStatTotalProxy = nullptr;
    QLabel *m_lblStatLiveProxy = nullptr;
    QLabel *m_lblStatDieProxy = nullptr;
    QPushButton *m_btnCheckAllProxies = nullptr;

    // License Key & Online Payment Controls
    struct SubscriptionPlan {
        QString title;
        QString durationText;
        int months;
        int priceVnd;
        QString saveBadge;
        bool isPopular = false;
    };
    QList<SubscriptionPlan> m_subscriptionPlans;
    int m_selectedPlanIndex = 2; // Default: 1 Year (Popular)

    QLabel *m_lblLicenseStatusBadge = nullptr;
    QLabel *m_lblLicensePlanName = nullptr;
    QLabel *m_lblLicenseExpiry = nullptr;
    QLabel *m_lblLicenseDaysLeft = nullptr;
    QLineEdit *m_licenseHwidEdit = nullptr;
    QLineEdit *m_licenseKeyInput = nullptr;
    QLabel *m_lblLicenseKeyMsg = nullptr;

    QList<QFrame*> m_planCardWidgets;
    QLabel *m_lblQrImage = nullptr;
    QLabel *m_lblTransferBank = nullptr;
    QLabel *m_lblTransferAccount = nullptr;
    QLabel *m_lblTransferOwner = nullptr;
    QLabel *m_lblTransferAmount = nullptr;
    QLabel *m_lblTransferContent = nullptr;
    QLabel *m_lblPaymentStatusFeedback = nullptr;
    QNetworkAccessManager *m_qrNam = nullptr;

    // Dashboard Controls
    QLabel *m_lblDashTotalProfiles = nullptr;
    QLabel *m_lblDashRunningProfiles = nullptr;
    QLabel *m_lblDashTotalProxies = nullptr;
    QLabel *m_lblDashLiveProxies = nullptr;
    QLabel *m_lblDashLicenseStatus = nullptr;
    QLabel *m_lblDashLicenseDays = nullptr;
};
#endif // MAINWINDOW_H
