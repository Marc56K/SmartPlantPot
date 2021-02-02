#include "RTClock.h"
#include "AppContext.h"
#include <DS3232RTC.h>

WiFiUDP ntpUDP;

String RTDateTime::ToString(const bool date, const bool time) const
{
    String result;
    if (date)
    {
        result += year;
        result += "-";
        result += StringFormatHelper::ToString(month, 2);
        result += "-";
        result += StringFormatHelper::ToString(day, 2);
        switch(weekday)
        {
            case 1:
                result += " Su";
                break;
            case 2:
                result += " Mo";
                break;
            case 3:
                result += " Tu";
                break;
            case 4:
                result += " We";
                break;
            case 5:
                result += " Th";
                break;
            case 6:
                result += " Fr";
                break;
            case 7:
                result += " Sa";
                break;
        }
    }

    if (date && time)
    {
        result += " ";
    }

    if (time)
    {
        result += StringFormatHelper::ToString(hour, 2);
        result += ":";
        result += StringFormatHelper::ToString(minute, 2);
        result += ":";
        result += StringFormatHelper::ToString(second, 2);
    }
    return result;
}

RTClock::RTClock(AppContext& ctx) : 
    _ctx(ctx),
    _ntpClient(ntpUDP),
    _alarmHH(-1),
    _alarmMM(-1)
{
}

RTClock::~RTClock()
{
}

void RTClock::Init()
{
    _ntpServer = _ctx.GetSettingsMgr().GetStringValue(Setting::TIME_SERVER);
    _ntpClient.setPoolServerName(_ntpServer.data());
    _ntpClient.setTimeOffset(0);
    _ntpClient.setUpdateInterval(300000);
    _ntpClient.begin();
}

void RTClock::Update()
{
    DS3232RTC rtc(true);

    if (_ntpClient.update())
    {   
        rtc.set(_ntpClient.getEpochTime());
        Serial.println("ntp update completed");  
    }

    const int hh = (24 + _ctx.GetSettingsMgr().GetIntValue(Setting::SCHEDULE_TIME_HH) - GetTimeOffset() / 3600) % 24;
    const int mm = _ctx.GetSettingsMgr().GetIntValue(Setting::SCHEDULE_TIME_MM);

    if (_alarmHH != hh || _alarmMM != mm)
    {
        _alarmHH = hh;
        _alarmMM = mm;
        Serial.println(String("set utc-alarm: ") + hh + ":" + mm);
        rtc.setAlarm(ALM1_MATCH_HOURS, 0, mm, hh, 0);
        rtc.alarm(ALARM_1);
        rtc.squareWave(SQWAVE_NONE);
        rtc.alarmInterrupt(ALARM_1, true);
    }
}

long RTClock::GetTimeOffset()
{
    return 3600 * _ctx.GetSettingsMgr().GetIntValue(Setting::TIME_OFFSET_HOURES);
}

RTDateTime RTClock::Now()
{
    DS3232RTC rtc(true);
    time_t t = rtc.get() + GetTimeOffset(); 

    RTDateTime dt = {};

    dt.year = year(t);
    dt.month = month(t);
    dt.day = day(t);
    dt.weekday = weekday(t);
    dt.hour = hour(t);
    dt.minute = minute(t);
    dt.second = second(t);

    return dt;
}