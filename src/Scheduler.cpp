#include "Scheduler.h"
#include "AppContext.h"

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
    if (_ctx.GetPowerMgr().PumpImpulseRunning())
    {
        return;
    }

    auto& sm = _ctx.GetSettingsMgr();
    auto now = _ctx.GetClock().Now();

    if (!pumpState.active && pumpState.lastImpulseTime + 61 < now.utcTime)
    {
        const bool validHour = sm.GetIntValue(Setting::SCHEDULE_TIME_HH) == now.hour;
        const bool validMinute = sm.GetIntValue(Setting::SCHEDULE_TIME_MM) == now.minute;
        const bool validTime = validHour && validMinute;

        bool validDay = false;
        switch(now.weekday)
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
    auto now = clk.Now();

    utcHour = (24 + sm.GetIntValue(Setting::SCHEDULE_TIME_HH) - clk.GetTimeOffset() / 3600) % 24;
    utcMinute = sm.GetIntValue(Setting::SCHEDULE_TIME_MM);
    auto seepage = sm.GetIntValue(Setting::SEEPAGE_DURATION_MINUTES) * 60;
    if (pumpState.active)
    {
        long nextImpulse = pumpState.lastImpulseTime + seepage;
        nextImpulse = std::max(now.utcTime + 60, nextImpulse);

        int utcHourImpule = hour(nextImpulse);
        int utcMinuteImpulse = minute(nextImpulse);
        if (utcHourImpule < utcHourImpule || (utcHourImpule == utcHourImpule && utcMinuteImpulse < utcMinute))
        {
            utcHour = utcHourImpule;
            utcMinute = utcMinuteImpulse;
        }
    }

    if (hour(now.utcTime) == utcHour && minute(now.utcTime) == utcMinute)
    {
        auto t = now.utcTime + 60;
        utcHour = hour(t);
        utcMinute = minute(t);
    }
}