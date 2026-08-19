#pragma once

#include <QDialog>
#include <QJsonArray>
#include <QListWidget>

class QPushButton;
class QLabel;

class SelectOnlineDeviceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SelectOnlineDeviceDialog(const QJsonArray &devices, QWidget *parent = nullptr);
    QString selectedDeviceId() const;
    QString selectedDeviceName() const;

private:
    QListWidget *m_listWidget;
    QPushButton *m_selectBtn;
    QPushButton *m_cancelBtn;
    QJsonArray m_devices;
    QString m_selectedId;
    QString m_selectedName;
};
