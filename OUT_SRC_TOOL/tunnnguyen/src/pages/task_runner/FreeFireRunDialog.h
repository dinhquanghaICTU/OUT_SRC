#ifndef FREEFIRERUNDIALOG_H
#define FREEFIRERUNDIALOG_H

#include <QDialog>
#include <QStackedWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QSpinBox>
#include <QTimer>
#include <QList>
#include "core/ChromeProfileItem.h"

class FreeFireRunDialog : public QDialog {
    Q_OBJECT

public:
    explicit FreeFireRunDialog(const QList<ChromeProfileItem> &profiles, QWidget *parent = nullptr);
    ~FreeFireRunDialog() override;

private slots:
    void onRefreshProfilesClicked();
    void onScanAdbClicked();
    void onSelectAllProfiles(bool checked);
    void onBrowseFileClicked();
    void onClearFileClicked();
    void onStartExecution();
    void onStopExecution();
    void onBackToConfig();
    void executeNextAccount();

private:
    void setupUi();
    QWidget* createConfigPage();
    QWidget* createExecutionPage();
    void populateProfileTable();
    void reloadProfilesFromDisk();
    void logMessage(const QString &msg, const QString &type = "INFO");

    QStackedWidget *m_stack = nullptr;

    // Config Page Controls
    QComboBox *m_cmbAdb = nullptr;
    QPushButton *m_btnScanAdb = nullptr;
    QLabel *m_lblAdbStatus = nullptr;

    QTableWidget *m_tableProfiles = nullptr;
    QPushButton *m_btnSyncChrome = nullptr;
    QCheckBox *m_chkSelectAllProfiles = nullptr;
    QCheckBox *m_chkResetDevice = nullptr;

    // File input controls
    QLineEdit *m_txtFilePath = nullptr;
    QPushButton *m_btnBrowseFile = nullptr;
    QPushButton *m_btnClearFile = nullptr;
    QLabel *m_lblFileInfo = nullptr;
    QString m_selectedFilePath;

    QCheckBox *m_chkChangePassword = nullptr;
    QCheckBox *m_chkChangeEmail = nullptr;
    QCheckBox *m_chkRemovePhone = nullptr;
    QCheckBox *m_chkExportResult = nullptr;
    QSpinBox *m_spinDelay = nullptr;

    QPushButton *m_btnStart = nullptr;
    QPushButton *m_btnCancel = nullptr;

    // Execution Page Controls
    QLabel *m_lblStatusBadge = nullptr;
    QLabel *m_lblTotal = nullptr;
    QLabel *m_lblProcessing = nullptr;
    QLabel *m_lblSuccess = nullptr;
    QLabel *m_lblFailed = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QPlainTextEdit *m_txtLog = nullptr;
    QPushButton *m_btnStopRun = nullptr;
    QPushButton *m_btnBackConfig = nullptr;
    QPushButton *m_btnClose = nullptr;

    // Execution state
    QTimer *m_timer = nullptr;
    QStringList m_accounts;
    int m_currentIndex = 0;
    int m_successCount = 0;
    int m_failCount = 0;
    bool m_isRunning = false;
    QString m_selectedAdbDevice;

    QList<ChromeProfileItem> m_profiles;
    QList<ChromeProfileItem> m_selectedProfiles;
};

#endif // FREEFIRERUNDIALOG_H
