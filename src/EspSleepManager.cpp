#include "EspSleepManager.h"
#include "Arduino.h"
#include "AppContext.h"

#define WAKE_TIME_IN_SECONDS 60
#define uS_TO_S_FACTOR 1000000ULL

EspSleepManager::EspSleepManager(AppContext& ctx) :
    _ctx(ctx),
    _wakeTime(0),
    _clockInterruptEnabled(false),
    _sleepDuration(0),
    _deepSleepRequested(false)
{
    _wakeupCause = esp_sleep_get_wakeup_cause();
    ResetWakeTimer();
}

EspSleepManager::~EspSleepManager()
{
}

void EspSleepManager::PrintWakeupCause()
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

void EspSleepManager::SetClockInterrupt(const bool enabled)
{
    _clockInterruptEnabled = enabled;
}

void EspSleepManager::SetSleepDuration(const int seconds)
{
    _sleepDuration = seconds;
}

void EspSleepManager::ResetWakeTimer()
{
    _wakeTime = millis();
}

int EspSleepManager::GetRemainingWakeTime()
{
    const unsigned long now = millis();
    if (now < _wakeTime)
    {
        return 0;
    }
    const int delta = (now - _wakeTime) / 1000;
    return max<int>(WAKE_TIME_IN_SECONDS - delta, 0);
}

void EspSleepManager::RequestDeepSleep()
{
    _deepSleepRequested = true;
}

bool EspSleepManager::DeepSleepRequested()
{
    return _deepSleepRequested;
}

void EspSleepManager::Update()
{
    if (!DeepSleepRequested())
    {
        if (GetRemainingWakeTime() == 0)
        {
            RequestDeepSleep();
        }
    }
    else
    {
        esp_sleep_enable_ext0_wakeup(ROTENC_SW_PIN, 0);

        if (_clockInterruptEnabled)
        {
            esp_sleep_enable_ext1_wakeup(0x8000, ESP_EXT1_WAKEUP_ALL_LOW); // clock interrupt at pin[15]
        }

        if (_sleepDuration > 0)
        {
            esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * _sleepDuration);
        }

        if (Serial)
        {
            Serial.println("going to sleep");
            Serial.flush();
        }

        esp_deep_sleep_start();
    }
}