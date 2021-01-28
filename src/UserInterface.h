#pragma once
#include "Display.h"
#include "InputManager.h"
#include "PageNavigator.h"

class AppContext;
class UserInterface
{
public:
    UserInterface(AppContext& ctx);
    ~UserInterface();

    void Init();
    void Update();

private:
    AppContext& _ctx;
    Display _display;
    InputManager _inputMgr;
    PageNavigator _navigator;
};