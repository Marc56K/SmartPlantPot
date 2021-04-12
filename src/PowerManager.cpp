#include "PowerManager.h"
#include "Arduino.h"
#include "AppContext.h"
#include <esp_wifi.h>
#include <esp_adc_cal.h>
#include <analogWrite.h>

#define uS_TO_S_FACTOR 1000000ULL
#define WAKE_TIME_AFTER_ALERT 10

PowerManager::PowerManager(AppContext& ctx) :
    _ctx(ctx),
    _sleepTime(std::numeric_limits<uint32_t>::max()),
    _deepSleepRequested(false),
    _pumpPower(25),
    _pumpDuration(0),
    _pumpUntil(0)
{
    _wakeupCause = esp_sleep_get_wakeup_cause();
}

PowerManager::~PowerManager()
{
}

void PowerManager::Init()
{
    pinMode(PUMP_VCC_PIN, OUTPUT);
    digitalWrite(PUMP_VCC_PIN, LOW);

    ResetAutoSleepTimer(_wakeupCause == ESP_SLEEP_WAKEUP_EXT0);

    PrintWakeupCause();
}

void PowerManager::PrintWakeupCause()
{
    switch (_wakeupCause)
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", _wakeupCause);
        break;
    }
}

void PowerManager::ResetAutoSleepTimer(const bool causedByUserInput)
{
    if (causedByUserInput)
    {
        _sleepTime = millis() + 1000 * _ctx.GetSettingsMgr().GetIntValue(Setting::WAKE_DURATION_SEC);
    }
    else // timer or alert
    {
        _sleepTime = millis() + 1000 * WAKE_TIME_AFTER_ALERT;
    }
}

int PowerManager::GetSecondsUntilSleep()
{
    const int now = millis();
    return max<int>(_sleepTime - now, 0) / 1000;
}

void PowerManager::RequestDeepSleep()
{
    _deepSleepRequested = true;
}

bool PowerManager::DeepSleepRequested()
{
    return _deepSleepRequested;
}

void PowerManager::RunWaterPump(const bool wait)
{
    Serial.println("run pump");
    _pumpPower = _ctx.GetSettingsMgr().GetIntValue(Setting::PUMPING_POWER_PERCENT);
    _pumpDuration = 1000 * _ctx.GetSettingsMgr().GetFloatValue(Setting::PUMPING_DURATION_SEC);

    if (wait)
    {
        PumpProc();
        _pumpUntil = millis();
    }
    else
    {
        xTaskCreatePinnedToCore(
        [](void* p)
        {
            static_cast<PowerManager*>(p)->PumpProc();
            vTaskDelete(nullptr);
        },
        "PumpRunner", /* Name of the task */
        10000,  /* Stack size in words */
        this,  /* Task input parameter */
        1,  /* Priority of the task */
        nullptr,  /* Task handle. */
        0); /* Core where the task should run */

        _pumpUntil = millis() + _pumpDuration;
    }    
}

void PowerManager::PumpProc()
{
    analogWrite(PUMP_VCC_PIN, std::min(_pumpPower, 100u), 100u);
    vTaskDelay(_pumpDuration / portTICK_PERIOD_MS);
    analogWrite(PUMP_VCC_PIN, 0u, 100u);
}

bool PowerManager::WaterPumpIsRunning()
{
    return WaterPumpWasRunning(0);
}

bool PowerManager::WaterPumpWasRunning(const float seconds)
{
    const uint32_t now = millis();
    if (now < _pumpUntil)
    {
        return true;
    }
    return (now - _pumpUntil) < seconds * 1000;
}

void PowerManager::Update()
{
    if (WaterPumpIsRunning())
    {
        return;
    }

    if (!DeepSleepRequested())
    {
        if (GetSecondsUntilSleep() == 0)
        {
            RequestDeepSleep();
        }
    }
    else
    {
        // wake on button press
        esp_sleep_enable_ext0_wakeup(ROTENC_SW_PIN, 1);

        // wake on timer alert
        esp_sleep_enable_ext1_wakeup(0x8000, ESP_EXT1_WAKEUP_ALL_LOW); // clock interrupt at pin[15]

        int h, m;
        long sleepDuration = _ctx.GetScheduler().GetNextWakupUtcTime(h, m);
        esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * sleepDuration);

        if (Serial)
        {
            Serial.println(String("going to sleep for ") + sleepDuration + " seconds");
            Serial.flush();
        }

        WiFi.mode(WIFI_OFF);
        esp_wifi_stop();
        adc_power_off();

        esp_deep_sleep_start();
    }
}