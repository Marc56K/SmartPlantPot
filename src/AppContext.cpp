#include "AppContext.h"

AppContext::AppContext()
{
}

AppContext::~AppContext()
{
}

void AppContext::Init()
{
    _settingsMgr.LoadFromEEPROM();
    _inputMgr.Init();
}

RTClock& AppContext::Clock()
{
    return _clock;
}

InputManager& AppContext::Input()
{
    return _inputMgr;
}

SettingsManager &AppContext::Settings()
{
    return _settingsMgr;
}

SensorManager &AppContext::Sensors()
{
    return _sensorMgr;
}