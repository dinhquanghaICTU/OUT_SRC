#include "auth/LicenseManager.h"
#include "auth/HwidHelper.h"
#include <QSettings>
#include <QDateTime>
#include <QRandomGenerator>

LicenseManager* LicenseManager::s_instance = nullptr;

LicenseManager* LicenseManager::instance()
{
    if (!s_instance) {
        s_instance = new LicenseManager();
    }
    return s_instance;
}

LicenseManager::LicenseManager(QObject *parent)
    : QObject(parent)
{
    m_hwid = HwidHelper::getHwid();
    loadLicense();
}

LicenseManager::~LicenseManager()
{
}

void LicenseManager::loadLicense()
{
    QSettings settings("TunnBit", "TunnAutomation");
    m_isActivated = settings.value("license/activated", false).toBool();
    m_licenseKey = settings.value("license/key", "").toString();
    m_planName = settings.value("license/plan", "").toString();
    
    QString actStr = settings.value("license/activation_date", "").toString();
    if (!actStr.isEmpty()) {
        m_activationDate = QDateTime::fromString(actStr, Qt::ISODate);
    } else {
        m_activationDate = QDateTime::currentDateTime();
    }

    QString expStr = settings.value("license/expiry_date", "").toString();
    if (!expStr.isEmpty()) {
        m_expiryDate = QDateTime::fromString(expStr, Qt::ISODate);
    } else {
        // Default 7 days trial if never initialized
        m_expiryDate = QDateTime::currentDateTime().addDays(7);
        m_planName = "Bản Dùng Thử (Trial 7 Ngày)";
        m_licenseKey = "TRIAL-" + m_hwid.left(8);
        m_isActivated = true; // allow trial usage out-of-the-box
        saveLicense();
    }
}

void LicenseManager::saveLicense()
{
    QSettings settings("TunnBit", "TunnAutomation");
    settings.setValue("license/activated", m_isActivated);
    settings.setValue("license/key", m_licenseKey);
    settings.setValue("license/plan", m_planName);
    settings.setValue("license/activation_date", m_activationDate.toString(Qt::ISODate));
    settings.setValue("license/expiry_date", m_expiryDate.toString(Qt::ISODate));
    settings.setValue("license/hwid", m_hwid);
    settings.sync();
}

bool LicenseManager::activate(const QString &licenseKey)
{
    QString key = licenseKey.trimmed().toUpper();
    if (key.isEmpty()) {
        emit activationFailed("Vui lòng nhập mã bản quyền (License Key).");
        return false;
    }

    if (key.length() < 10) {
        emit activationFailed("Mã bản quyền không hợp lệ! Định dạng mẫu: TUNN-PRO-XXXX-XXXX");
        return false;
    }

    // Determine plan type from key
    m_licenseKey = key;
    m_activationDate = QDateTime::currentDateTime();

    if (key.contains("LIFE") || key.contains("VINHVIEN")) {
        m_planName = "Gói Vĩnh Viễn (Lifetime VIP Pro)";
        m_expiryDate = QDateTime(QDate(2099, 12, 31), QTime(23, 59, 59));
    } else if (key.contains("YEAR") || key.contains("12M") || key.contains("PRO")) {
        m_planName = "Gói Chuyên Nghiệp (1 Năm)";
        m_expiryDate = QDateTime::currentDateTime().addDays(365);
    } else if (key.contains("3M") || key.contains("QUARTER")) {
        m_planName = "Gói Tiêu Chuẩn (3 Tháng)";
        m_expiryDate = QDateTime::currentDateTime().addDays(90);
    } else if (key.contains("1M") || key.contains("MONTH")) {
        m_planName = "Gói Cá Nhân (1 Tháng)";
        m_expiryDate = QDateTime::currentDateTime().addDays(30);
    } else {
        m_planName = "Gói VIP Doanh Nghiệp (1 Năm)";
        m_expiryDate = QDateTime::currentDateTime().addDays(365);
    }

    m_isActivated = true;
    saveLicense();

    emit activationSuccess();
    emit licenseUpdated();
    return true;
}

bool LicenseManager::applyRenewal(const QString &planName, int months)
{
    m_planName = planName;
    m_activationDate = QDateTime::currentDateTime();

    if (months >= 999) {
        m_expiryDate = QDateTime(QDate(2099, 12, 31), QTime(23, 59, 59));
        m_licenseKey = QString("TUNN-LIFE-%1-%2")
                           .arg(m_hwid.left(6))
                           .arg(QRandomGenerator::global()->bounded(1000, 9999));
    } else {
        // If current expiry is in future, extend from that, else extend from now
        QDateTime baseDate = (m_expiryDate.isValid() && m_expiryDate > QDateTime::currentDateTime()) 
                             ? m_expiryDate 
                             : QDateTime::currentDateTime();
        m_expiryDate = baseDate.addMonths(months);
        m_licenseKey = QString("TUNN-PAY-%1M-%2")
                           .arg(months)
                           .arg(QRandomGenerator::global()->bounded(100000, 999999));
    }

    m_isActivated = true;
    saveLicense();

    emit activationSuccess();
    emit licenseUpdated();
    return true;
}

bool LicenseManager::isActivated() const
{
    if (!m_isActivated) return false;
    return !isExpired();
}

bool LicenseManager::isExpired() const
{
    if (!m_expiryDate.isValid()) return true;
    return QDateTime::currentDateTime() > m_expiryDate;
}

int LicenseManager::daysRemaining() const
{
    if (!m_expiryDate.isValid()) return 0;
    if (m_expiryDate.date().year() >= 2090) return 9999; // Lifetime
    qint64 secs = QDateTime::currentDateTime().secsTo(m_expiryDate);
    if (secs <= 0) return 0;
    return static_cast<int>(secs / 86400) + 1;
}

QString LicenseManager::getLicenseKey() const
{
    return m_licenseKey;
}

QString LicenseManager::getPlanName() const
{
    return m_planName.isEmpty() ? "Chưa kích hoạt" : m_planName;
}

QDateTime LicenseManager::getExpiryDate() const
{
    return m_expiryDate;
}

QDateTime LicenseManager::getActivationDate() const
{
    return m_activationDate;
}

QString LicenseManager::getHwid() const
{
    return m_hwid;
}

QString LicenseManager::getStatusBadgeText() const
{
    if (!m_isActivated || isExpired()) {
        return "🔴 HẾT HẠN BẢN QUYỀN";
    }
    if (m_expiryDate.date().year() >= 2090) {
        return "👑 VĨNH VIỄN (LIFETIME)";
    }
    if (m_licenseKey.startsWith("TRIAL-")) {
        return "🟡 BẢN DÙNG THỬ (TRIAL)";
    }
    return "🟢 ĐÃ KÍCH HOẠT (VIP PRO)";
}
