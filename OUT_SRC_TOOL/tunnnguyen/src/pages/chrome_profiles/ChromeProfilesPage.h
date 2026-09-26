#ifndef CHROMEPROFILESPAGE_H
#define CHROMEPROFILESPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QList>
#include "core/ChromeProfileItem.h"

class ChromeLauncher;
class SyncManager;

class ChromeProfilesPage : public QWidget {
    Q_OBJECT

public:
    explicit ChromeProfilesPage(ChromeLauncher *launcher, SyncManager *syncManager, QWidget *parent = nullptr);
    ~ChromeProfilesPage() override = default;

    const QList<ChromeProfileItem>& getProfiles() const { return m_profiles; }
    void refreshProfileTable();
    void loadProfiles();
    void saveProfiles();

signals:
    void profilesChanged(const QList<ChromeProfileItem> &profiles);

public slots:
    void onCreateProfileClicked();
    void onImportProfilesClicked();
    void onExportProfilesTemplateClicked();
    void onOpenProfilesFolderClicked();
    void onToggleSyncBarClicked();
    void onToggleSelectAllProfiles();
    void onLaunchSelectedProfilesClicked();
    void onStopSelectedProfilesClicked();
    void onStopAllProfilesClicked();
    void onBatchAssignProxyClicked();
    void onDeleteSelectedProfilesClicked();
    void onArrangeGridClicked();
    void onProfileSearchFilterChanged(const QString &text);

private:
    void setupUi();
    void updateKpiBadges();
    void setupConnections();

    ChromeLauncher *m_launcher = nullptr;
    SyncManager *m_syncManager = nullptr;
    QList<ChromeProfileItem> m_profiles;

    // UI Widgets
    QLabel *m_lblTotalProfiles = nullptr;
    QLabel *m_lblActiveProfiles = nullptr;
    QLabel *m_lblProxyProfiles = nullptr;

    QLineEdit *m_searchEdit = nullptr;
    QComboBox *m_windowSizeCombo = nullptr;
    QPushButton *m_btnToggleSyncBar = nullptr;
    QTableWidget *m_profileTable = nullptr;

    QWidget *m_syncBarWidget = nullptr;
    QLabel *m_lblSyncStatus = nullptr;
    QPushButton *m_btnSyncToggleMaster = nullptr;

    bool m_allSelected = false;
    QString m_searchQuery;
};

#endif // CHROMEPROFILESPAGE_H
