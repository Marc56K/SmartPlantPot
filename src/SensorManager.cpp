#include "SensorManager.h"
#include <Arduino.h>
#include <DS3232RTC.h>

SensorManager::SensorManager()
{
    pinMode(SENSOR_VCC_PIN, OUTPUT);
    digitalWrite(SENSOR_VCC_PIN, HIGH);

    pinMode(SENSOR_0_VALUE_PIN, INPUT);
    pinMode(SENSOR_1_VALUE_PIN, INPUT);

    pinMode(BAT_LEVEL_PIN, INPUT);

    delay(400);
}

SensorManager::~SensorManager()
{
    digitalWrite(SENSOR_VCC_PIN, LOW);
}

float SensorManager::GetBatVoltage()
{
    float val = analogRead(BAT_LEVEL_PIN);
    return 2.22 * 3.3 * val / 4095;
}

int SensorManager::GetSoilHumidity()
{
    float value = analogRead(SENSOR_0_VALUE_PIN);
    return std::max(3000.0f - value, 0.0f) / 30;
}

int SensorManager::GetWaterTankLevel()
{
    float value = analogRead(SENSOR_1_VALUE_PIN);
    return std::max(3000.0f - value, 0.0f) / 30;
}

float SensorManager::GetTemperature()
{
    DS3232RTC rtc(true);
    return 0.25f * rtc.temperature();
}