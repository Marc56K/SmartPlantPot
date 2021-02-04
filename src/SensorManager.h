#pragma once
#include "Config.h"

class SensorManager
{
public:
    SensorManager();
    ~SensorManager();

    float GetBatVoltage();
    int GetSoilMoisture();
    int GetWaterTankLevel();
    float GetTemperature();
};