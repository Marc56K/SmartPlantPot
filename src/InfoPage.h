#pragma once
#include "TextPage.h"
#include "AppContext.h"

class InfoPage : public TextPage
{
public:
    InfoPage(AppContext& ctx);
    virtual ~InfoPage();

    virtual void Render(Paint& paint, const int x, const int y) override;

private:
    AppContext& _ctx;
};