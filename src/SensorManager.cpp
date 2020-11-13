#include "SensorManager.h"
#include <Arduino.h>

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

double SensorManager::GetBatVoltage()
{
    double val = analogRead(BAT_LEVEL_PIN);
    return 2.22 * 3.3 * val / 4095;
}

double SensorManager::GetSoilHumidity()
{
    double value = analogRead(SENSOR_0_VALUE_PIN);
    return std::max(3000.0 - value, 0.0) / 3000.0;
}

double SensorManager::GetWaterTankLevel()
{
    double value = analogRead(SENSOR_1_VALUE_PIN);
    return std::max(3000.0 - value, 0.0) / 3000.0;
}