#include "SensorManager.h"
#include <Arduino.h>
#include <DS3232RTC.h>
#include <algorithm>

#define INIT_DURATION_MILLIS 500

SensorManager::SensorManager() :
    _created(millis()),    
    _mutex(nullptr),
    _shutdownRequested(nullptr),
    _shutdownCompleted(nullptr),
    _task(nullptr)
{
    pinMode(SENSOR_VCC_PIN, OUTPUT);
    digitalWrite(SENSOR_VCC_PIN, HIGH);

    pinMode(SOIL_SENSOR_PIN, INPUT);
    pinMode(TANK_SENSOR_PIN, INPUT);

    pinMode(BAT_LEVEL_PIN, INPUT);
}

SensorManager::~SensorManager()
{
    if (_task != nullptr)
    {
        xEventGroupSetBits(_shutdownRequested, 1);
        xEventGroupWaitBits(_shutdownCompleted, 1, pdFALSE, pdTRUE, portMAX_DELAY);
    }

    vEventGroupDelete(_shutdownRequested);
    vEventGroupDelete(_shutdownCompleted);

    vTaskDelete(_task);

    vSemaphoreDelete(_mutex);

    digitalWrite(SENSOR_VCC_PIN, LOW);
}

void SensorManager::Init()
{
    _sensorStates.IsValid = false;

    _mutex = xSemaphoreCreateRecursiveMutex();
    _shutdownRequested = xEventGroupCreate();
    _shutdownCompleted = xEventGroupCreate();

    xTaskCreatePinnedToCore(
      [](void* p)
      {
          auto* sm = static_cast<SensorManager*>(p);
          while(true)
          {
            sm->ReadAnalogPins();

            if (xEventGroupWaitBits(sm->_shutdownRequested, 1, pdFALSE, pdTRUE, 10) == 1)
            {
                break;
            }
          }
      },
      "SensorReadTask", /* Name of the task */
      10000,  /* Stack size in words */
      this,  /* Task input parameter */
      1,  /* Priority of the task */
      &_task,  /* Task handle. */
      0); /* Core where the task should run */

    const auto now = millis();
    const auto delta = now - _created;
    const auto waitTime = delta > INIT_DURATION_MILLIS ? 0 : INIT_DURATION_MILLIS - delta;

    if (waitTime > 0)
    {
        // moisture sensor need some time to deliver valid values
        vTaskDelay(waitTime / portTICK_PERIOD_MS);
    }
}

void SensorManager::ReadAnalogPins()
{
    xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
    {
        bool init = _analogPinValues.empty();
        auto readAnalogPin = [&](const int pin)
        {
            if (init)
            {
                _analogPinValues[pin] = analogRead(pin);
            }
            _analogPinValues[pin] = (_analogPinValues[pin] * 99 + analogRead(pin) * 1) / 100;
        };

        readAnalogPin(BAT_LEVEL_PIN);
        readAnalogPin(SOIL_SENSOR_PIN);
        readAnalogPin(TANK_SENSOR_PIN);        
    }
    xSemaphoreGiveRecursive(_mutex);
}

void SensorManager::Update()
{
    xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
    {
        if (_analogPinValues.empty())
        {
            ReadAnalogPins();
        }
        _sensorStates.BatRaw = _analogPinValues[BAT_LEVEL_PIN];
        _sensorStates.SoilMoistureRaw = _analogPinValues[SOIL_SENSOR_PIN];
        _sensorStates.WaterTankLevelRaw = _analogPinValues[TANK_SENSOR_PIN];
    }
    xSemaphoreGiveRecursive(_mutex);

    // BatVoltage
    {
        _sensorStates.BatVoltage = 2.22f * 3.3f * _sensorStates.BatRaw / 4095;
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
        _sensorStates.SoilMoistureInPerCent = GetTransformedSensorValue(_sensorStates.SoilMoistureRaw, sensor2Percent, 2);
    }

    // WaterTankLevel
    {
        static const int sensor2Percent[8][2] =
        {
            { 2867,	0 },
            { 2853,	20 },
            { 2818,	41 },
            { 2740,	61 },
            { 2654,	71 },
            { 2352,	85 },
            { 1710,	92 },
            { 138,	100 }
        };
        _sensorStates.WaterTankLevelInPerCent = GetTransformedSensorValue(_sensorStates.WaterTankLevelRaw, sensor2Percent, 8);
    }

    _sensorStates.IsValid = true;
}

const SensorStates& SensorManager::States() const
{
    return _sensorStates;
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