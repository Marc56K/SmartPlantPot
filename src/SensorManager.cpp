#include "SensorManager.h"
#include "AppContext.h"
#include <DS3232RTC.h>
#include <algorithm>

#define INIT_DURATION_MILLIS 500

SensorManager::SensorManager(AppContext& ctx) :
    _ctx(ctx),
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
    if (_ctx.GetPowerMgr().WaterPumpIsRunning())
    {
        // pump is interfering with sensor readings
        return;
    }

    xSemaphoreTakeRecursive(_mutex, portMAX_DELAY);
    {
        if (_ctx.GetPowerMgr().WaterPumpWasRunning(1))
        {
            _analogPinValues.clear();
        }

        if (_analogPinValues.empty())
        {
            ReadAnalogPins();
        }
        _sensorStates.BatRaw = lroundf(_analogPinValues[BAT_LEVEL_PIN]);
        _sensorStates.SoilMoistureRaw = lroundf(_analogPinValues[SOIL_SENSOR_PIN]);
        _sensorStates.WaterTankLevelRaw = lroundf(_analogPinValues[TANK_SENSOR_PIN]);
    }
    xSemaphoreGiveRecursive(_mutex);

    auto& s = _ctx.GetSettingsMgr();

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
        static const float lut[][2] = 
        {
            { 1.0000f, 0 },
            { 0.5844f, 14 },
            { 0.3144f, 29 },
            { 0.1766f, 43 },
            { 0.1030f, 57 },
            { 0.0551f, 71 },
            { 0.0240f, 86 },
            { 0.0000f, 100 }
        };
        _sensorStates.SoilMoistureInPerCent = GetMappedSensorValue(
            _sensorStates.SoilMoistureRaw,
            s.GetIntValue(Setting::SOIL_SENSOR_MIN_VALUE),
            s.GetIntValue(Setting::SOIL_SENSOR_MAX_VALUE),
            lut, sizeof(lut) / sizeof(lut[0]));
    }

    // WaterTankLevel
    {
        static const float lut[][2] =        
        {
            { 1.0000f, 0 },
            { 0.9931f, 17 },
            { 0.9814f, 34 },
            { 0.9622f, 52 },
            { 0.9219f, 69 },
            { 0.8725f, 78 },
            { 0.7514f, 87 },
            { 0.4872f, 95 },
            { 0.0000f, 100 }
        };
        _sensorStates.WaterTankLevelInPerCent = GetMappedSensorValue(
            _sensorStates.WaterTankLevelRaw, 
            s.GetIntValue(Setting::TANK_SENSOR_MIN_VALUE),
            s.GetIntValue(Setting::TANK_SENSOR_MAX_VALUE),
            lut, sizeof(lut) / sizeof(lut[0]));
    }

    _sensorStates.IsValid = true;
}

const SensorStates& SensorManager::States() const
{
    return _sensorStates;
}

float SensorManager::GetMappedSensorValue(
        const float value,
        const float minValue,
        const float maxValue,
        const float lut[][2],
        const uint8_t mapSize) const
{
    const float val = (value - minValue) / (maxValue - minValue);

    if (val > lut[0][0])
        return lut[0][1]; // 0%

    if (val < lut[mapSize - 1][0])
        return lut[mapSize - 1][1]; // 100%

    auto mapf = [](float x, float in_min, float in_max, float out_min, float out_max) -> float
    {
        const float divisor = (in_max - in_min);
        return (x - in_min) * (out_max - out_min) / divisor + out_min;
    };

    for (uint8_t i = 0; i < mapSize - 1; ++i)
    {
        if (val < lut[i][0] && val >= lut[i + 1][0])
        {
            return mapf(
                val, 
                lut[i][0], lut[i + 1][0],
                lut[i][1], lut[i + 1][1]);
        }
    }
    return 0;
}