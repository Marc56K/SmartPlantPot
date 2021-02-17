#include "SensorManager.h"
#include <Arduino.h>
#include <DS3232RTC.h>
#include <algorithm>

#define INIT_DURATION_MILLIS 500

SensorManager::SensorManager() :
    _created(millis())
{
    pinMode(SENSOR_VCC_PIN, OUTPUT);
    digitalWrite(SENSOR_VCC_PIN, HIGH);

    pinMode(SOIL_SENSOR_PIN, INPUT);
    pinMode(TANK_SENSOR_PIN, INPUT);

    pinMode(BAT_LEVEL_PIN, INPUT);
}

SensorManager::~SensorManager()
{
    digitalWrite(SENSOR_VCC_PIN, LOW);
}

void SensorManager::Init()
{
    _sensorStates.IsValid = false;

    const auto now = millis();
    const auto delta = now - _created;
    const auto waitTime = delta > INIT_DURATION_MILLIS ? 0 : INIT_DURATION_MILLIS - delta;

    if (waitTime > 0)
    {
        // moisture sensor need some time to deliver valid values
        vTaskDelay(waitTime / portTICK_PERIOD_MS);
    }

    Update();
}

void SensorManager::Update()
{
    // BatVoltage
    {
        const float val = GetSensorValueMedian(BAT_LEVEL_PIN, 31);
        _sensorStates.BatVoltage = 2.22 * 3.3 * val / 4095;
    }

    // Temperature
    {
        DS3232RTC rtc(true);
        _sensorStates.Temperature = 0.25f * rtc.temperature();
    }

    // SoilMoisture
    {
        static const int sensor2Percent[2][2] = 
        {
            { 2660, 0 },
            { 1270, 100 }
        };
        _sensorStates.SoilMoistureRaw = GetSensorValueMedian(SOIL_SENSOR_PIN, 11);
        _sensorStates.SoilMoistureInPerCent = GetTransformedSensorValue(_sensorStates.SoilMoistureRaw, sensor2Percent, 2);
    }

    // WaterTankLevel
    {
        static const int sensor2Percent[10][2] =
        {
            { 3030,	0 },
            { 3019,	19 },
            { 2970,	38 },
            { 2868,	57 },
            { 2680,	76 },
            { 2425,	86 },
            { 2138,	90 },
            { 1520,	95 },
            { 950,	97 },
            { 112,	100 }
        };
        _sensorStates.WaterTankLevelRaw = GetSensorValueMedian(TANK_SENSOR_PIN, 31);
        _sensorStates.WaterTankLevelInPerCent = GetTransformedSensorValue(_sensorStates.WaterTankLevelRaw, sensor2Percent, 10);
    }

    _sensorStates.IsValid = true;
}

const SensorStates& SensorManager::States() const
{
    return _sensorStates;
}

int SensorManager::GetSensorValueMedian(
        const int pin, 
        const int samples)
{
    const int s = (samples > 0 && samples % 2 == 1) ? samples : samples + 1;
    std::vector<int> values(s);
    for (int i = 0; i < values.size(); ++i)
    {
        values[i] = analogRead(pin);
    }
    std::sort(values.begin(), values.end());
    return values[values.size() / 2];
}

int SensorManager::GetTransformedSensorValue(
        const int value,
        const int m[][2],
        const int mapSize)
{
    if (value > m[0][0])
        return m[0][1]; // 0%

    if (value < m[mapSize - 1][0])
        return m[mapSize - 1][1]; // 100%

    for (int i = 0; i < mapSize - 1; ++i)
    {
        if (value < m[i][0] && value >= m[i + 1][0])
        {
            return map(
                value, 
                m[i][0], m[i + 1][0],
                m[i][1], m[i + 1][1]);
        }
    }
    return 0;
}