#include "Display.h"
#include <images.h>

extern sIMAGE IMG_root;
extern sIMAGE IMG_soil;
extern sIMAGE IMG_wet_soil;
extern sIMAGE IMG_tank_0;
extern sIMAGE IMG_tank_25;
extern sIMAGE IMG_tank_50;
extern sIMAGE IMG_tank_75;
extern sIMAGE IMG_tank_100;

Display::Display()
    : _paint(_buffer, EPD_WIDTH, EPD_HEIGHT)
{

}

Display::~Display()
{
    
}

bool Display::Init()
{
    if (_epd.Init(lut_full_update) != 0)
    {
        Serial.print("e-Paper init failed");
        return false;
    }

    _epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
    _epd.DisplayFrame();
    _epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
    _epd.DisplayFrame();

    if (_epd.Init(lut_partial_update) != 0)
    {
        Serial.println("e-Paper init failed");
        return false;
    }

    return true;
}

void Display::Clear()
{
    _paint.Clear(1);
}

bool Display::Present()
{
    _epd.SetFrameMemory(_paint.GetImage(), 0, 0, _paint.GetWidth(), _paint.GetHeight());
    _epd.DisplayFrame();
    return true;
}

void Display::RenderTankIndicator(const uint32_t x, const uint32_t y, const float v)
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

void Display::RenderSoilHumidityIndicator(const uint32_t top, const uint32_t bottom, const float sensorValue)
{
    const uint32_t totalRange = bottom - top;
    const uint32_t height = round(totalRange * max(0.0f, min(1.0f, sensorValue)));
    const uint32_t yStart = top + totalRange - height;
    //const uint32_t yEnd = yStart + height;

    _paint.DrawImage(0, yStart - IMG_soil.Height, &IMG_soil);
    _paint.DrawImage(0, yStart, &IMG_wet_soil);
}

void Display::RenderMainScreen(SensorManager& sm)
{
    RenderSoilHumidityIndicator(0, 267, sm.GetSoilHumidity());
    _paint.DrawImage(0, 0, &IMG_root);

    _paint.DrawFilledRectangle(0, 267, EPD_WIDTH, EPD_HEIGHT, 1);
    _paint.DrawHorizontalLine(0, 267, EPD_WIDTH, 0);

    RenderBatteryIndicator(4, 272, sm.GetBatVoltage());
    RenderTankIndicator(100, 272, sm.GetWaterTankLevel());
}

void Display::RenderDebugMessages(std::vector<String>& messages)
{
    auto& fnt = Font12;
    for (size_t i = 0; i < messages.size(); i++)
    {
        String& msg = messages[i];
        int y = 2 + i * (fnt.Height + 2);

        _paint.DrawFilledRectangle(0, y, msg.length() * fnt.Width + 2, y + fnt.Height, 0);
        _paint.DrawUtf8StringAt(2, y + 1, msg.c_str(), &fnt, 1);
    }
}

void Display::RenderSettingsScreen(PropertyPage& settingsPage)
{
    settingsPage.Render(_paint, 1, 150);
}