#include "AppContext.h"

AppContext::AppContext() : 
    _userInterface(*this),
    _powerMgr(*this),
    _networkMgr(*this)
{
}

AppContext::~AppContext()
{
}

void AppContext::Init()
{
    _powerMgr.PrintWakeupCause();
    _settingsMgr.LoadFromEEPROM();
    _userInterface.Init();
}

void AppContext::Update()
{
    _clock.Update();
    _networkMgr.Update();
    _powerMgr.Update();
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

PowerManager& AppContext::GetPowerMgr()
{
    return _powerMgr;
}

NetworkManager& AppContext::GetNetworkMgr()
{
    return _networkMgr;
}