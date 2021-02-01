#include "UserInterface.h"
#include "HomePage.h"
#include "InfoPage.h"

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

    // WiFi-page
    auto& sm = _ctx.GetSettingsMgr();
    auto onSelectedChanged = [&]() { sm.SaveToEEPROM(); };
    auto p = std::make_shared<PropertyPage>(onSelectedChanged);
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
        sm.GetIntValue(TIME_OFFSET),
        [&](const double val)
        {
            sm.SetValue(TIME_OFFSET, (int)val);
        }));

    // Pumping-page
    p = std::make_shared<PropertyPage>(onSelectedChanged);
    _navigator.AddPage("Pumping", p);
    p->Add(std::make_shared<TimeEditor>(
        "Time", 
        sm.GetIntValue(PUMP_TIME_HH),
        sm.GetIntValue(PUMP_TIME_MM),
        [&](const uint8_t hh, const uint8_t mm)
        {
            sm.SetValue(PUMP_TIME_HH, hh);
            sm.SetValue(PUMP_TIME_MM, mm);
        }));

    p->Add(std::make_shared<NumberEditor>(
        "Duration",
        "s", 1, 0.1, 0.0, 5,
        sm.GetFloatValue(PUMP_DURATION),
        [&](const double val)
        {
            sm.SetValue(PUMP_DURATION, val);
        }));

    // info-page
    _navigator.AddPage("Info", std::make_shared<InfoPage>(_ctx));
}

void UserInterface::Update()
{
    // handle input

    if (!_ctx.GetPowerMgr().DeepSleepRequested())
    {
        auto btnPressed = _inputMgr.ButtonPressed();
        auto encoderDelta = _inputMgr.GetRotaryEncoderDelta();

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
            _ctx.GetPowerMgr().ResetAutoSleepTimer();
        }
    }
    else
    {
        _navigator.SetCurrentPage(0);
    }

    // update display

    _display.RenderNavigator(_navigator);
    _display.RenderStatusBar(
        _ctx.GetSensorMgr().GetBatVoltage(),
        _ctx.GetSensorMgr().GetWaterTankLevel(),
        _ctx.GetNetworkMgr().WifiConnected());
    _display.Present();
}