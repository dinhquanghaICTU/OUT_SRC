#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDate>
#include <QJsonArray>
#include <QJsonObject>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QWidget>

class QChart;
class QChartView;
class QComboBox;
class QButtonGroup;
class QFrame;
class QGridLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class QTableWidget;
class QTimer;
class VirtualKeyboard;

/* =========================================================================
   TACTICAL RADAR SCAN WIDGET (Radar Giám Sát Chuyển Động Xoay 360 Độ)
   ========================================================================= */
class RadarScanWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RadarScanWidget(QWidget *parent = nullptr);
    void setMotionDetected(bool detected);
    bool isMotionDetected() const { return m_motionDetected; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_motionDetected = false;
    double m_angle = 0.0;
    double m_pulse = 0.0;
    bool m_pulseUp = true;
    QTimer *m_animTimer = nullptr;
};

/* =========================================================================
   BAROMETER DIAL GAUGE WIDGET (Đồng Hồ Áp Suất Khí Quyển Chuyên Dụng)
   ========================================================================= */
class BarometerDialWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BarometerDialWidget(QWidget *parent = nullptr);
    void setPressure(double pressureHpa);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_pressure = 1013.25;
    bool m_hasData = false;
};

/* =========================================================================
   THERMAL METER WIDGET (Thước Đo Nhiệt Độ Khí Quyển Môi Trường)
   ========================================================================= */
class ThermalMeterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ThermalMeterWidget(QWidget *parent = nullptr);
    void setTemperature(double tempC);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_temp = 28.0;
    bool m_hasData = false;
};

/* =========================================================================
   MAIN WINDOW (Trạm Chỉ Huy Quan Trắc Khí Quyển & Chuyển Động)
   ========================================================================= */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setBaseUrl(const QString &url) { m_baseUrl = url; }
    void triggerLogin(const QString &username = "admin", const QString &password = "admin", int targetPage = 0);
    void setInitialHistoryDate(const QDate &d);
    void setHistorySubTab(int index);

