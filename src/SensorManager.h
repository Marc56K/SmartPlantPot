#pragma once
#include <Arduino.h>
#include <map>
#include "Config.h"

struct SensorStates
{
    bool IsValid;
    float Temperature;
    int BatRaw;
    float BatVoltage;
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
    void ReadAnalogPins();
    int GetSensorValueMedian(const int pin, 
        const int samples);

    int GetTransformedSensorValue(
        const int value,
        const int m[][2],
        const int mapSize);

private:
    SemaphoreHandle_t _mutex;
    EventGroupHandle_t _shutdownRequested;
    EventGroupHandle_t _shutdownCompleted;
    TaskHandle_t _task;
    SensorStates _sensorStates;
    std::map<int, float> _analogPinValues;
};