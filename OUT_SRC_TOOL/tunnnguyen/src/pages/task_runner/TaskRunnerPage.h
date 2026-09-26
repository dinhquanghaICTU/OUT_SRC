#ifndef TASKRUNNERPAGE_H
#define TASKRUNNERPAGE_H

#include <QWidget>
#include <QStackedWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QSpinBox>
#include <QTimer>
#include "core/ChromeProfileItem.h"
#include "FreeFireLogic.h"

class TaskRunnerPage : public QWidget {
    Q_OBJECT

public:
    explicit TaskRunnerPage(QWidget *parent = nullptr);
    ~TaskRunnerPage() override = default;

    void setSubPage(int pageIndex, bool autoStart = false);
    void updateProfiles(const QList<ChromeProfileItem> &profiles);
    void logMessage(const QString &msg, const QString &type = "INFO");

private slots:
    void onStartScriptClicked();
    void onStopScriptClicked();
    void onTestConnectionClicked();
    void onSelectAccountFileClicked();
    void onRefreshAdbDevicesClicked();

private:
    void setupUi();
    QWidget* createStorePage();
    QWidget* createConfigPage();

    QStackedWidget *m_taskStack = nullptr; // Index 0: Store, Index 1: Config

    // Config Page Controls
    QComboBox *m_cmbAdbDevices = nullptr;
    QComboBox *m_cmbChromeProfiles = nullptr;
    QTextEdit *m_txtAccounts = nullptr;
    QPlainTextEdit *m_txtLog = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QPushButton *m_btnStart = nullptr;
    QPushButton *m_btnStop = nullptr;

    QLabel *m_lblStatusBadge = nullptr;
    QLabel *m_lblTotal = nullptr;
    QLabel *m_lblSuccess = nullptr;
    QLabel *m_lblFailed = nullptr;

    QCheckBox *m_chkChangePassword = nullptr;
    QCheckBox *m_chkChangeEmail = nullptr;
    QCheckBox *m_chkRemovePhone = nullptr;
    QCheckBox *m_chkExportResult = nullptr;
    QSpinBox *m_spinThreads = nullptr;
    QSpinBox *m_spinDelay = nullptr;

    // Execution state
    QTimer *m_simTimer = nullptr;
    QStringList m_loadedAccounts;
    int m_currentAccIndex = 0;
    int m_successCount = 0;
    int m_failCount = 0;
    bool m_isRunning = false;

    QList<ChromeProfileItem> m_cachedProfiles;
};

#endif // TASKRUNNERPAGE_H
