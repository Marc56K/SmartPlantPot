#pragma once
#include "Config.h"

struct SensorStates
{
    bool IsValid;
    float BatVoltage;
    float Temperature;
    int SoilMoistureRaw;
    int SoilMoistureInPerCent;
    int WaterTankLevelRaw;
    int WaterTankLevelInPerCent;

    SensorStates() :
        IsValid(false)
    {            
    }
};

class SensorManager
{
public:
    SensorManager();
    ~SensorManager();

    void Init();
    void Update();

    const SensorStates& States() const;

private:
    int GetSensorValueMedian(
        const int pin, 
        const int samples);
    int GetTransformedSensorValue(
        const int value,
        const int m[][2],
        const int mapSize);

private:
    unsigned long _created;
    SensorStates _sensorStates;
};