#include "SensorManager.h"
#include <Arduino.h>
#include <DS3232RTC.h>
#include <algorithm>

SensorManager::SensorManager()
{
    pinMode(SENSOR_VCC_PIN, OUTPUT);
    gpio_set_drive_capability((gpio_num_t)SENSOR_VCC_PIN, GPIO_DRIVE_CAP_3);
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
    // wait for sensors to startup
    delay(100);

    // read each input once to reduce invalid values
    analogRead(SOIL_SENSOR_PIN);
    analogRead(TANK_SENSOR_PIN);
    analogRead(BAT_LEVEL_PIN);
}

float SensorManager::GetBatVoltage()
{
    const float val = GetSensorValueMedian(BAT_LEVEL_PIN, 31);
    return 2.22 * 3.3 * val / 4095;
}

int SensorManager::GetSoilMoisture()
{
    static const int sensor2Percent[8][2] = 
    {
        { 2960,	0 },
        { 2266,	14 },
        { 1815,	29 },
        { 1585,	43 },
        { 1462,	57 },
        { 1382,	71 },
        { 1330,	86 },
        { 1270,	100 }
    };

    return GetTransformedSensorValue(GetSensorValueMedian(SOIL_SENSOR_PIN, 11), sensor2Percent, 8);;
}

int SensorManager::GetWaterTankLevel()
{
    static const int m[10][2] =
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

    return GetTransformedSensorValue(GetSensorValueMedian(TANK_SENSOR_PIN, 31), m, 10);
}

float SensorManager::GetTemperature()
{
    DS3232RTC rtc(true);
    return 0.25f * rtc.temperature();
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