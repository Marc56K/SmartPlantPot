#include "RTClock.h"
#include "AppContext.h"
#include "StringFormatHelper.h"

WiFiUDP ntpUDP;

String RTDateTime::GetDate(const bool utc) const
{
    auto t = utc ? utcTime : localTime;
    String result;
    result += year(t);
    result += "-";
    result += StringFormatHelper::ToString(month(t), 2);
    result += "-";
    result += StringFormatHelper::ToString(day(t), 2);
    return result;
}

String RTDateTime::GetWDay(const bool utc) const
{
    auto t = utc ? utcTime : localTime;
    switch(weekday(t))
    {
        case dowSunday:
            return "Sunday";
        case dowMonday:
            return "Monday";
        case dowTuesday:
            return "Tuesday";
        case dowWednesday:
            return "Wednesday";
        case dowThursday:
            return "Thursday";
        case dowFriday:
            return "Friday";
        case dowSaturday:
            return "Saturday";
        case dowInvalid:
        default:
            return "";
    }
}

String RTDateTime::GetTime(const bool utc) const
{
    auto t = utc ? utcTime : localTime;
    String result;
    result += StringFormatHelper::ToString(hour(t), 2);
    result += ":";
    result += StringFormatHelper::ToString(minute(t), 2);
    result += ":";
    result += StringFormatHelper::ToString(second(t), 2);
    return result;
}

RTClock::RTClock(AppContext& ctx) : 
    _ctx(ctx),
    _ntpClient(ntpUDP),
    _syncDone(false)
{
}

RTClock::~RTClock()
{
}

void RTClock::Init()
{
    auto now = Now();
    Serial.println(String("RTC: ") + now.GetDate(true) + " - " + now.GetTime(true));  

    _ntpServer = _ctx.GetSettingsMgr().GetStringValue(Setting::TIME_SERVER);
    _ntpClient.setPoolServerName(_ntpServer.data());
    _ntpClient.setTimeOffset(0);
    _ntpClient.setUpdateInterval(300000);
    _ntpClient.begin();
}

void RTClock::Update()
{
    DS3232RTC rtc(true);

    if (!_syncDone && _ctx.GetNetworkMgr().WifiConnected() && _ntpClient.update())
    {   
        rtc.set(_ntpClient.getEpochTime());
        rtc.alarmInterrupt(ALARM_1, false);
        auto now = Now();
        Serial.println(String("ntp update completed: ") + now.GetDate(true) + " - " + now.GetTime(true));
        _syncDone = true;
    }
}

long RTClock::GetTimeOffset()
{
    return 3600 * _ctx.GetSettingsMgr().GetIntValue(Setting::TIME_OFFSET_HOURES);
}

RTDateTime RTClock::Now()
{
    DS3232RTC rtc(true);
    time_t t = rtc.get();
    RTDateTime dt = {};
    dt.utcTime = t;

    t += GetTimeOffset();
    dt.localTime = t;

    return dt;
}