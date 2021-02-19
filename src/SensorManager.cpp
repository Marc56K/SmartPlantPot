#include "SensorManager.h"
#include <Arduino.h>
#include <DS3232RTC.h>
#include <algorithm>

#define INIT_DURATION_MILLIS 500

SensorManager::SensorManager() :
    _mutex(nullptr),
    _shutdownRequested(nullptr),
    _shutdownCompleted(nullptr),
    _task(nullptr)
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
    // wait for sensors to startup
    delay(100);

    // read each input once to prevent garbage on first access
    analogRead(SOIL_SENSOR_PIN);
    analogRead(TANK_SENSOR_PIN);
    analogRead(BAT_LEVEL_PIN);

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
}

void SensorManager::ReadAnalogPins()
{
    xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
    {
        bool init = _analogPinValues.empty();
        auto readAnalogPin = [&](const int pin, const int samples)
        {
            if (init)
            {
                _analogPinValues[pin] = GetSensorValueMedian(pin, 101);
            }
            const float newValue = GetSensorValueMedian(pin, samples);
            _analogPinValues[pin] = _analogPinValues[pin] * 0.99f + newValue * 0.01f;
        };

        readAnalogPin(BAT_LEVEL_PIN, 11);
        readAnalogPin(SOIL_SENSOR_PIN, 21);
        readAnalogPin(TANK_SENSOR_PIN, 51);        
    }
    xSemaphoreGiveRecursive(_mutex);
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

void SensorManager::Update()
{
    xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
    {
        if (_analogPinValues.empty())
        {
            ReadAnalogPins();
        }
        _sensorStates.BatRaw = lroundf(_analogPinValues[BAT_LEVEL_PIN]);
        _sensorStates.SoilMoistureRaw = lroundf(_analogPinValues[SOIL_SENSOR_PIN]);
        _sensorStates.WaterTankLevelRaw = lroundf(_analogPinValues[TANK_SENSOR_PIN]);
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
        static const int sensor2Percent[][2] = 
        {
            { 2960,	0 },
            { 2266,	14 },
            { 1815,	29 },
            { 1585,	43 },
            { 1462,	57 },
            { 1382,	71 },
            { 1330,	86 },
            { 1290,	100 }
        };
        _sensorStates.SoilMoistureInPerCent = GetTransformedSensorValue(
            _sensorStates.SoilMoistureRaw, 
            sensor2Percent, 
            sizeof(sensor2Percent) / sizeof(sensor2Percent[0]));
    }

    // WaterTankLevel
    {
        static const int sensor2Percent[][2] =
        
        {
            { 3725,	0 },
            { 3700,	17 },
            { 3658,	34 },
            { 3589,	52 },
            { 3444,	69 },
            { 3266,	78 },
            { 2830,	87 },
            { 1879,	95 },
            { 125, 100 }
        };
        _sensorStates.WaterTankLevelInPerCent = GetTransformedSensorValue(
            _sensorStates.WaterTankLevelRaw, 
            sensor2Percent, 
            sizeof(sensor2Percent) / sizeof(sensor2Percent[0]));
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