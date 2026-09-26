#ifndef ACCOUNTPARSERPAGE_H
#define ACCOUNTPARSERPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QFrame>

struct MmoAccount {
    QStringList fields;
    bool isValid = true;
    QString statusMsg;
};

class AccountParserPage : public QObject {
    Q_OBJECT

public:
    explicit AccountParserPage(QWidget *parent = nullptr);
    ~AccountParserPage() override = default;

    QWidget* getCenterWidget() { return m_centerWidget; }
    QWidget* getRightSummaryWidget() { return m_rightPanelWidget; }

public slots:
    void onImportFileClicked();
    void onProcessLinesClicked();
    void onExportExcelClicked();
    void onCopyValidClicked();
    void onClearAllClicked();
    void onSearchFilterChanged(const QString &text);
    void onPillFilterClicked(int filterIndex);
    void onFormatChanged(int index);

private:
    void setupCenterPanel();
    void setupRightSummaryPanel();
    void updateStats(int total, int valid, int error, int duplicates);
    void renderTable(const QList<MmoAccount> &accounts);
    QString getDelimiter() const;
    QStringList getCurrentHeaders() const;

    QWidget *m_parentWidget = nullptr;
    QWidget *m_centerWidget = nullptr;
    QWidget *m_rightPanelWidget = nullptr;

    // Parser Controls
    QPlainTextEdit *m_rawInputEdit = nullptr;
    QTableWidget *m_tableWidget = nullptr;
    QComboBox *m_delimiterCombo = nullptr;
    QComboBox *m_formatCombo = nullptr;
    QComboBox *m_exportFormatCombo = nullptr;
    QLineEdit *m_searchEdit = nullptr;

    QList<QPushButton*> m_pillButtons;
    int m_currentFilter = 0;

    QLabel *m_lblTotal = nullptr;
    QLabel *m_lblValid = nullptr;
    QLabel *m_lblError = nullptr;
    QLabel *m_lblDuplicates = nullptr;
    QLabel *m_lblTotalExport = nullptr;

    QList<MmoAccount> m_allAccounts;
};

#endif // ACCOUNTPARSERPAGE_H
