#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QStringList>

class CreateProfileDialog : public QDialog {
    Q_OBJECT
public:
    explicit CreateProfileDialog(const QString &suggestedName = "Profile 01",
                                int suggestedPort = 9222,
                                const QStringList &existingNames = QStringList(),
                                QWidget *parent = nullptr);
    ~CreateProfileDialog() override = default;

    QString getProfileName() const;
    QString getDeviceType() const;
    QString getUserAgent() const;
    QString getProxy() const;
    int getPort() const;
    int getWindowWidth() const;
    int getWindowHeight() const;

private:
    void setupUi(const QString &suggestedName, int suggestedPort);

    QStringList m_existingNames;
    QLineEdit *m_nameEdit = nullptr;
    class QComboBox *m_deviceCombo = nullptr;
    QLineEdit *m_uaEdit = nullptr;
    class ProxyInputWidget *m_proxyWidget = nullptr;
    QSpinBox *m_portSpin = nullptr;
    class QComboBox *m_sizeCombo = nullptr;
    QSpinBox *m_widthSpin = nullptr;
    QSpinBox *m_heightSpin = nullptr;
    QWidget *m_customSizeWidget = nullptr;
};
