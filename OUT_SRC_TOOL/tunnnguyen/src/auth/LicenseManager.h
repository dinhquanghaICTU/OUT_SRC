#pragma once

#include <QString>
#include <QObject>
#include <QDateTime>

class LicenseManager : public QObject {
    Q_OBJECT
public:
    static LicenseManager* instance();
    explicit LicenseManager(QObject *parent = nullptr);
    ~LicenseManager();

    void loadLicense();
    void saveLicense();

    bool activate(const QString &licenseKey);
    bool applyRenewal(const QString &planName, int months);
    
    bool isActivated() const;
    bool isExpired() const;
    int daysRemaining() const;

    QString getLicenseKey() const;
    QString getPlanName() const;
    QDateTime getExpiryDate() const;
    QDateTime getActivationDate() const;
    QString getHwid() const;
    QString getStatusBadgeText() const;

signals:
    void activationSuccess();
    void activationFailed(const QString &reason);
    void licenseUpdated();

private:
    static LicenseManager *s_instance;
    bool m_isActivated = false;
    QString m_licenseKey;
    QString m_planName;
    QDateTime m_activationDate;
    QDateTime m_expiryDate;
    QString m_hwid;
};
