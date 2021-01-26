#pragma once
#include <epd2in9.h>
#include <epdpaint.h>
#include <string>
#include "AppContext.h"
#include <PageNavigator.h>
#include "TextPage.h"

class Display
{
public:
    Display();
    ~Display();

    PageNavigator& Navigator() { return _navigator; }
    TextPage& InfoPage() { return *_infoPage.get(); }

    bool Init(AppContext& ctx);

    void Clear();
    bool Present();

    void RenderTankIndicator(const uint32_t x, const uint32_t y, const float sensorValue);
    void RenderBatteryIndicator(const uint32_t x, const uint32_t y, const float voltage);
    void RenderOnlineIndicator(const uint32_t x, const uint32_t y, const bool online);
    void RenderBusyAnimation(const uint32_t x, const uint32_t y);

    void Render(AppContext& ctx);

private:
    unsigned char _buffer[EPD_WIDTH * EPD_HEIGHT];
    Paint _paint;
    Epd _epd;
    PageNavigator _navigator;
    std::shared_ptr<TextPage> _infoPage;
};