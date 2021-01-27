#include "UserInterface.h"
#include "HomePage.h"
#include "InfoPage.h"
#include <images.h>

extern sIMAGE IMG_tank_0;
extern sIMAGE IMG_tank_25;
extern sIMAGE IMG_tank_50;
extern sIMAGE IMG_tank_75;
extern sIMAGE IMG_tank_100;

UserInterface::UserInterface(AppContext& ctx) :
    _ctx(ctx), 
    _paint(_frameBuffer.data(), EPD_WIDTH, EPD_HEIGHT),
    _firstFrame(true)
{
}

UserInterface::~UserInterface()
{
    
}

bool UserInterface::Init()
{
    _inputMgr.Init();

    _firstFrame = true;

    if (_display.Init(lut_partial_update) != 0)
    {
        Serial.println("e-Paper init failed");
        return false;
    }

    _navigator.AddPage("", std::make_shared<HomePage>(_ctx));  
    _ctx.GetSettingsMgr().CreatePropertyPages(_navigator);
    _navigator.AddPage("Info", std::make_shared<InfoPage>(_ctx));

    return true;
}

void UserInterface::HandleInput()
{
    if (!_ctx.GetEspSleepMgr().DeepSleepRequested())
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
            _ctx.GetEspSleepMgr().ResetWakeTimer();
        }
    }
}

void UserInterface::Update()
{
    HandleInput();
    RenderFrame();
    RefreshDisplay();
}

void UserInterface::RenderFrame()
{
    RenderPages();
    RenderStatusBar();
}

void UserInterface::RefreshDisplay()
{
    if (_firstFrame)
    {
        _firstFrame = false;
        for (auto& p : _frameBuffer)
        {
            p = ~p;
        }
    }

    _display.SetFrameMemory(_paint.GetImage(), 0, 0, _paint.GetWidth(), _paint.GetHeight());
    _display.DisplayFrame(true);
}

void UserInterface::RenderPages()
{
    if (_ctx.GetEspSleepMgr().DeepSleepRequested())
    {
        // switch to home page
        _navigator.SetCurrentPage(0);
    }

    _navigator.SetWdith(EPD_WIDTH);
    _navigator.SetHeight(267);
    _navigator.Render(_paint, 0, 0);
}

void UserInterface::RenderStatusBar()
{
    _paint.DrawFilledRectangle(0, 267, EPD_WIDTH, EPD_HEIGHT, WHITE);
    _paint.DrawHorizontalLine(0, 267, EPD_WIDTH, 0);
    RenderBatteryIndicator(4, 272, _ctx.GetSensorMgr().GetBatVoltage());
    RenderTankIndicator(100, 272, _ctx.GetSensorMgr().GetWaterTankLevel());
}

void UserInterface::RenderTankIndicator(const uint32_t x, const uint32_t y, const float v)
{
    sIMAGE* img = &IMG_tank_100;
    if (v < 0.2f)
        img = &IMG_tank_0;
    else if (v < 0.4f)
        img = &IMG_tank_25;
    else if (v < 0.53f)
        img = &IMG_tank_50;
    else if (v < 0.56f)
        img = &IMG_tank_75;

    _paint.DrawImage(x, y, img);
}

void UserInterface::RenderBatteryIndicator(const uint32_t x, const uint32_t y, const float v)
{
    int level = -240.0673368f * v * v * v + 2658.611075f * v * v - 9650.778487f * v + 11521.76802f;
    level = max(0, min(level, 100));

    sIMAGE* img = &IMG_bat_100;
    if (level <= 10)
        img = &IMG_bat_0;
    else if (level <= 25)
        img = &IMG_bat_25;
    else if (level <= 50)
        img = &IMG_bat_50;
    else if (level <= 75)
        img = &IMG_bat_75;

    _paint.DrawImage(x, y, img);
}

void UserInterface::RenderOnlineIndicator(const uint32_t x, const uint32_t y, const bool online)
{
    if (online)
    {
        _paint.DrawImage(x, y, &IMG_online);
    }
    else
    {
        _paint.DrawImage(x, y, &IMG_offline);
    }    
}

void UserInterface::RenderBusyAnimation(const uint32_t x, const uint32_t y)
{
    int i = ((millis() / 750) % 6);
    switch(i)
    {
        case 0: 
            _paint.DrawImage(x, y, &IMG_spinner_0);
            break;
        case 1: 
            _paint.DrawImage(x, y, &IMG_spinner_1);
            break;
        case 2: 
            _paint.DrawImage(x, y, &IMG_spinner_2);
            break;
        case 3: 
            _paint.DrawImage(x, y, &IMG_spinner_3);
            break;
        case 4: 
            _paint.DrawImage(x, y, &IMG_spinner_4);
            break;
        default:
            _paint.DrawImage(x, y, &IMG_spinner_5);
    }
}