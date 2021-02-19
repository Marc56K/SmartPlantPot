#pragma once
#include "Config.h"

class SensorManager
{
public:
    SensorManager();
    ~SensorManager();

    void Init();

    float GetBatVoltage();
    int GetSoilMoisture();
    int GetWaterTankLevel();
    float GetTemperature();

private:
    int GetSensorValueMedian(
        const int pin, 
        const int samples);
    int GetTransformedSensorValue(
        const int value,
        const int m[][2],
        const int mapSize);
};