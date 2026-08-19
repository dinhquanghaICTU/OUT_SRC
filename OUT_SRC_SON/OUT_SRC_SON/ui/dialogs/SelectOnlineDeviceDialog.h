#pragma once

#include <QDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

class QVBoxLayout;
class QLabel;

class SelectOnlineDeviceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectOnlineDeviceDialog(const QJsonArray &availableDevices, QWidget *parent = nullptr);
    void updateAvailableDevices(const QJsonArray &availableDevices);

signals:
    void deviceSelected(const QString &deviceId, const QString &deviceName);
    void refreshRequested();

private:
    void populateDeviceList(const QJsonArray &devices);

    QVBoxLayout *m_listLayout = nullptr;
    QLabel *m_emptyLabel = nullptr;
};
