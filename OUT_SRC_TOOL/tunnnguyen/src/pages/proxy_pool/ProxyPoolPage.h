#ifndef PROXYPOOLPAGE_H
#define PROXYPOOLPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QList>
#include "core/ChromeProfileItem.h"

struct ProxyPoolItem {
    QString rawProxy;
    QStringList associatedProfileNames;
    bool checked = false;
    bool isChecking = false;
    bool isLive = false;
    qint64 pingMs = -1;
    QString country;
    QString countryCode;
    QString flagEmoji;
    QString publicIp;
    QString errorMsg;
};

class ProxyChecker;

class ProxyPoolPage : public QWidget {
    Q_OBJECT

public:
    explicit ProxyPoolPage(QWidget *parent = nullptr);
    ~ProxyPoolPage() override = default;

    void updateProfiles(const QList<ChromeProfileItem> &profiles);
    void refreshProxyPoolTable();

    int getTotalProxies() const { return m_proxyItems.size(); }
    int getLiveProxies() const;

private slots:
    void onCheckAllProxiesClicked();
    void onCheckSingleProxy(int index);
    void onSyncProxiesFromProfiles();
    void onProxyPoolSearchChanged(const QString &text);
    void onFilterProxyStatusChanged(int index);

private:
    void setupUi();
    void updateProxyPoolKpis();

    QList<ChromeProfileItem> m_cachedProfiles;
    QList<ProxyPoolItem> m_proxyItems;
    ProxyChecker *m_proxyChecker = nullptr;

    // UI Widgets
    QLabel *m_lblStatTotalProxy = nullptr;
    QLabel *m_lblStatLiveProxy = nullptr;
    QLabel *m_lblStatDieProxy = nullptr;

    QLineEdit *m_searchEdit = nullptr;
    QComboBox *m_filterCombo = nullptr;
    QPushButton *m_btnCheckAll = nullptr;
    QTableWidget *m_proxyTable = nullptr;

    QString m_searchQuery;
    int m_filterStatus = 0;
};

#endif // PROXYPOOLPAGE_H
