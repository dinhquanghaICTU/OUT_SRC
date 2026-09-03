#pragma once

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QWidget>

class QBoxLayout;
class QGridLayout;
class QDoubleSpinBox;
class QFormLayout;
class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QTableWidget;
class QTimer;
class QCheckBox;

class DeviceManagementPage final : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceManagementPage(QWidget *parent = nullptr);

    void setCurrentUser(const QString &username, bool isAdmin);
    void setOwnedDevices(const QJsonArray &devices);
    void setAvailableDevices(const QJsonArray &devices);
    void startRealtime();
    void stopRealtime();
    void configSaved(const QString &deviceId, bool mqttPublished);
    void onDeviceReleased(const QString &deviceId);

signals:
    void claimDeviceRequested(const QString &deviceId, const QString &name);
    void relayControlRequested(const QString &deviceId, bool state);
    void deviceConfigRequested(const QString &deviceId, const QJsonObject &config);
    void releaseDeviceRequested(const QString &deviceId);
    void refreshRequested();

private:
    QWidget *createOwnedCard(const QJsonObject &device);
    QWidget *createAvailableCard(const QJsonObject &device);
    void rebuildOwnedGrid();
    void rebuildAvailableGrid();
    void rebuildLogTable();
    void openDeviceDrawer(const QJsonObject &device);
    void saveThresholds();
    static void clearGrid(QGridLayout *layout);

    QPushButton *m_cardsTabBtn = nullptr;
    QPushButton *m_logTabBtn = nullptr;
    QStackedWidget *m_viewStack = nullptr;
    QTableWidget *m_deviceLogTable = nullptr;

    QGridLayout *m_ownedGrid = nullptr;
    QGridLayout *m_availableGrid = nullptr;
    QLabel *m_ownedEmpty = nullptr;
    QLabel *m_availableEmpty = nullptr;
    QTimer *m_refreshTimer = nullptr;

    // Drawer widgets
    QFrame *m_drawer = nullptr;
    QLabel *m_drawerTitle = nullptr;
    QLabel *m_drawerId = nullptr;
    QDoubleSpinBox *m_inputMinSoil = nullptr;
    QDoubleSpinBox *m_inputMaxSoil = nullptr;
    QSpinBox *m_inputMaxRuntime = nullptr;
    QCheckBox *m_chkAutoWatering = nullptr;
    QPushButton *m_saveConfigBtn = nullptr;
    QPushButton *m_releaseDevBtn = nullptr;

    QJsonObject m_selectedDevice;
    QJsonArray m_ownedDevices;
    QJsonArray m_availableDevices;
    QString m_currentUsername;
    bool m_isAdmin = false;
};
