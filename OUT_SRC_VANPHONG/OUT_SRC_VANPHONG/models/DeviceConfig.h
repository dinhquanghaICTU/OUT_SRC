#pragma once

#include <QString>

struct DeviceConfig
{
    // Ngưỡng độ ẩm đất điều khiển bơm
    double minSoilMoistureTriggerPct = 40.0; // Dưới mức này tự động bật bơm tưới (%)
    double maxSoilMoistureStopPct = 75.0;    // Đạt mức này tự động ngắt bơm (%)
    int maxPumpRuntimeMinutes = 5;           // Thời gian tối đa cho 1 lần tưới (phút)
    int pumpFlowRateLitersPerMin = 3;        // Lưu lượng máy bơm (L/phút)

    // Cảnh báo môi trường
    double maxTemperatureAlertC = 38.0;      // Cảnh báo quá nhiệt nhà màng (°C)
    double minWaterTankLevelPct = 15.0;      // Cảnh báo cạn nước bồn chứa (%)
    bool autoWateringEnabled = true;         // Bật tính năng tưới tự động
    int samplingIntervalSeconds = 1;         // Chu kỳ telemetry (s)
};
