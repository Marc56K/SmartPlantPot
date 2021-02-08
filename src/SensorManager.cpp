#include "SensorManager.h"
#include <Arduino.h>
#include <DS3232RTC.h>

SensorManager::SensorManager()
{
    pinMode(SENSOR_VCC_PIN, OUTPUT);
    digitalWrite(SENSOR_VCC_PIN, HIGH);

    pinMode(SOIL_SENSOR_PIN, INPUT);
    pinMode(TANK_SENSOR_PIN, INPUT);

    pinMode(BAT_LEVEL_PIN, INPUT);

    delay(400);
}

SensorManager::~SensorManager()
{
    digitalWrite(SENSOR_VCC_PIN, LOW);
}

float SensorManager::GetBatVoltage()
{
    float val = 0;
    for (int i = 0; i < 10; ++i)
    {
        val += analogRead(BAT_LEVEL_PIN);
    }
    val /= 10;
    return 2.22 * 3.3 * val / 4095;
}

int SensorManager::GetSoilMoisture()
{
    static int sensor2Percent[5][2] = 
    {
         { 2660, 0 },
         { 1780, 30 },
         { 1424, 60 },
         { 1320, 88 },
         { 1270, 100 }
    };
    return GetTransformedSensorValue(analogRead(SOIL_SENSOR_PIN), sensor2Percent, 5);
}

int SensorManager::GetWaterTankLevel(const bool percent)
{
    static int sensor2Percent[8][2] =
    {
        { 3000, 0 },
        { 2980, 18 },
        { 2920, 36 },
        { 2855, 52 },
        { 2540, 69 },
        { 2000, 87 },
        { 1400, 91 },
        { 99, 100 }
    };

    static int sensor2ml[8][2] =
    {
        { 3000, 0 },
        { 2980, 102 },
        { 2920, 207 },
        { 2855, 302 },
        { 2540, 400 },
        { 2000, 500 },
        { 1400, 528 },
        { 99, 580 }
    };

    return GetTransformedSensorValue(analogRead(TANK_SENSOR_PIN), percent ? sensor2Percent : sensor2ml, 8);
}

float SensorManager::GetTemperature()
{
    DS3232RTC rtc(true);
    return 0.25f * rtc.temperature();
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