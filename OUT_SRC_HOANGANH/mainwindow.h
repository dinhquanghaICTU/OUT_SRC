#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QJsonArray>
#include <QJsonObject>
#include <QMainWindow>
#include <QNetworkAccessManager>

class QChart;
class QChartView;
class QComboBox;
class QFrame;
class QGridLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class QTableWidget;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QNetworkRequest request(const QString &path) const;
    void get(const QString &path, std::function<void(QJsonObject)> ok);
    void post(const QString &path, const QJsonObject &body, std::function<void(QJsonObject)> ok = {});
    void del(const QString &path, std::function<void(QJsonObject)> ok = {});
    static QString metricText(const QJsonObject &device);
    static QString deviceIcon(const QString &type);
    static QString deviceTypeName(const QString &type);

    void buildLogin();
    void buildShell();
    void buildHome();
    void buildDevices();
    void buildHistory();
    void buildUsers();
    void setPage(int index);
    void refreshAll();
    void refreshDevices();
    void refreshAvailable();
    void refreshUsers();
    void refreshHistory();
    void renderDevices();
    void renderAvailable();
    void renderUsers();
    void renderHistory(const QJsonObject &history);
    void claimDevice(const QString &deviceId, const QString &name);
    void releaseDevice(const QString &deviceId);
    void toggleRelay(const QString &deviceId, bool nextState);
    void createUserDialog();
    void editUserDialog(const QJsonObject &user);
    void openDeviceConfigDialog(const QJsonObject &device);
    void updateDeviceConfig(const QString &deviceId, const QJsonObject &config);

    QWidget *m_loginPage = nullptr;
    QWidget *m_shellPage = nullptr;
    QStackedWidget *m_root = nullptr;
    QStackedWidget *m_pages = nullptr;
    QLabel *m_status = nullptr;
    QLabel *m_homeTitle = nullptr;
    QLabel *m_kpiDevices = nullptr;
    QLabel *m_kpiOnline = nullptr;
    QLabel *m_kpiType = nullptr;
    QGridLayout *m_homeDeviceGrid = nullptr;
    QGridLayout *m_deviceGrid = nullptr;
    QGridLayout *m_availableGrid = nullptr;
    QComboBox *m_historyDevice = nullptr;
    QComboBox *m_historyPeriod = nullptr;
    QGridLayout *m_historyCharts = nullptr;
    QTableWidget *m_historyTable = nullptr;
    QTableWidget *m_usersTable = nullptr;

    QNetworkAccessManager m_net;
    QString m_baseUrl = QStringLiteral("http://127.0.0.1:8080");
    QString m_token;
    QString m_role;
    QString m_username;
    QJsonArray m_devices;
    QJsonArray m_available;
    QJsonArray m_users;
    QTimer *m_timer = nullptr;
};

#endif
