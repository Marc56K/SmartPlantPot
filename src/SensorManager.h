#pragma once
#include "Config.h"

class SensorManager
{
public:
    SensorManager();
    ~SensorManager();

    double GetBatVoltage();
    double GetSoilHumidity();
    double GetWaterTankLevel();
};