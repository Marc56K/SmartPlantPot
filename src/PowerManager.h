#pragma once
#include <esp_sleep.h>

class AppContext;

class PowerManager
{
public:
    PowerManager(AppContext& ctx);
    ~PowerManager();

    void PrintWakeupCause();

    void SetClockInterrupt(const bool enabled);
    void SetSleepDuration(const int seconds);

    void ResetAutoSleepTimer();
    int GetTimeUntilSleep();

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