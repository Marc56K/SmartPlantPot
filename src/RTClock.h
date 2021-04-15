#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <DS3232RTC.h>
#include "NTPClient.h"
#include "StringFormatHelper.h"

class AppContext;
struct RTDateTime
{
    long utcTime;
    long localTime;

    String GetDate(const bool utc) const;
    String GetWDay(const bool utc) const;
    String GetTime(const bool utc) const;
};

class RTClock
{
public:
    RTClock(AppContext& ctx);
    ~RTClock();

    void Init();
    void Update();

    long GetTimeOffset();

    RTDateTime Now();

private:
    AppContext& _ctx;
    std::string _ntpServer;
    NTPClient _ntpClient;
    bool _syncDone;
};