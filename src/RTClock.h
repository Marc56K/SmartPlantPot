#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "StringFormatHelper.h"

struct RTDateTime
{
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;

  String ToString(const bool date, const bool time) const
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
};

class RTClock
{
public:
  RTClock();
  ~RTClock();

  void WakeInOneMinute();
  RTDateTime Now();
  bool SetFromString(const String& inputStr);
  void PrintHelpText() const;
};
