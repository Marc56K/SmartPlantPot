#include "AppContext.h"

AppContext::AppContext()
{
}

AppContext::~AppContext()
{
}

RTClock& AppContext::Clock()
{
    return _clock;
}

SettingsManager &AppContext::Settings()
{
    return _settingsMgr;
}

SensorManager &AppContext::Sensors()
{
    return _sensorMgr;
}