#include "Scheduler.h"
#include "AppContext.h"
#include "TimeLib.h"

struct PumpState
{
    bool initDone;
    bool active;
    long day;
    int numImpulses;
    long nextImpulseTime; // local time
};

RTC_DATA_ATTR PumpState pumpState = {};

Scheduler::Scheduler(AppContext& ctx) :
    _ctx(ctx)
{
}

Scheduler::~Scheduler()
{
}

void Scheduler::Init()
{
}

void Scheduler::Update()
{
    if (_ctx.GetPowerMgr().WaterPumpIsRunning())
    {
        return;
    }

    auto& sm = _ctx.GetSettingsMgr();
    auto& clk = _ctx.GetClock();
    auto now = clk.Now();

    const long today = elapsedDays(now.localTime);
    const long hh = sm.GetIntValue(Setting::WATERING_TIME_HH);
    const long mm = sm.GetIntValue(Setting::WATERING_TIME_MM);
    const long scheduleTime = previousMidnight(now.localTime) + hh * SECS_PER_HOUR + mm * SECS_PER_MIN;

    if (pumpState.day != today)
    {
        pumpState.day = today;
        if (today % sm.GetIntValue(Setting::WATERING_INTERVAL_DAYS) == 0)
        {
            pumpState.active = true;
            pumpState.nextImpulseTime = scheduleTime;
            pumpState.numImpulses = 0;
        }
    }

    if (pumpState.active)
    {
        pumpState.active &= now.localTime < pumpState.nextImpulseTime + SECS_PER_HOUR;
        pumpState.active &= sm.HasPendingChanges() == false;
        pumpState.active &= sm.GetIntValue(Setting::PUMP_ENABLED) != 0;
        pumpState.active &= pumpState.numImpulses < sm.GetIntValue(Setting::MAX_PUMPING_REPEATS);
        pumpState.active &= _ctx.GetSensorMgr().States().SoilMoistureInPerCent < sm.GetIntValue(Setting::SOIL_MOISTURE_PERCENT);
    }
    
    if (pumpState.active && now.localTime >= pumpState.nextImpulseTime)
    {
        _ctx.GetPowerMgr().RunWaterPump(true);
        pumpState.numImpulses++;
        pumpState.nextImpulseTime = now.localTime + sm.GetIntValue(Setting::SEEPAGE_DURATION_MINUTES) * SECS_PER_MIN;
    }
}

long Scheduler::GetSleepDuration()
{
    auto& sm = _ctx.GetSettingsMgr();
    auto& clk = _ctx.GetClock();
    const auto now = clk.Now();

    long wakeTime = now.localTime + SECS_PER_DAY;
    auto updateWakeTime = [&wakeTime, &now](const long t)
    {
        const long minWakeTime = now.localTime;
        if (t >= minWakeTime && t < wakeTime)
        {
            wakeTime = t;
        }
    };

    const long hh = sm.GetIntValue(Setting::WATERING_TIME_HH);
    const long mm = sm.GetIntValue(Setting::WATERING_TIME_MM);
    const long scheduleTime = hh * SECS_PER_HOUR + mm * SECS_PER_MIN;
    updateWakeTime(previousMidnight(now.localTime) + scheduleTime);
    updateWakeTime(nextMidnight(now.localTime) + scheduleTime);
    
    const long sleepDuration = sm.GetIntValue(Setting::SLEEP_DURATION_MINUTES) * SECS_PER_MIN;
    const long nextWakeTime = now.localTime + sleepDuration;
    updateWakeTime(nextWakeTime);

    updateWakeTime(pumpState.nextImpulseTime);

    return wakeTime - now.localTime;
}