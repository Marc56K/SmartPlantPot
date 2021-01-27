#pragma once
#include <epd2in9.h>
#include <epdpaint.h>
#include <string>
#include "InputManager.h"
#include <PageNavigator.h>

class AppContext;
class UserInterface
{
public:
    UserInterface(AppContext& ctx);
    ~UserInterface();

    bool Init();
    void Update();

private:
    void HandleInput();
    void UpdateDisplay();

    void RenderPages();
    void RenderStatusBar();
    void RenderTankIndicator(const uint32_t x, const uint32_t y, const float sensorValue);
    void RenderBatteryIndicator(const uint32_t x, const uint32_t y, const float voltage);
    void RenderOnlineIndicator(const uint32_t x, const uint32_t y, const bool online);
    void RenderBusyAnimation(const uint32_t x, const uint32_t y);    

private:
    AppContext& _ctx;
    InputManager _inputMgr;
    unsigned char _buffer[EPD_WIDTH * EPD_HEIGHT];
    Paint _paint;
    Epd _epd;
    PageNavigator _navigator;
};