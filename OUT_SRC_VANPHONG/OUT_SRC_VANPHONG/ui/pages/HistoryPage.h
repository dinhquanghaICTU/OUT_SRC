#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QWidget>

namespace Ui { class HistoryPage; }

class HistoryPage : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryPage(QWidget *parent = nullptr);
    ~HistoryPage() override;

    void setDevices(const QJsonArray &devices);
    void setHistory(const QJsonObject &history);

signals:
    void historyRequested(const QString &deviceId, const QString &period, const QString &date);

private:
    void requestCurrentHistory();

    Ui::HistoryPage *ui;
    QJsonArray m_devices;
    QString m_selectedDeviceId;
};
