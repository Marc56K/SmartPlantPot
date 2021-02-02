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

    // Schedule-page
    auto& sm = _ctx.GetSettingsMgr();
    auto onSelectedChanged = [&]() { sm.SaveToEEPROM(); };
    auto p = std::make_shared<PropertyPage>(onSelectedChanged);
    _navigator.AddPage("Watering", p);
    p->Add(std::make_shared<NumberEditor>(
        "Soil Humidity",
        "%", 0, 1, 1, 99,
        sm.GetIntValue(SOIL_HUMIDITY_PERCENT),
        [&](const double val)
        {
            sm.SetValue(SOIL_HUMIDITY_PERCENT, val);
        }));
    p->Add(std::make_shared<NumberEditor>(
        "Seepage Time",
        "s", 0, 10, 10, 900,
        sm.GetIntValue(SEEPAGE_DURATION_SEC),
        [&](const double val)
        {
            sm.SetValue(SEEPAGE_DURATION_SEC, val);
        }));
    p->Add(std::make_shared<NumberEditor>(
        "Pump Impulse",
        "s", 1, 0.1, 0.0, 5,
        sm.GetFloatValue(PUMP_IMPULSE_SEC),
        [&](const double val)
        {
            sm.SetValue(PUMP_IMPULSE_SEC, val);
        }));
    p->Add(std::make_shared<NumberEditor>(
        "Pump Cycles",
        "x", 0, 1, 0, 5,
        sm.GetIntValue(PUMP_CYCLES),
        [&](const double val)
        {
            sm.SetValue(PUMP_CYCLES, val);
        }));
    p->Add(std::make_shared<TimeEditor>(
        "Pump Time", 
        sm.GetIntValue(SCHEDULE_TIME_HH),
        sm.GetIntValue(SCHEDULE_TIME_MM),
        [&](const uint8_t hh, const uint8_t mm)
        {
            sm.SetValue(SCHEDULE_TIME_HH, hh);
            sm.SetValue(SCHEDULE_TIME_MM, mm);
        }));
    p->Add(std::make_shared<BoolEditor>(
        "Monday",
        sm.GetIntValue(SCHEDULE_DAY_MO) != 0,
        [&](const bool val)
        {
            sm.SetValue(SCHEDULE_DAY_MO, val ? 1 : 0);
        }));
    p->Add(std::make_shared<BoolEditor>(
        "Tuesday",
        sm.GetIntValue(SCHEDULE_DAY_TU) != 0,
        [&](const bool val)
        {
            sm.SetValue(SCHEDULE_DAY_TU, val ? 1 : 0);
        }));
    p->Add(std::make_shared<BoolEditor>(
        "Wednesday",
        sm.GetIntValue(SCHEDULE_DAY_WE) != 0,
        [&](const bool val)
        {
            sm.SetValue(SCHEDULE_DAY_WE, val ? 1 : 0);
        }));
    p->Add(std::make_shared<BoolEditor>(
        "Thursday",
        sm.GetIntValue(SCHEDULE_DAY_TH) != 0,
        [&](const bool val)
        {
            sm.SetValue(SCHEDULE_DAY_TH, val ? 1 : 0);
        }));
    p->Add(std::make_shared<BoolEditor>(
        "Friday",
        sm.GetIntValue(SCHEDULE_DAY_FR) != 0,
        [&](const bool val)
        {
            sm.SetValue(SCHEDULE_DAY_FR, val ? 1 : 0);
        }));
    p->Add(std::make_shared<BoolEditor>(
        "Saturday",
        sm.GetIntValue(SCHEDULE_DAY_SA) != 0,
        [&](const bool val)
        {
            sm.SetValue(SCHEDULE_DAY_SA, val ? 1 : 0);
        }));
    p->Add(std::make_shared<BoolEditor>(
        "Sunday",
        sm.GetIntValue(SCHEDULE_DAY_SU) != 0,
        [&](const bool val)
        {
            sm.SetValue(SCHEDULE_DAY_SU, val ? 1 : 0);
        }));

    // Settings-page
    p = std::make_shared<PropertyPage>(onSelectedChanged);
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
        "Sleep Time",
        "m", 0, 1, 1, 999,
        sm.GetIntValue(SLEEP_DURATION_MINUTES),
        [&](const double val)
        {
            sm.SetValue(SLEEP_DURATION_MINUTES, (int)val);
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
            _ctx.GetPowerMgr().ResetAutoSleepTimer(true);
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