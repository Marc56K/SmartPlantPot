#include "AppContext.h"

AppContext::AppContext() : 
    _userInterface(*this),
    _clock(*this),
    _powerMgr(*this),
    _networkMgr(*this),
    _scheduler(*this),
    _mqttClient(*this)
{
}

AppContext::~AppContext()
{
}

void AppContext::Init()
{
    _settingsMgr.Init();
    _sensorMgr.Init();
    _powerMgr.Init();
    _networkMgr.Init();
    _clock.Init();
    _scheduler.Init();
    _mqttClient.Init();
    _userInterface.Init();
}

void AppContext::Update()
{
    _networkMgr.Update();
    _clock.Update();
    _sensorMgr.Update();
    _scheduler.Update();
    _powerMgr.Update();
    _mqttClient.Update();
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

Scheduler& AppContext::GetScheduler()
{
    return _scheduler;
}