#ifndef REPORTSLOGSPAGE_H
#define REPORTSLOGSPAGE_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class ReportsLogsPage : public QWidget {
    Q_OBJECT

public:
    explicit ReportsLogsPage(QWidget *parent = nullptr);
    ~ReportsLogsPage() override = default;

    void appendLog(const QString &level, const QString &message);

public slots:
    void onExportLogs();
    void onClearLogs();
    void onFilterLevelChanged(int index);

private:
    void setupUi();

    QPlainTextEdit *m_logConsole = nullptr;
    QComboBox *m_filterLevelCombo = nullptr;
    QLabel *m_lblTotalLogs = nullptr;
    int m_logCount = 0;
};

#endif // REPORTSLOGSPAGE_H
