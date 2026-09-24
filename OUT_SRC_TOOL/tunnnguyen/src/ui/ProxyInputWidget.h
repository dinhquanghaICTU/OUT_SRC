#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include "core/ProxyConfig.h"

class ProxyInputWidget : public QWidget {
    Q_OBJECT
public:
    explicit ProxyInputWidget(QWidget *parent = nullptr);
    ~ProxyInputWidget() override = default;

    void setProxy(const ProxyConfig &cfg);
    void setProxyString(const QString &rawProxy);
    ProxyConfig getProxy() const;
    QString getProxyString() const;

signals:
    void proxyChanged();

private slots:
    void onQuickPasteChanged(const QString &text);
    void onFieldChanged();

private:
    void setupUi();

    bool m_updating = false;
    QComboBox *m_protocolCombo = nullptr;
    QLineEdit *m_hostEdit = nullptr;
    QSpinBox *m_portSpin = nullptr;
    QLineEdit *m_userEdit = nullptr;
    QLineEdit *m_passEdit = nullptr;
    QLineEdit *m_quickPasteEdit = nullptr;
};
