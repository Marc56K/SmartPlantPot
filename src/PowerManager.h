#pragma once
#include <esp_sleep.h>
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
    int GetSecondsUntilSleep();

    void RequestDeepSleep();
    bool DeepSleepRequested();

    void RunWaterPump();
    bool WaterPumpIsRunning();
    uint32_t GetMillisSinceLastPumping();

    void Update();

private:
    void PumpProc();

private:
    AppContext& _ctx;
    uint32_t _sleepTime;
    bool _deepSleepRequested;
    uint32_t _pumpPower;
    uint32_t _pumpDuration;
    uint32_t _pumpUntil;
    esp_sleep_wakeup_cause_t _wakeupCause;
};