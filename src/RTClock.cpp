#include "RTClock.h"
#include <DS3232RTC.h>

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

RTClock::RTClock()
{
}

RTClock::~RTClock()
{
}

void RTClock::WakeInOneMinute()
{
  delay(10);
  DS3232RTC RTC(true);
  time_t t = RTC.get();
  RTC.setAlarm(ALM1_MATCH_MINUTES, 0, minute(t) + 1, 0, 0);
  RTC.alarm(ALARM_1);
  RTC.squareWave(SQWAVE_NONE);
  RTC.alarmInterrupt(ALARM_1, true);
}

RTDateTime RTClock::Now()
{
  DS3232RTC RTC(true);
  time_t t = RTC.get();

  RTDateTime dt = {};

  dt.year = year(t);
  dt.month = month(t);
  dt.day = day(t);
  dt.hour = hour(t);
  dt.minute = minute(t);
  dt.second = second(t);

  return dt;
}

bool RTClock::SetFromString(const String &inputStr)
{
  return false;
}

void RTClock::PrintHelpText() const
{
  Serial.println(F("Format YYMMDDwhhmmssx"));
  Serial.println(F("Where YY = Year (ex. 20 for 2020)"));
  Serial.println(F("      MM = Month (ex. 04 for April)"));
  Serial.println(F("      DD = Day of month (ex. 09 for 9th)"));
  Serial.println(F("      w  = Day of week from 1 to 7, 1 = Sunday (ex. 5 for Thursday)"));
  Serial.println(F("      hh = hours in 24h format (ex. 09 for 9AM or 21 for 9PM)"));
  Serial.println(F("      mm = minutes (ex. 02)"));
  Serial.println(F("      ss = seconds (ex. 42)"));
  Serial.println(F("Example for input : 2004095090242x"));
  Serial.println(F("-----------------------------------------------------------------------------"));
  Serial.println(F("Please enter the current time to set on DS3231 ended by 'x':"));
}