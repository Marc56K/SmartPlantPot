#include "Scheduler.h"
#include "AppContext.h"

struct PumpState
{
    bool active;
    int numImpulses;
    long nextImpulseTime;
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

    if (!pumpState.active)
    {
        bool validTime = false;
        validTime &= sm.GetIntValue(Setting::SCHEDULE_TIME_HH) == now.hour;
        validTime &= sm.GetIntValue(Setting::SCHEDULE_TIME_MM) == now.minute;

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
            pumpState.nextImpulseTime = now.utcTime;

        }
    }

    if (pumpState.active)
    {
        pumpState.active &= pumpState.numImpulses < sm.GetIntValue(Setting::MAX_PUMP_IMPULSES);
        pumpState.active &= _ctx.GetSensorMgr().GetSoilHumidity() < sm.GetIntValue(Setting::SOIL_HUMIDITY_PERCENT);
    }

    if (pumpState.active && pumpState.nextImpulseTime <= now.utcTime)
    {
        _ctx.GetPowerMgr().StartPumpImpulse();
        pumpState.numImpulses++;
        pumpState.nextImpulseTime = now.utcTime + sm.GetIntValue(Setting::SEEPAGE_DURATION_SEC);
    }
}