#include "AppContext.h"

AppContext::AppContext() : 
    _userInterface(*this)
{
}

AppContext::~AppContext()
{
}

void AppContext::Init()
{
    _settingsMgr.LoadFromEEPROM();
    _userInterface.Init();
}

UserInterface& AppContext::GetUserInterface()
{
    return _userInterface;
}

RTClock& AppContext::GetClock()
{
    return _clock;
}

SettingsManager &AppContext::GetSettingsMgr()
{
    return _settingsMgr;
}

SensorManager &AppContext::GetSensorMgr()
{
    return _sensorMgr;
}