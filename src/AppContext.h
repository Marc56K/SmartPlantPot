#pragma once
#include "RTClock.h"
#include "InputManager.h"
#include "SettingsManager.h"
#include "SensorManager.h"

class AppContext
{
public:
    AppContext();
    ~AppContext();

    void Init();

    RTClock& Clock();
    InputManager& Input();
    SettingsManager& Settings();
    SensorManager& Sensors();

private:
    RTClock _clock;
    InputManager _inputMgr;
    SettingsManager _settingsMgr;
    SensorManager _sensorMgr;
};