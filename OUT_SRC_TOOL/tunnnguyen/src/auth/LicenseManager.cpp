#include "auth/LicenseManager.h"

LicenseManager::LicenseManager(QObject *parent)
    : QObject(parent)
{
}

LicenseManager::~LicenseManager()
{
}

bool LicenseManager::activate(const QString &licenseKey)
{
    if (licenseKey.isEmpty()) {
        emit activationFailed("License key cannot be empty.");
        return false;
    }
    // TODO: Verify with Server
    m_isActivated = true;
    emit activationSuccess();
    return true;
}

bool LicenseManager::isActivated() const
{
    return m_isActivated;
}
