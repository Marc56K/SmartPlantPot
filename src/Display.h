#pragma once
#include <epd2in9.h>
#include <epdpaint.h>
#include <array>
#include "PageNavigator.h"

class Display
{
public:
    Display();
    ~Display();

    void Init();
    void Present();

    void RenderNavigator(PageNavigator& navigator);
    void RenderStatusBar(const float batVoltage, const int tankLevel, const bool wifi, const bool connected);

    void RenderTankIndicator(const uint32_t x, const uint32_t y, const int sensorValue);
    void RenderBatteryIndicator(const uint32_t x, const uint32_t y, const float voltage);
    void RenderOnlineIndicator(const uint32_t x, const uint32_t y, const bool online);
    void RenderBusyAnimation(const uint32_t x, const uint32_t y);    

private:
    std::array<unsigned char, (EPD_WIDTH * EPD_HEIGHT)> _frameBuffer;
    Paint _paint;
    Epd _display;
};