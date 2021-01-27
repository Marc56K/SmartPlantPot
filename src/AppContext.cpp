#include "AppContext.h"

AppContext::AppContext() : 
    _userInterface(*this),
    _espSleepMgr(*this)
{
}

AppContext::~AppContext()
{
}

void AppContext::Init()
{
    _espSleepMgr.PrintWakeupCause();
    _settingsMgr.LoadFromEEPROM();
    _userInterface.Init();
}

void AppContext::Update()
{
    _espSleepMgr.Update();
    _userInterface.Update();
}

UserInterface& AppContext::GetUserInterface()
{
    return _userInterface;
}

RTClock& AppContext::GetClock()
{
    return _clock;
}

SettingsManager& AppContext::GetSettingsMgr()
{
    return _settingsMgr;
}

SensorManager& AppContext::GetSensorMgr()
{
    return _sensorMgr;
}

EspSleepManager& AppContext::GetEspSleepMgr()
{
    return _espSleepMgr;
}