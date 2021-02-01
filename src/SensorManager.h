#pragma once
#include "Config.h"

class SensorManager
{
public:
    SensorManager();
    ~SensorManager();

    float GetBatVoltage();
    int GetSoilHumidity();
    int GetWaterTankLevel();
    float GetTemperature();
};