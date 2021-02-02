#pragma once
#include <esp_sleep.h>

class AppContext;

class PowerManager
{
public:
    PowerManager(AppContext& ctx);
    ~PowerManager();

    void Init();

    void PrintWakeupCause();

    void ResetAutoSleepTimer(const bool causedByUserInput);
    int GetTimeUntilSleep();

    void RequestDeepSleep();
    bool DeepSleepRequested();

    void StartPumpImpulse();
    void StopPumpImpulse();
    bool PumpImpulseRunning();

    void Update();

private:
    AppContext& _ctx;
    unsigned long _sleepTime;
    bool _deepSleepRequested;
    unsigned long _pumpUntil;
    esp_sleep_wakeup_cause_t _wakeupCause;
};