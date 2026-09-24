#include "AuthService.h"

#include "api/ApiClient.h"
#include "config/AppConfig.h"

AuthService::AuthService(ApiClient *apiClient, QObject *parent)
    : QObject(parent), m_apiClient(apiClient)
{
    connect(m_apiClient, &ApiClient::loginSucceeded, this, [this](const QString &role) {
        m_currentUser.role = role;
        m_authenticated = true;
        m_offlineMode = false;
        m_pendingUsername.clear();
        m_pendingPassword.clear();
        emit authenticated();
    });
    connect(m_apiClient, &ApiClient::loginFailed, this, [this](const QString &message) {
        const bool offlineAdmin = m_pendingUsername.trimmed() == QStringLiteral("admin1")
            && m_pendingPassword == QStringLiteral("1");
        const bool serverRejectedCredentials =
            message.contains(tr("Tài khoản hoặc mật khẩu không đúng"));
        if (offlineAdmin && !serverRejectedCredentials) {
            m_currentUser.username = QStringLiteral("admin1");
            m_currentUser.role = QStringLiteral("admin");
            m_authenticated = true;
            m_offlineMode = true;
            m_pendingUsername.clear();
            m_pendingPassword.clear();
            emit authenticated();
            return;
        }
        m_pendingUsername.clear();
        m_pendingPassword.clear();
        emit authenticationFailed(message);
    });
}

void AuthService::login(const QString &username, const QString &password)
{
    m_currentUser.username = username;
    m_offlineMode = true;
    m_pendingUsername = username;
    m_pendingPassword = password;

    // Tai khoan tam cho giai doan thiet ke UI. Khi backend san sang, dat
    // DemoMode=false de dang nhap qua API tren Raspberry Pi.
    if (AppConfig::DemoMode) {
        if (username.trimmed() == QStringLiteral("admin") &&
            password == QStringLiteral("1")) {
            m_currentUser.role = QStringLiteral("admin");
            m_authenticated = true;
            m_offlineMode = false;
            emit authenticated();
        } else {
            emit authenticationFailed(
                tr("Tài khoản hoặc mật khẩu không đúng. Dùng admin / 1 để xem bản demo."));
        }
        return;
    }

    m_apiClient->login(username, password);
}

void AuthService::logout()
{
    m_authenticated = false;
    m_offlineMode = false;
    m_pendingUsername.clear();
    m_pendingPassword.clear();
    m_currentUser = {};
    emit loggedOut();
}

bool AuthService::isAuthenticated() const
{
    return m_authenticated;
}

bool AuthService::isAdmin() const
{
    return m_authenticated && m_currentUser.role == QStringLiteral("admin");
}

bool AuthService::isOfflineMode() const
{
    return m_authenticated && m_offlineMode;
}

QString AuthService::currentUsername() const
{
    return m_currentUser.username;
}
