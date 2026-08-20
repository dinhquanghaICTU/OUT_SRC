#pragma once

#include <QString>

namespace AppConfig {

inline constexpr bool DemoMode = false;
inline constexpr int RefreshIntervalMs = 800;

inline const QString DefaultApiHost = QStringLiteral("127.0.0.1");
inline constexpr quint16 DefaultApiPort = 8080;

inline const QString DefaultMqttHost = QStringLiteral("127.0.0.1");
inline constexpr quint16 DefaultMqttPort = 1883;

inline const QString TargetDeviceId = QStringLiteral("Vanphong-190782");

} // namespace AppConfig
