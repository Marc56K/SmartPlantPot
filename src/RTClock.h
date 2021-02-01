#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "NTPClient.h"
#include "StringFormatHelper.h"

struct RTDateTime
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

    String ToString(const bool date, const bool time) const;
};

class RTClock
{
public:
    RTClock();
    ~RTClock();

    void Update();

    void WakeInOneMinute();
    RTDateTime Now();
    bool SetFromString(const String &inputStr);
    void PrintHelpText() const;

private:
    NTPClient _ntpClient;
};
