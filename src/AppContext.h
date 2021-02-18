#pragma once
#include "Config.h"
#include "UserInterface.h"
#include "RTClock.h"
#include "SettingsManager.h"
#include "SensorManager.h"
#include "PowerManager.h"
#include "NetworkManager.h"
#include "Scheduler.h"
#include "MqttClient.h"
#include "SerialInterface.h"

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
    PowerManager& GetPowerMgr();
    NetworkManager& GetNetworkMgr();
    Scheduler& GetScheduler();

private:
    UserInterface _userInterface;
    RTClock _clock;
    SettingsManager _settingsMgr;
    SensorManager _sensorMgr;
    PowerManager _powerMgr;
    NetworkManager _networkMgr;
    Scheduler _scheduler;
    MqttClient _mqttClient;
    SerialInterface _serialInterface;
};