#include "Scheduler.h"
#include "AppContext.h"
#include "TimeLib.h"

struct PumpState
{
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
    if (_ctx.GetPowerMgr().GetMillisSinceLastPumpImpulse() == 0)
    {
        return;
    }

    auto& sm = _ctx.GetSettingsMgr();
    auto& clk = _ctx.GetClock();
    auto now = clk.Now();

    const long today = elapsedDays(now.localTime);
    const long hh = sm.GetIntValue(Setting::SCHEDULE_TIME_HH);
    const long mm = sm.GetIntValue(Setting::SCHEDULE_TIME_MM);
    const long scheduleTime = previousMidnight(now.localTime) + hh * SECS_PER_HOUR + mm * SECS_PER_MIN;

    if (sm.HasPendingChanges())
    {
        Serial.println("reset");
        pumpState.day = 0;
        return;
    }

    if (pumpState.day != today)
    {
        pumpState.day = today;
        bool validDay = false;
        switch(weekday(now.localTime + 30))
        {
            case dowSunday:
                validDay = sm.GetIntValue(Setting::SCHEDULE_DAY_SU) != 0;
                break;
            case dowMonday:
                validDay = sm.GetIntValue(Setting::SCHEDULE_DAY_MO) != 0;
                break;
            case dowTuesday:
                validDay = sm.GetIntValue(Setting::SCHEDULE_DAY_TU) != 0;
                break;
            case dowWednesday:
                validDay = sm.GetIntValue(Setting::SCHEDULE_DAY_WE) != 0;
                break;
            case dowThursday:
                validDay = sm.GetIntValue(Setting::SCHEDULE_DAY_TH) != 0;
                break;
            case dowFriday:
                validDay = sm.GetIntValue(Setting::SCHEDULE_DAY_FR) != 0;
                break;
            case dowSaturday:
                validDay = sm.GetIntValue(Setting::SCHEDULE_DAY_SA) != 0;
                break;
            case dowInvalid:
            default:
                break;
        }
        if (validDay)
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
        pumpState.active &= pumpState.numImpulses < sm.GetIntValue(Setting::MAX_PUMP_IMPULSES);
        pumpState.active &= _ctx.GetSensorMgr().GetSoilMoisture() < sm.GetIntValue(Setting::SOIL_MOISTURE_PERCENT);
    }
    
    if (pumpState.active && now.localTime >= pumpState.nextImpulseTime)
    {
        _ctx.GetPowerMgr().StartPumpImpulse();
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

    const long hh = sm.GetIntValue(Setting::SCHEDULE_TIME_HH);
    const long mm = sm.GetIntValue(Setting::SCHEDULE_TIME_MM);
    const long scheduleTime = hh * SECS_PER_HOUR + mm * SECS_PER_MIN;
    updateWakeTime(previousMidnight(now.localTime) + scheduleTime);
    updateWakeTime(nextMidnight(now.localTime) + scheduleTime);
    
    const long sleepDuration = sm.GetIntValue(Setting::SLEEP_DURATION_MINUTES) * SECS_PER_MIN;
    const long nextWakeTime = now.localTime + sleepDuration;
    updateWakeTime(nextWakeTime);

    updateWakeTime(pumpState.nextImpulseTime);

    return wakeTime - now.localTime;
}