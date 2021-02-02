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

    void StartPumpImpulse();
    void StopPumpImpulse();
    bool PumpImpulseRunning();

    void Update();

private:
    AppContext& _ctx;
    unsigned long _wakeTime;
    bool _clockInterruptEnabled;
    int _sleepDuration;
    bool _deepSleepRequested;
    unsigned long _pumpUntil;
    esp_sleep_wakeup_cause_t _wakeupCause;
};