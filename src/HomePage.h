#pragma once
#include "BaseControl.h"
#include "AppContext.h"

class HomePage : public BaseControl
{
public:
    HomePage(AppContext& ctx);
    virtual ~HomePage();
    
    virtual void Render(Paint& paint, const int x, const int y) override;

private:
    AppContext& _ctx;
};