protected:
    void keyPressEvent(QKeyEvent *event) override;

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
    void logout();
    void refreshAll();
    void refreshDevices();
    void refreshAvailable();
    void refreshUsers();
    void refreshHistory();
    void renderDevices();
    void renderAvailable();
    void renderUsers();
    void renderHistory(const QJsonObject &history);
    void updateHistoryDateDisplay();
    void claimDevice(const QString &deviceId, const QString &name);
    void releaseDevice(const QString &deviceId);
    void toggleRelay(const QString &deviceId, bool nextState);
    void createUserDialog();
    void editUserDialog(const QJsonObject &user);
    void openAddDeviceDialog();
    void openClaimDeviceDialog(const QJsonObject &device);
    void openDeviceConfigDialog(const QString &deviceId);
    void updateDeviceConfig(const QString &deviceId, const QJsonObject &config);

    void showChartZoomDialog(const QString &key);

    QWidget *m_loginPage = nullptr;
    VirtualKeyboard *m_loginKeyboard = nullptr;
    QWidget *m_shellPage = nullptr;
    QStackedWidget *m_root = nullptr;
    QStackedWidget *m_pages = nullptr;
    QLabel *m_status = nullptr;
    QLabel *m_homeTitle = nullptr;
    QLabel *m_kpiDevices = nullptr;
    QLabel *m_kpiOnline = nullptr;
    QLabel *m_kpiType = nullptr;
    QStackedWidget *m_homeStack = nullptr;
    QWidget *m_homeLiveView = nullptr;
    QWidget *m_homeEmptyView = nullptr;
    
    // Telemetry Visual Widgets
    BarometerDialWidget *m_baroDial = nullptr;
    ThermalMeterWidget *m_thermalMeter = nullptr;
    RadarScanWidget *m_radarWidget = nullptr;

    QLabel *m_liveTemp = nullptr;
    QLabel *m_livePressure = nullptr;
    QLabel *m_liveMotion = nullptr;
    QLabel *m_liveMotionSub = nullptr;
    QLabel *m_liveStationName = nullptr;
    QLabel *m_liveStationId = nullptr;
    QLabel *m_liveStationStatus = nullptr;
    QFrame *m_liveMotionCard = nullptr;
    QPushButton *m_liveConfigBtn = nullptr;
    QPushButton *m_liveReleaseBtn = nullptr;
    // Device Manager Cockpit (Designed for 7-inch display)
    QStackedWidget *m_devLeftStack = nullptr;
    QWidget *m_devLeftActiveView = nullptr;
    QWidget *m_devLeftEmptyView = nullptr;
    QComboBox *m_devSelectCombo = nullptr;
    QLabel *m_devActiveName = nullptr;
    QLabel *m_devActiveId = nullptr;
    QLabel *m_devActiveStatus = nullptr;
    QLabel *m_devActivePressure = nullptr;
    QLabel *m_devActiveTemp = nullptr;
    QLabel *m_devActiveMotion = nullptr;
    QLabel *m_devActiveDetails = nullptr;
    QPushButton *m_devConfigBtn = nullptr;
    QPushButton *m_devReleaseBtn = nullptr;

    QStackedWidget *m_availStack = nullptr;
    QLabel *m_availDevId = nullptr;
    QLabel *m_availDevDesc = nullptr;
    QPushButton *m_availClaimBtn = nullptr;
    QComboBox *m_historyDevice = nullptr;
    QComboBox *m_historyPeriod = nullptr;
    QLabel *m_histDateDisplay = nullptr;
    QPushButton *m_histDatePrev = nullptr;
    QPushButton *m_histDateNext = nullptr;
    QPushButton *m_histDateToday = nullptr;
    QDate m_historyDate = QDate::currentDate();
    QStackedWidget *m_historyStack = nullptr;
    QTableWidget *m_historyTable = nullptr;

    // View switch buttons
    QPushButton *m_histBtnPressure = nullptr;
    QPushButton *m_histBtnMotion = nullptr;
    QPushButton *m_histBtnTemp = nullptr;
    QPushButton *m_histBtnMulti = nullptr;
    QPushButton *m_histBtnTable = nullptr;

    // Dedicated Pressure Barogram widgets
    QLabel *m_histPressCur = nullptr;
    QLabel *m_histPressDelta = nullptr;
    QLabel *m_histPressEval = nullptr;
    QLabel *m_histPressAlt = nullptr;
    QLabel *m_histPressRange = nullptr;
    QChartView *m_histPressView = nullptr;

    // Dedicated Motion PIR Timeline widgets
    QLabel *m_histMotionCur = nullptr;
    QLabel *m_histMotionCount = nullptr;
    QLabel *m_histMotionLast = nullptr;
    QLabel *m_histMotionDuty = nullptr;
    QLabel *m_histMotionSummary = nullptr;
    QChartView *m_histMotionView = nullptr;

    // Dedicated Temperature Thermogram widgets
    QLabel *m_histTempCur = nullptr;
    QLabel *m_histTempMin = nullptr;
    QLabel *m_histTempMax = nullptr;
    QLabel *m_histTempAvg = nullptr;
    QLabel *m_histTempSummary = nullptr;
    QChartView *m_histTempView = nullptr;

    // Multi-stream Overview widgets
    QChartView *m_histMultiPressView = nullptr;
    QChartView *m_histMultiMotionView = nullptr;
    QChartView *m_histMultiTempView = nullptr;
    QTableWidget *m_usersTable = nullptr;
    QButtonGroup *m_navGroup = nullptr;

    QNetworkAccessManager m_net;
    QString m_baseUrl = QStringLiteral("http://127.0.0.1:8080");
    QString m_token;
    QString m_role;
    QString m_username;
    QJsonArray m_devices;
    QJsonArray m_available;
    QJsonArray m_users;
    QMap<QString, QJsonObject> m_deviceConfigs;
    QJsonObject m_lastHistory;
    QTimer *m_timer = nullptr;
};

#endif // MAINWINDOW_H
