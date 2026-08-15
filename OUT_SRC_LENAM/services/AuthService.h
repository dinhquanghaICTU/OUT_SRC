#pragma once

#include "models/User.h"

#include <QObject>

class ApiClient;

class AuthService : public QObject
{
    Q_OBJECT

public:
    explicit AuthService(ApiClient *apiClient, QObject *parent = nullptr);
    void login(const QString &username, const QString &password);
    void logout();
    bool isAuthenticated() const;
    bool isAdmin() const;
    bool isOfflineMode() const;
    QString currentUsername() const;

signals:
    void authenticated();
    void authenticationFailed(const QString &message);
    void loggedOut();

private:
    ApiClient *m_apiClient;
    User m_currentUser;
    QString m_pendingUsername;
    QString m_pendingPassword;
    bool m_authenticated = false;
    bool m_offlineMode = false;
};
