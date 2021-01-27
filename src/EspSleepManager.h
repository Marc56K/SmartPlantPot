#pragma once
#include <esp_sleep.h>

class AppContext;

class EspSleepManager
{
public:
    EspSleepManager(AppContext& ctx);
    ~EspSleepManager();

    void PrintWakeupCause();

    void SetClockInterrupt(const bool enabled);
    void SetSleepDuration(const int seconds);

    void ResetWakeTimer();
    int GetRemainingWakeTime();

    void RequestDeepSleep();
    bool DeepSleepRequested();

    void Update();

private:
    AppContext& _ctx;
    unsigned long _wakeTime;
    bool _clockInterruptEnabled;
    int _sleepDuration;
    bool _deepSleepRequested;
    esp_sleep_wakeup_cause_t _wakeupCause;
};