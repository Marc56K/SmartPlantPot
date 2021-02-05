#include "HomePage.h"
#include "Arduino.h"
#include <images.h>

extern sIMAGE IMG_root;
extern sIMAGE IMG_soil;
extern sIMAGE IMG_wet_soil;

HomePage::HomePage(AppContext& ctx) 
    : _ctx(ctx)
{
}

HomePage::~HomePage()
{
}

bool HomePage::Click()
{
    if (IsSelected())
    {
        Deselect();
    }
    else
    {
        Select();
    }    
    return IsSelected();
}

bool HomePage::Scroll(const int delta)
{
    if (IsSelected() && delta != 0)
    {
        auto& sm = _ctx.GetSettingsMgr();
        int level = sm.GetIntValue(SOIL_MOISTURE_PERCENT);
        level -= delta;
        level = std::max(0, std::min(100, level));
        sm.SetValue(SOIL_MOISTURE_PERCENT, level);
        sm.SaveToEEPROM();
    }    
    return IsSelected();
}

void HomePage::Render(Paint& paint, const int x, const int y)
{
    const uint32_t top = y;
    const uint32_t bottom = 267;
    const uint32_t totalRange = bottom - top;
    const uint32_t height = round(totalRange * max(0.0f, min<float>(1.0f, _ctx.GetSensorMgr().GetSoilMoisture() / 100.0f)));
    const uint32_t yStart = top + totalRange - height;
    
    paint.DrawImage(0, yStart - IMG_soil.Height, &IMG_soil);
    paint.DrawImage(0, yStart, &IMG_wet_soil);
    paint.DrawImage(0, y, &IMG_root);

    const int soilMoistLevel = _ctx.GetSettingsMgr().GetIntValue(SOIL_MOISTURE_PERCENT);
    const float level = soilMoistLevel / 100.0f;
    const uint32_t levelLine = top + (1.0f - level) * IMG_root.Height;
    const int segWidth = 5;
    static int xOffset = 0;
    if (IsSelected())
    {
        xOffset += 2;
        const int xCenter = IMG_root.Width / 2;
        const int yTop = levelLine + 3;
        const String txt = String(soilMoistLevel) + "%";
        const int halfTxtWidth = txt.length() * _font.Width / 2;
        paint.DrawFilledRectangle(xCenter - halfTxtWidth, yTop, xCenter + halfTxtWidth, yTop + _font.Height, WHITE);
        paint.DrawUtf8StringAt(xCenter, yTop + 1, txt.c_str(), &_font, BLACK, CENTER);
    }

    for (int x = -2 * segWidth; x < IMG_root.Width; x += segWidth)
    {
        const int color = (x / segWidth) % 2 == 0 ? BLACK : WHITE;
        const int xPos = x + (xOffset % (2 * segWidth));
        paint.DrawHorizontalLine(xPos + 1, levelLine - 1, segWidth, color);
        paint.DrawHorizontalLine(xPos,     levelLine,     segWidth, color);
        paint.DrawHorizontalLine(xPos - 1, levelLine + 1, segWidth, color);
    }
}