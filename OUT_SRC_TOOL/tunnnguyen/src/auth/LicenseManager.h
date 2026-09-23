#pragma once

#include <QString>
#include <QObject>

class LicenseManager : public QObject {
    Q_OBJECT
public:
    explicit LicenseManager(QObject *parent = nullptr);
    ~LicenseManager();

    bool activate(const QString &licenseKey);
    bool isActivated() const;

signals:
    void activationSuccess();
    void activationFailed(const QString &reason);

private:
    bool m_isActivated = false;
};
