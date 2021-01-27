#pragma once
#include "Config.h"
#include "UserInterface.h"
#include "RTClock.h"
#include "SettingsManager.h"
#include "SensorManager.h"
#include "EspSleepManager.h"

class AppContext
{
public:
    AppContext();
    ~AppContext();

    void Init();
    void Update();

    UserInterface& GetUserInterface();
    RTClock& GetClock();
    SettingsManager& GetSettingsMgr();
    SensorManager& GetSensorMgr();
    EspSleepManager& GetEspSleepMgr();

private:
    UserInterface _userInterface;
    RTClock _clock;
    SettingsManager _settingsMgr;
    SensorManager _sensorMgr;
    EspSleepManager _espSleepMgr;
};