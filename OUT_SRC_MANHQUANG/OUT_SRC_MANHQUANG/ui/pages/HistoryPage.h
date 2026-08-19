#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QWidget>

namespace Ui { class HistoryPage; }
class QChart;
class QChartView;
class QLineSeries;

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
    void updateChart();

    Ui::HistoryPage *ui;
    QChart *m_chart = nullptr;
    QChartView *m_chartView = nullptr;
    QLineSeries *m_series = nullptr;
    QJsonArray m_cachedRows;
};
