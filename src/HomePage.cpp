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

void HomePage::Render(Paint& paint, const int x, const int y)
{
    const uint32_t top = y;
    const uint32_t bottom = 267;
    const uint32_t totalRange = bottom - top;
    const uint32_t height = round(totalRange * max(0.0f, min<float>(1.0f, _ctx.GetSensorMgr().GetSoilHumidity() / 100.0f)));
    const uint32_t yStart = top + totalRange - height;
    
    paint.DrawImage(0, yStart - IMG_soil.Height, &IMG_soil);
    paint.DrawImage(0, yStart, &IMG_wet_soil);

    paint.DrawImage(0, y, &IMG_root);
}