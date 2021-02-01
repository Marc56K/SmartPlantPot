#include "AppContext.h"

AppContext::AppContext() : 
    _userInterface(*this),
    _clock(*this),
    _powerMgr(*this),
    _networkMgr(*this)
{
}

AppContext::~AppContext()
{
}

void AppContext::Init()
{
    _settingsMgr.LoadFromEEPROM();
    _powerMgr.PrintWakeupCause();
    _userInterface.Init();
    _clock.Init();
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