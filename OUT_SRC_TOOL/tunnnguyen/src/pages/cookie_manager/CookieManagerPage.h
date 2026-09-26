#ifndef COOKIEMANAGERPAGE_H
#define COOKIEMANAGERPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QPlainTextEdit>
#include <QComboBox>

class CookieManagerPage : public QWidget {
    Q_OBJECT

public:
    explicit CookieManagerPage(QWidget *parent = nullptr);
    ~CookieManagerPage() override = default;

public slots:
    void onImportCookies();
    void onExportCookies();
    void onConvertFormat();
    void onClearAll();

private:
    void setupUi();

    QLineEdit *m_searchEdit = nullptr;
    QTableWidget *m_cookieTable = nullptr;
    QPlainTextEdit *m_rawCookieInput = nullptr;
    QComboBox *m_profileSelector = nullptr;
    QLabel *m_lblTotalCookies = nullptr;
};

#endif // COOKIEMANAGERPAGE_H
