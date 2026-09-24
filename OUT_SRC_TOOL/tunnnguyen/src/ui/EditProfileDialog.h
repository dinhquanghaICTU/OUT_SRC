#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QStringList>

class EditProfileDialog : public QDialog {
    Q_OBJECT
public:
    explicit EditProfileDialog(const QString &currentName,
                               const QString &currentDevice,
                               const QString &currentUserAgent,
                               const QString &currentProxy,
                               int currentPort,
                               int currentWidth,
                               int currentHeight,
                               const QStringList &otherNames = QStringList(),
                               QWidget *parent = nullptr);
    ~EditProfileDialog() override = default;

    QString getProfileName() const;
    QString getDeviceType() const;
    QString getUserAgent() const;
    QString getProxy() const;
    int getPort() const;
    int getWindowWidth() const;
    int getWindowHeight() const;

    static QString getUserAgentForDevice(const QString &deviceType, const QString &customUa = "");
    static QString getDeviceIcon(const QString &deviceType);
    static QString getDeviceDisplayName(const QString &deviceType);

private:
    void setupUi();
    void onDeviceChanged(int index);

    QString m_originalName;
    QString m_deviceType;
    QString m_userAgent;
    QString m_proxy;
    int m_port = 9222;
    int m_width = 0;
    int m_height = 0;
    QStringList m_otherNames;

    QLineEdit *m_nameEdit = nullptr;
    QComboBox *m_deviceCombo = nullptr;
    QLineEdit *m_uaEdit = nullptr;
    class ProxyInputWidget *m_proxyWidget = nullptr;
    QSpinBox *m_portSpin = nullptr;
    QComboBox *m_sizeCombo = nullptr;
    QWidget *m_customSizeWidget = nullptr;
    QSpinBox *m_widthSpin = nullptr;
    QSpinBox *m_heightSpin = nullptr;
};
