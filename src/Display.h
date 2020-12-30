#pragma once
#include "SensorManager.h"
#include <epd2in9.h>
#include <epdpaint.h>
#include <string>

class Display
{
public:
    Display();
    ~Display();

    bool Init();

    void Clear();
    bool Present();

    void RenderTankIndicator(const uint32_t x, const uint32_t y, const float sensorValue);
    void RenderBatteryIndicator(const uint32_t x, const uint32_t y, const float voltage);
    void RenderOnlineIndicator(const uint32_t x, const uint32_t y, const bool online);
    void RenderBusyAnimation(const uint32_t x, const uint32_t y);
    void RenderSoilHumidityIndicator(const uint32_t top, const uint32_t bottom, const float sensorValue);

    void RenderMainScreen(SensorManager& sm);
    void RenderDebugMessages(std::vector<String>& messages);

private:
    unsigned char _buffer[EPD_WIDTH * EPD_HEIGHT];
    Paint _paint;
    Epd _epd;
};