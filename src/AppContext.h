#pragma once
#include "RTClock.h"
#include "SettingsManager.h"
#include "SensorManager.h"

class AppContext
{
public:
    AppContext();
    ~AppContext();

    RTClock& Clock();
    SettingsManager& Settings();
    SensorManager& Sensors();

private:
    RTClock _clock;
    SettingsManager _settingsMgr;
    SensorManager _sensorMgr;
};