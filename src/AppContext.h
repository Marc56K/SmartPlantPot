#pragma once
#include "Config.h"
#include "UserInterface.h"
#include "RTClock.h"
#include "SettingsManager.h"
#include "SensorManager.h"
#include "DeepSleepManager.h"

class AppContext
{
public:
    AppContext();
    ~AppContext();

    void Init();

    UserInterface& GetUserInterface();
    RTClock& GetClock();
    SettingsManager& GetSettingsMgr();
    SensorManager& GetSensorMgr();
    DeepSleepManager& GetDeepSleepMgr();

private:
    UserInterface _userInterface;
    RTClock _clock;
    SettingsManager _settingsMgr;
    SensorManager _sensorMgr;
    DeepSleepManager _deepSleepMgr;
};