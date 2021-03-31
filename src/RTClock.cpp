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
    _alarmHH(-1),
    _alarmMM(-1)
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

    if (_ctx.GetNetworkMgr().WifiConnected() && _ntpClient.update())
    {   
        rtc.set(_ntpClient.getEpochTime());
        auto now = Now();
        Serial.println(String("ntp update completed: ") + now.GetDate(true) + " - " + now.GetTime(true));  
    }

    int hh = 0;
    int mm = 0;
    _ctx.GetScheduler().GetNextWakupUtcTime(hh, mm);

    if (_alarmHH != hh || _alarmMM != mm)
    {
        _alarmHH = hh;
        _alarmMM = mm;
        Serial.println(String("set utc-alarm: ") + StringFormatHelper::ToString(hh, 2) + ":" + StringFormatHelper::ToString(mm, 2));
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
    time_t t = rtc.get();
    RTDateTime dt = {};
    dt.utcTime = t;

    t += GetTimeOffset();
    dt.localTime = t;

    return dt;
}