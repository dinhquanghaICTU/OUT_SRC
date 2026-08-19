#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QString>

struct SensorReading
{
    // Cảm biến Độ ẩm đất (Soil Moisture)
    double soilMoisturePct = 55.0;      // 0.0 -> 100.0%
    QString soilStatus = QStringLiteral("OPTIMAL"); // DRY, OPTIMAL, WET

    // Cảm biến DHT11 (Không khí)
    double temperatureC = 27.5;         // Nhiệt độ môi trường °C
    double humidityPct = 65.0;          // Độ ẩm không khí %RH

    // Máy bơm nước tưới & Bồn chứa
    bool pumpActive = false;            // Trạng thái máy bơm ON/OFF
    int pumpRuntimeSeconds = 0;         // Thời gian đã bơm trong phiên này (s)
    double waterTankLevelPct = 85.0;    // Mực nước bồn chứa %
    bool autoIrrigationMode = true;     // Chế độ tự động tưới theo độ ẩm

    // Thống kê
    int totalWateringCountToday = 3;    // Tổng số lần tưới trong ngày
    double totalWaterUsedLiters = 12.5; // Lượng nước ước tính đã dùng (L)
    QDateTime measuredAt;
};

Q_DECLARE_METATYPE(SensorReading)
