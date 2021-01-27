#pragma once
#include <esp_deep_sleep.h>

class AppContext;

class DeepSleepManager
{
public:
    DeepSleepManager(AppContext& ctx);
    ~DeepSleepManager();

    void PrintWakeupCause();

    void SetClockInterrupt(const bool enabled);
    void SetSleepDuration(const int seconds);

    void ResetWakeDuration();
    int GetRemainingWakeDuration();

    void Update();

private:
    AppContext& _ctx;
    unsigned long _wakeTime;
    bool _clockInterruptEnabled;
    int _sleepDuration;
    esp_sleep_wakeup_cause_t _wakeupCause;
};