#include "PowerManager.h"
#include "Arduino.h"
#include "AppContext.h"

#define uS_TO_S_FACTOR 1000000ULL
#define WAKE_TIME_AFTER_USER_INPUT 60 // sec
#define WAKE_TIME_AFTER_ALERT 10

PowerManager::PowerManager(AppContext& ctx) :
    _ctx(ctx),
    _sleepTime(-1),
    _deepSleepRequested(false),
    _pumpUntil(0)
{
    _wakeupCause = esp_sleep_get_wakeup_cause();
    PrintWakeupCause();
}

PowerManager::~PowerManager()
{
}

void PowerManager::Init()
{
    pinMode(PUMP_VCC_PIN, OUTPUT);
    digitalWrite(PUMP_VCC_PIN, LOW);

    ResetAutoSleepTimer(_wakeupCause == ESP_SLEEP_WAKEUP_EXT0);
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
        _sleepTime = millis() + 1000 * WAKE_TIME_AFTER_USER_INPUT;
    }
    else // timer or alert
    {
        _sleepTime = millis() + 1000 * WAKE_TIME_AFTER_ALERT;
    }
}

int PowerManager::GetTimeUntilSleep()
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

void PowerManager::StartPumpImpulse()
{
    Serial.println("run pump");
    const long pumpDuration = 1000 * _ctx.GetSettingsMgr().GetFloatValue(Setting::PUMP_IMPULSE_SEC);
    _pumpUntil = millis() + pumpDuration;
    
    digitalWrite(PUMP_VCC_PIN, HIGH);
    vTaskDelay(pumpDuration / portTICK_PERIOD_MS);
    digitalWrite(PUMP_VCC_PIN, LOW);
    delay(50);
}

unsigned long PowerManager::GetMillisSinceLastPumpImpulse()
{
    auto now = millis();
    if (now < _pumpUntil)
    {
        return 0;
    }
    return now - _pumpUntil;
}

void PowerManager::Update()
{
    if (millis() < _pumpUntil)
    {
        return;
    }

    if (!DeepSleepRequested())
    {
        if (GetTimeUntilSleep() == 0)
        {
            RequestDeepSleep();
        }
    }
    else
    {
        // wake on button press
        esp_sleep_enable_ext0_wakeup(ROTENC_SW_PIN, 0);

        // wake on timer alert
        //esp_sleep_enable_ext1_wakeup(0x8000, ESP_EXT1_WAKEUP_ALL_LOW); // clock interrupt at pin[15]

        const long sleepDuration = _ctx.GetScheduler().GetSleepDuration();
        esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * sleepDuration);

        if (Serial)
        {
            Serial.println(String("going to sleep for ") + sleepDuration + " seconds");
            Serial.flush();
        }

        esp_deep_sleep_start();
    }
}