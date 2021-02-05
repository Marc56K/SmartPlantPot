#pragma once
#include "BaseControl.h"
#include "AppContext.h"

class HomePage : public BaseControl
{
public:
    HomePage(AppContext& ctx);
    virtual ~HomePage();

    virtual bool Click() override;
    virtual bool Scroll(const int delta) override;
    
    virtual void Render(Paint& paint, const int x, const int y) override;

private:
    AppContext& _ctx;
};