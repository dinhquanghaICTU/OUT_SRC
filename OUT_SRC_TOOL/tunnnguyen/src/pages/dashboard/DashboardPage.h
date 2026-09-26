#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class DashboardPage : public QWidget {
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage() override = default;

    void updateMetrics(int totalProfiles, int runningProfiles,
                       int totalProxies, int liveProxies,
                       const QString &licenseStatus, const QString &licenseDays);

signals:
    void requestNavigate(int navIndex);

private:
    void setupUi();

    QLabel *m_lblTotalProfiles = nullptr;
    QLabel *m_lblRunningProfiles = nullptr;
    QLabel *m_lblTotalProxies = nullptr;
    QLabel *m_lblLiveProxies = nullptr;
    QLabel *m_lblLicenseStatus = nullptr;
    QLabel *m_lblLicenseDays = nullptr;
};

#endif // DASHBOARDPAGE_H
