#include "UserInterface.h"
#include "HomePage.h"
#include "InfoPage.h"
#include "ActionButton.h"

UserInterface::UserInterface(AppContext& ctx) :
    _ctx(ctx)
{
}

UserInterface::~UserInterface()
{    
}

void UserInterface::Init()
{
    _inputMgr.Init();
    _display.Init();

    // home-page
    _navigator.AddPage("", std::make_shared<HomePage>(_ctx));

    // actions-page
    auto p = std::make_shared<PropertyPage>();
    _navigator.AddPage("Actions", p);
    p->Add(std::make_shared<ActionButton>(
        "Give some water",
        [&]()
        {
            _ctx.GetPowerMgr().RunWaterPump();
        }));
    p->Add(std::make_shared<ActionButton>(
        "Got to sleep",
        [&]()
        {
            _navigator.SetCurrentPage(0);
            _ctx.GetPowerMgr().RequestDeepSleep();
        }));

    // watering-page
    auto& sm = _ctx.GetSettingsMgr();
    auto onEditingFinished = [&]() { sm.SaveToEEPROM(); };
    p = std::make_shared<PropertyPage>(onEditingFinished);
    _navigator.AddPage("Watering", p);
    p->Add(std::make_shared<BoolEditor>(
        "Enabled",
        sm.GetIntValue(PUMP_ENABLED) != 0,
        [&](const bool val)
        {
            sm.SetValue(PUMP_ENABLED, val ? 1 : 0);
        }));
    p->Add(std::make_shared<NumberEditor>(
        "Interval",
        "d", 0, 1, 1, 99,
        sm.GetIntValue(WATERING_INTERVAL_DAYS),
        [&](const double val)
        {
            sm.SetValue(WATERING_INTERVAL_DAYS, val);
        }));
    p->Add(std::make_shared<TimeEditor>(
        "Time", 
        sm.GetIntValue(WATERING_TIME_HH),
        sm.GetIntValue(WATERING_TIME_MM),
        [&](const uint8_t hh, const uint8_t mm)
        {
            sm.SetValue(WATERING_TIME_HH, hh);
            sm.SetValue(WATERING_TIME_MM, mm);
        }));
    p->Add(std::make_shared<NumberEditor>(
        "Pumping",
        "s", 1, 0.1, 0.0, 5,
        sm.GetFloatValue(PUMPING_DURATION_SEC),
        [&](const double val)
        {
            sm.SetValue(PUMPING_DURATION_SEC, val);
        }));
    p->Add(std::make_shared<NumberEditor>(
        "Repeats",
        "x", 0, 1, 1, 99,
        sm.GetIntValue(MAX_PUMPING_REPEATS),
        [&](const double val)
        {
            sm.SetValue(MAX_PUMPING_REPEATS, val);
        }));
    p->Add(std::make_shared<NumberEditor>(
        "Seepage Time",
        "m", 0, 1, 1, 60,
        sm.GetIntValue(SEEPAGE_DURATION_MINUTES),
        [&](const double val)
        {
            sm.SetValue(SEEPAGE_DURATION_MINUTES, val);
        }));

    // settings-page
    p = std::make_shared<PropertyPage>(onEditingFinished);
    _navigator.AddPage("Settings", p); 
    p->Add(std::make_shared<StringEditor>(
        "WiFi SSID", 
        sm.GetStringValue(WIFI_SSID), 
        [&](const std::string& val) 
        { 
            sm.SetValue(WIFI_SSID, val);
        }));
    p->Add(std::make_shared<StringEditor>(
        "WiFi Key",
        sm.GetStringValue(WIFI_KEY),
        [&](const std::string& val)
        {
            sm.SetValue(WIFI_KEY, val);
        }));
    p->Add(std::make_shared<StringEditor>(
        "NTP Server",
        sm.GetStringValue(TIME_SERVER),
        [&](const std::string& val)
        {
            sm.SetValue(TIME_SERVER, val);
        }));
    p->Add(std::make_shared<NumberEditor>(
        "Time Offset",
        "h", 0, 1, -23, 23,
        sm.GetIntValue(TIME_OFFSET_HOURES),
        [&](const double val)
        {
            sm.SetValue(TIME_OFFSET_HOURES, (int)val);
        }));
    p->Add(std::make_shared<NumberEditor>(
        "Wake Time",
        "s", 0, 1, 10, 999,
        sm.GetIntValue(WAKE_DURATION_SEC),
        [&](const double val)
        {
            sm.SetValue(WAKE_DURATION_SEC, (int)val);
        }));
    p->Add(std::make_shared<NumberEditor>(
        "Sleep Time",
        "m", 0, 1, 1, 999,
        sm.GetIntValue(SLEEP_DURATION_MINUTES),
        [&](const double val)
        {
            sm.SetValue(SLEEP_DURATION_MINUTES, (int)val);
        }));

    // MQTT-page
    p = std::make_shared<PropertyPage>(onEditingFinished);
    _navigator.AddPage("MQTT", p); 
    p->Add(std::make_shared<BoolEditor>(
        "Enabled",
        sm.GetIntValue(MQTT_ENABLED) != 0,
        [&](const bool val)
        {
            sm.SetValue(MQTT_ENABLED, val ? 1 : 0);
        }));
    p->Add(std::make_shared<StringEditor>(
        "Server", 
        sm.GetStringValue(MQTT_SERVER), 
        [&](const std::string& val) 
        { 
            sm.SetValue(MQTT_SERVER, val);
        }));
    p->Add(std::make_shared<NumberEditor>(
        "Port",
        "", 0, 1, 0, 65535,
        sm.GetIntValue(MQTT_PORT),
        [&](const double val)
        {
            sm.SetValue(MQTT_PORT, (int)val);
        }));
    p->Add(std::make_shared<StringEditor>(
        "User", 
        sm.GetStringValue(MQTT_USER), 
        [&](const std::string& val) 
        { 
            sm.SetValue(MQTT_USER, val);
        }));
    p->Add(std::make_shared<StringEditor>(
        "Key", 
        sm.GetStringValue(MQTT_KEY), 
        [&](const std::string& val) 
        { 
            sm.SetValue(MQTT_KEY, val);
        }));
    p->Add(std::make_shared<StringEditor>(
        "Topic", 
        sm.GetStringValue(MQTT_TOPIC), 
        [&](const std::string& val) 
        { 
            sm.SetValue(MQTT_TOPIC, val);
        }));

    // info-page
    _navigator.AddPage("Info", std::make_shared<InfoPage>(_ctx));
}

