#pragma once

#include <QWidget>

class QResizeEvent;

#include <QJsonArray>
#include <QJsonObject>

namespace Ui { class HistoryPage; }
class QChart;
class QChartView;
class QLabel;
class QFrame;
class QGridLayout;

class HistoryPage : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryPage(QWidget *parent = nullptr);
    ~HistoryPage() override;

    void setDevices(const QJsonArray &devices);
    void setHistory(const QJsonObject &history);

signals:
    void historyRequested(const QString &deviceId, const QString &period,
                          const QString &date);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void requestCurrentHistory();
    void applyResponsiveLayout();
    void updateChart(const QJsonArray &keys, const QJsonArray &rows);
    QString currentDeviceType() const;
    static QString metricTitle(const QString &key);
    static QString compactMetricTitle(const QString &key);
    static QString metricUnit(const QString &key);
    Ui::HistoryPage *ui;
    QChart *m_chart;
    QChartView *m_chartView;
    QLabel *m_primaryStat;
    QLabel *m_secondaryStat;
    QLabel *m_thirdStat;
    QLabel *m_chartHint;
    QLabel *m_headerSubtitle;
    QGridLayout *m_analyticsGrid;
    QFrame *m_chartCard;
    QFrame *m_primaryStatCard;
    QFrame *m_secondaryStatCard;
    QFrame *m_summaryStatCard;
};
