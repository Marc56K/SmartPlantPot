#include "InfoPage.h"

InfoPage::InfoPage(AppContext& ctx) :
    _ctx(ctx)
{
}

InfoPage::~InfoPage()
{
}

void InfoPage::Render(Paint& paint, const int x, const int y)
{
    auto now = _ctx.GetClock().Now();
    auto& sm = _ctx.GetSensorMgr();
    auto& lines = Lines();
    lines.clear();
    lines.push_back(std::string("ETA: ") + String(_ctx.GetPowerMgr().GetTimeUntilSleep()).c_str() + "s");
    lines.push_back(std::string("BAT: ") + String(sm.GetBatVoltage()).c_str() + "V");
    lines.push_back(std::string("DATE: ") + now.GetDate(false).c_str());
    lines.push_back(std::string("WDAY: ") + now.GetWDay(false).c_str());
    lines.push_back(std::string("TIME: ") + now.GetTime(false).c_str());
    lines.push_back(std::string("SOIL: ") + String(sm.GetSoilMoisture()).c_str() + "%");
    lines.push_back(std::string("TANK: ") + String(sm.GetWaterTankLevel(false)).c_str() + "ml");
    lines.push_back(std::string("TEMP: ") + String(sm.GetTemperature()).c_str());

    TextPage::Render(paint, x, y);
}