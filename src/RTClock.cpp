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
    _ctx(ctx), _ntpClient(ntpUDP)//, "pool.ntp.org")
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
    if (_ntpClient.update())
    {
        Serial.println("NTP update");
        DS3232RTC rtc(true);
        time_t t = _ntpClient.getEpochTime();
        rtc.set(t);
    }
}

long RTClock::GetTimeOffset()
{
    return 3600 * _ctx.GetSettingsMgr().GetIntValue(Setting::TIME_OFFSET);
}

void RTClock::WakeInOneMinute()
{
    DS3232RTC rtc(true);
    time_t t = rtc.get();
    rtc.setAlarm(ALM1_MATCH_MINUTES, 0, minute(t) + 1, 0, 0);
    rtc.alarm(ALARM_1);
    rtc.squareWave(SQWAVE_NONE);
    rtc.alarmInterrupt(ALARM_1, true);
}

RTDateTime RTClock::Now()
{
    DS3232RTC rtc(true);
    time_t t = rtc.get() + GetTimeOffset(); 

    RTDateTime dt = {};

    dt.year = year(t);
    dt.month = month(t);
    dt.day = day(t);
    dt.hour = hour(t);
    dt.minute = minute(t);
    dt.second = second(t);

    return dt;
}