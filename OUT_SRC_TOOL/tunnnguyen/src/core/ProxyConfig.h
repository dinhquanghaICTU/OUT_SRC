#pragma once

#include <QString>
#include <QStringList>

struct ProxyConfig {
    QString protocol = "http"; // "http", "socks5", "socks4"
    QString host;
    int port = 0;
    QString username;
    QString password;

    bool isEmpty() const {
        return host.trimmed().isEmpty() || port <= 0;
    }

    bool hasAuth() const {
        return !username.trimmed().isEmpty();
    }

    QString toServerUrl() const {
        if (isEmpty()) return "";
        QString proto = protocol.toLower();
        if (proto.isEmpty()) proto = "http";
        return QString("%1://%2:%3").arg(proto, host.trimmed()).arg(port);
    }

    // Returns format: [protocol://][user:pass@]host:port or host:port:user:pass
    QString toFormattedString() const {
        if (isEmpty()) return "";
        QString result;
        if (!protocol.isEmpty() && protocol.toLower() != "http") {
            result += protocol.toLower() + "://";
        }
        if (!username.trimmed().isEmpty() || !password.trimmed().isEmpty()) {
            result += QString("%1:%2@%3:%4").arg(username.trimmed(), password.trimmed(), host.trimmed()).arg(port);
        } else {
            result += QString("%1:%2").arg(host.trimmed()).arg(port);
        }
        return result;
    }

    // Standard string for display in tables: e.g. "103.149.28.12:8080" or "SOCKS5 1.2.3.4:1080 (user)"
    QString toDisplayString() const {
        if (isEmpty()) return "⚡ Direct (Mạng máy)";
        QString result = QString("%1:%2").arg(host.trimmed()).arg(port);
        if (!username.trimmed().isEmpty()) {
            result += QString(" (%1)").arg(username.trimmed());
        }
        if (!protocol.isEmpty() && protocol.toLower() != "http") {
            result = protocol.toUpper() + " " + result;
        }
        return result;
    }

    static ProxyConfig fromString(const QString &raw) {
        ProxyConfig cfg;
        QString s = raw.trimmed();
        if (s.isEmpty()) return cfg;

        // Check protocol prefix
        if (s.startsWith("socks5://", Qt::CaseInsensitive)) {
            cfg.protocol = "socks5";
            s = s.mid(9);
        } else if (s.startsWith("socks4://", Qt::CaseInsensitive)) {
            cfg.protocol = "socks4";
            s = s.mid(9);
        } else if (s.startsWith("http://", Qt::CaseInsensitive)) {
            cfg.protocol = "http";
            s = s.mid(7);
        } else if (s.startsWith("https://", Qt::CaseInsensitive)) {
            cfg.protocol = "http";
            s = s.mid(8);
        }

        // Check if format is user:pass@host:port
        if (s.contains("@")) {
            QStringList atParts = s.split("@");
            QString authPart = atParts[0];
            QString hostPart = atParts[1];
            if (authPart.contains(":")) {
                QStringList auth = authPart.split(":");
                cfg.username = auth[0].trimmed();
                cfg.password = auth[1].trimmed();
            } else {
                cfg.username = authPart.trimmed();
            }
            if (hostPart.contains(":")) {
                QStringList hp = hostPart.split(":");
                cfg.host = hp[0].trimmed();
                cfg.port = hp[1].toInt();
            } else {
                cfg.host = hostPart.trimmed();
            }
            return cfg;
        }

        // Check colon separated formats
        QStringList tokens = s.split(":");
        if (tokens.size() == 2) {
            // host:port
            cfg.host = tokens[0].trimmed();
            cfg.port = tokens[1].toInt();
        } else if (tokens.size() == 4) {
            // host:port:user:pass OR user:pass:host:port
            bool isP1Num = false;
            int p1 = tokens[1].toInt(&isP1Num);
            bool isP3Num = false;
            int p3 = tokens[3].toInt(&isP3Num);

            if (isP1Num && p1 > 0 && p1 <= 65535) {
                // host:port:user:pass
                cfg.host = tokens[0].trimmed();
                cfg.port = p1;
                cfg.username = tokens[2].trimmed();
                cfg.password = tokens[3].trimmed();
            } else if (isP3Num && p3 > 0 && p3 <= 65535) {
                // user:pass:host:port
                cfg.username = tokens[0].trimmed();
                cfg.password = tokens[1].trimmed();
                cfg.host = tokens[2].trimmed();
                cfg.port = p3;
            } else {
                cfg.host = tokens[0].trimmed();
                cfg.port = tokens[1].toInt();
                cfg.username = tokens[2].trimmed();
                cfg.password = tokens[3].trimmed();
            }
        } else if (tokens.size() == 1) {
            cfg.host = tokens[0].trimmed();
        }

        return cfg;
    }
};
