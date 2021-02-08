#pragma once
#include "Config.h"

class SensorManager
{
public:
    SensorManager();
    ~SensorManager();

    float GetBatVoltage();
    int GetSoilMoisture();
    int GetWaterTankLevel(const bool percent);
    float GetTemperature();

private:
    int GetTransformedSensorValue(
        const int value,
        const int m[][2],
        const int mapSize);
};