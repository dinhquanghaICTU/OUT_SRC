#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QHash>
#include <QWidget>

class QGridLayout;
class QDoubleSpinBox;
class QFormLayout;
class QFrame;
class QLabel;
class QPushButton;
class QSpinBox;
class QTimer;

class DeviceManagementPage final : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceManagementPage(QWidget *parent = nullptr);
    void setOwnedDevices(const QJsonArray &devices);
    void setAvailableDevices(const QJsonArray &devices);
    void startRealtime();
    void stopRealtime();
    void configSaved(const QString &deviceId, bool mqttPublished);

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
    void openDeviceDrawer(const QJsonObject &device);
    void rebuildThresholdForm(const QJsonObject &device);
    void saveThresholds();
    static void clearGrid(QGridLayout *layout);
    static QString deviceIcon(const QString &type);
    static QString deviceTypeName(const QString &type);
    static QString metricsSummary(const QJsonObject &metrics);

    QGridLayout *m_ownedGrid;
    QGridLayout *m_availableGrid;
    QLabel *m_ownedEmpty;
    QLabel *m_availableEmpty;
    QLabel *m_liveLabel;
    QTimer *m_refreshTimer;
    QFrame *m_drawer;
    QLabel *m_drawerIcon;
    QLabel *m_drawerName;
    QLabel *m_drawerId;
    QLabel *m_drawerMetrics;
    QLabel *m_thresholdTitle;
    QFormLayout *m_thresholdForm;
    QSpinBox *m_samplingInterval;
    QPushButton *m_saveThresholds;
    QPushButton *m_releaseDevice;
    QJsonObject m_selectedDevice;
    QHash<QString, QDoubleSpinBox *> m_thresholdInputs;
    QJsonArray m_ownedDevices;
    QJsonArray m_availableDevices;
};
