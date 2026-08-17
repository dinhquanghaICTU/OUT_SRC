#pragma once

struct DeviceConfig
{
    double minimumPressureHpa = 990.0;
    double maximumPressureHpa = 1030.0;
    double minimumDistanceCm = 20.0;
    int samplingIntervalSeconds = 5;
};
