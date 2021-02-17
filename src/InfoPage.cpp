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
    auto& s = _ctx.GetSensorMgr().States();
    auto& lines = Lines();
    lines.clear();
    lines.push_back(std::string("WAKE: ") + String(_ctx.GetPowerMgr().GetTimeUntilSleep()).c_str() + "s");    
    lines.push_back(std::string("DATE: ") + now.GetDate(false).c_str());
    lines.push_back(std::string("WDAY: ") + now.GetWDay(false).c_str());
    lines.push_back(std::string("TIME: ") + now.GetTime(false).c_str());
    lines.push_back(std::string("BAT: ") + String(s.BatVoltage).c_str() + "V");
    lines.push_back(std::string("TEMP: ") + String(s.Temperature).c_str());    
    lines.push_back(std::string("SOIL: ") + String(s.SoilMoistureRaw).c_str() + " (" + String(s.SoilMoistureInPerCent).c_str() + "%)");
    lines.push_back(std::string("TANK: ") + String(s.WaterTankLevelRaw).c_str() + " (" + String(s.WaterTankLevelInPerCent).c_str() + "%)");
    

    TextPage::Render(paint, x, y);
}