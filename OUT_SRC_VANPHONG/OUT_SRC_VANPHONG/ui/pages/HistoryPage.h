#pragma once

#include <QWidget>
#include <QJsonArray>
#include <QJsonObject>
#include <QDate>

namespace Ui { class HistoryPage; }
class QChart;
class QChartView;
class QLineSeries;
class QBarSeries;

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

private slots:
    void requestCurrentHistory();
    void rebuildDateOptions();
    void updateChart();

private:
    void setupChart();
    static QString metricTitle(const QString &key);

    Ui::HistoryPage *ui;
    QJsonArray m_devices;
    QString m_selectedDeviceId;
    QDate m_selectedDate;

    QChart *m_chart = nullptr;
    QChartView *m_chartView = nullptr;
    QString m_selectedMetricKey = QStringLiteral("soil_moisture");

    QJsonArray m_cachedRows;
    QJsonObject m_cachedHistory;
};
