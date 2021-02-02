#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "NTPClient.h"
#include "StringFormatHelper.h"

class AppContext;
struct RTDateTime
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int weekday;

    String ToString(const bool date, const bool time) const;
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
    int _alarmHH;
    int _alarmMM;
};