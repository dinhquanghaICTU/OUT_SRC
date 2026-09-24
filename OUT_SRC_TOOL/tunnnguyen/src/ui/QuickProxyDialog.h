#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QString>

class ProxyInputWidget;

class QuickProxyDialog : public QDialog {
    Q_OBJECT
public:
    explicit QuickProxyDialog(const QString &profileName, const QString &currentProxy, QWidget *parent = nullptr);
    ~QuickProxyDialog() override = default;

    QString getProxy() const;

private:
    void setupUi(const QString &profileName, const QString &currentProxy);

    ProxyInputWidget *m_proxyWidget = nullptr;
};
