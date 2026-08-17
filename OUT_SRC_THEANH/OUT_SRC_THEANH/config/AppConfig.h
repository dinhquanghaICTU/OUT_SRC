#pragma once

#include <QString>

namespace AppConfig {
inline const QString ApiBaseUrl = QStringLiteral("http://127.0.0.1:8080");
inline constexpr int RefreshIntervalMs = 1500;
inline constexpr bool DemoMode = false;
}