void UserInterface::Update()
{
    auto& pm = _ctx.GetPowerMgr();
    auto& sm = _ctx.GetSensorMgr();
    auto& nm = _ctx.GetNetworkMgr();

    // handle input

    if (!pm.DeepSleepRequested())
    {
        auto btnPressed = _inputMgr.ButtonPressed();
        auto encoderDelta = _inputMgr.GetRotaryEncoderDelta();

        if (pm.GetMillisSinceLastPumping() > 2000)
        {
            if (btnPressed)
            {
                _navigator.Click();
            }

            if (encoderDelta != 0)
            {
                _navigator.Scroll(encoderDelta);
            }

            if (btnPressed || encoderDelta != 0)
            {
                pm.ResetAutoSleepTimer(true);
            }
        }
    }
    else
    {
        _navigator.SetCurrentPage(0);
    }

    // update display
    
    if (pm.DeepSleepRequested() || pm.GetMillisSinceLastPumping() > 0)
    {
        _display.RenderNavigator(_navigator);
        _display.RenderStatusBar(
            sm.States().BatVoltage,
            sm.States().WaterTankLevelInPerCent,
            pm.DeepSleepRequested() == false,
            nm.WifiConnected());
        _display.Present();
    }

    if (pm.DeepSleepRequested())
    {
        _display.Sleep();
    }
}