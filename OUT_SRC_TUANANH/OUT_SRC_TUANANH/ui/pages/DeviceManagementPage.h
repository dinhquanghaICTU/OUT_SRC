#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QHash>
#include <QWidget>

class QBoxLayout;
class QGridLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class QTableWidget;
class QTimer;
class QResizeEvent;

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

protected:
    void resizeEvent(QResizeEvent *event) override;

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
    void applyResponsiveLayout();
    void openDeviceConfigDialog(const QJsonObject &device);
    static void clearGrid(QGridLayout *layout);
    static QString deviceIcon(const QString &type);
    static QString deviceTypeName(const QString &type);
    static QString metricsSummary(const QJsonObject &metrics);

    QPushButton *m_logTabBtn = nullptr;
    QPushButton *m_cardsTabBtn = nullptr;
    QStackedWidget *m_viewStack = nullptr;
    QTableWidget *m_deviceLogTable = nullptr;
    QLineEdit *m_logSearchEdit = nullptr;
    QLabel *m_statTotalDevices = nullptr;
    QLabel *m_statOnlineDevices = nullptr;
    QLabel *m_statLinkedUsers = nullptr;
    QLabel *m_logEmptyLabel = nullptr;

    QGridLayout *m_ownedGrid = nullptr;
    QGridLayout *m_availableGrid = nullptr;
    QLabel *m_ownedEmpty = nullptr;
    QLabel *m_availableEmpty = nullptr;
    QLabel *m_liveLabel = nullptr;
    QTimer *m_refreshTimer = nullptr;

    QJsonArray m_ownedDevices;
    QJsonArray m_availableDevices;
    QString m_currentUsername;
    bool m_isAdmin = false;
    bool m_compact = false;
    int m_gridColumns = 2;
};
