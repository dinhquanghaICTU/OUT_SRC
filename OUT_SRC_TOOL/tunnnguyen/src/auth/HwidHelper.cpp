#include "auth/HwidHelper.h"
#include <QCryptographicHash>
#include <QSysInfo>

QString HwidHelper::getHwid()
{
    QString rawInfo = QSysInfo::machineUniqueId();
    if (rawInfo.isEmpty()) {
        rawInfo = QSysInfo::machineHostName() + "_" + QSysInfo::kernelType();
    }
    QByteArray hash = QCryptographicHash::hash(rawInfo.toUtf8(), QCryptographicHash::Sha256);
    return QString::fromLatin1(hash.toHex()).left(16).toUpper();
}
