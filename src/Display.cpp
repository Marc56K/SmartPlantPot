#include "Display.h"
#include <images.h>

extern sIMAGE IMG_tank_0;
extern sIMAGE IMG_tank_25;
extern sIMAGE IMG_tank_50;
extern sIMAGE IMG_tank_75;
extern sIMAGE IMG_tank_100;

Display::Display() :
    _paint(_frameBuffer.data(), EPD_WIDTH, EPD_HEIGHT)
{
}

Display::~Display()
{
}

void Display::Init()
{
    if (_display.Init(lut_full_update) == 0)
    {
        _display.ClearFrameMemory(0xFF);
        _display.DisplayFrame();
    }

    if (_display.Init(lut_partial_update) != 0)
    {
        Serial.println("e-Paper init failed");
    }
}

void Display::Present()
{
    _display.SetFrameMemory(_paint.GetImage(), 0, 0, _paint.GetWidth(), _paint.GetHeight());
    _display.DisplayFrame(true);
}

void Display::RenderNavigator(PageNavigator& navigator)
{
    navigator.SetWdith(EPD_WIDTH);
    navigator.SetHeight(267);
    navigator.Render(_paint, 0, 0);
}

void Display::RenderStatusBar(const float batVoltage, const int tankLevel, const bool wifi, const bool connected)
{
    _paint.DrawFilledRectangle(0, 267, EPD_WIDTH, EPD_HEIGHT, WHITE);
    _paint.DrawHorizontalLine(0, 267, EPD_WIDTH, 0);
    RenderBatteryIndicator(4, 272, batVoltage);
    RenderTankIndicator(100, 272, tankLevel);
    if (wifi)
    {
        RenderOnlineIndicator(60, 272, connected);
    }
}

void Display::RenderTankIndicator(const uint32_t x, const uint32_t y, const int v)
{
    sIMAGE* img = &IMG_tank_100;
    if (v < 15)
        img = &IMG_tank_0;
    else if (v < 35)
        img = &IMG_tank_25;
    else if (v < 60)
        img = &IMG_tank_50;
    else if (v < 85)
        img = &IMG_tank_75;

    _paint.DrawImage(x, y, img);
}

void Display::RenderBatteryIndicator(const uint32_t x, const uint32_t y, const float v)
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

void Display::RenderOnlineIndicator(const uint32_t x, const uint32_t y, const bool online)
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

void Display::RenderBusyAnimation(const uint32_t x, const uint32_t y)
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