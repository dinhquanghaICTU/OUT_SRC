#pragma once

#include <QString>

struct DeviceConfig
{
    // Cài đặt động cơ bước
    int motorSpeedStepsPerSec = 800;   // Tốc độ bước/giây (100 - 3200)
    int maxTravelSteps = 3200;         // Tổng số bước mở hết hành trình (100%)
    int microsteppingMode = 8;         // 1 (Full), 2 (Half), 4, 8, 16
    int accelerationSteps = 400;       // Bước gia tốc mượt mà

    // Cài đặt cảm biến & thời gian
    int autoCloseDelaySeconds = 5;     // Thời gian giữ mở trước khi tự động đóng (s)
    int sr602HoldTimeSeconds = 3;      // Thời gian duy trì tín hiệu chuyển động PIR (s)
    int irSafetyDebounceMs = 50;       // Độ nhạy chống kẹt IR (ms)
    bool autoPinchReverse = true;      // Tự động đảo chiều mở lại khi gặp vật cản IR
    bool soundBuzzerOnMove = true;     // Còi bíp cảnh báo khi cửa di chuyển
    int samplingIntervalSeconds = 1;   // Chu kỳ gửi telemetry (s)
};
