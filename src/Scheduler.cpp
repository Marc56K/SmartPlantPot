#include "Scheduler.h"
#include "AppContext.h"
#include "TimeLib.h"

struct PumpState
{
    bool active;
    int numImpulses;
    long lastImpulseTime;
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
    auto now = _ctx.GetClock().Now();

    if (!pumpState.active && pumpState.lastImpulseTime + 61 < now.utcTime)
    {
        const bool validHour = sm.GetIntValue(Setting::SCHEDULE_TIME_HH) == hour(now.localTime);
        const bool validMinute = sm.GetIntValue(Setting::SCHEDULE_TIME_MM) == minute(now.localTime);
        const bool validTime = validHour && validMinute;

        bool validDay = false;
        switch(weekday(now.localTime))
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

        if (validTime && validDay)
        {
            pumpState.active = true;
            pumpState.numImpulses = 0;
            pumpState.lastImpulseTime = 0;

        }
    }

    if (pumpState.active)
    {
        pumpState.active &= sm.HasPendingChanges() == false;
        pumpState.active &= sm.GetIntValue(Setting::PUMP_ENABLED) != 0;
        pumpState.active &= pumpState.numImpulses < sm.GetIntValue(Setting::MAX_PUMP_IMPULSES);
        pumpState.active &= _ctx.GetSensorMgr().GetSoilMoisture() < sm.GetIntValue(Setting::SOIL_MOISTURE_PERCENT);
    }

    auto seepage = sm.GetIntValue(Setting::SEEPAGE_DURATION_MINUTES) * 60;
    if (pumpState.active && (pumpState.numImpulses == 0 || pumpState.lastImpulseTime + seepage <= now.utcTime))
    {
        _ctx.GetPowerMgr().StartPumpImpulse();
        pumpState.numImpulses++;
        pumpState.lastImpulseTime = now.utcTime;
    }
}

void Scheduler::GetNextWakupUtcTime(int& utcHour, int& utcMinute)
{
    auto& sm = _ctx.GetSettingsMgr();
    auto& clk = _ctx.GetClock();
    const auto now = clk.Now();

    long wakeTime = now.utcTime + SECS_PER_DAY;
    auto updateWakeTime = [&wakeTime, &now](const long t)
    {
        const long minWakeTime = now.utcTime + SECS_PER_MIN;
        if (t >= minWakeTime && t < wakeTime)
        {
            wakeTime = t;
        }
    };

    const long hh = sm.GetIntValue(Setting::SCHEDULE_TIME_HH);
    const long mm = sm.GetIntValue(Setting::SCHEDULE_TIME_MM);
    const long scheduleTime = hh * SECS_PER_HOUR + mm * SECS_PER_MIN - clk.GetTimeOffset();
    updateWakeTime(previousMidnight(now.utcTime) + scheduleTime);
    updateWakeTime(nextMidnight(now.utcTime) + scheduleTime);

    if (pumpState.active)
    {
        const long seepageDuration = sm.GetIntValue(Setting::SEEPAGE_DURATION_MINUTES) * SECS_PER_MIN;
        const long nextImpulseTime = pumpState.lastImpulseTime + seepageDuration;
        updateWakeTime(nextImpulseTime);
    }
    
    utcHour = hour(wakeTime);
    utcMinute = minute(wakeTime);
}