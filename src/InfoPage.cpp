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
    lines.push_back(std::string("ETA: ") + String(_ctx.GetEspSleepMgr().GetRemainingWakeTime()).c_str() + "s");
    lines.push_back(std::string("BAT: ") + String(sm.GetBatVoltage()).c_str() + "V");
    lines.push_back(std::string("DT: ") + now.ToString(true, false).c_str());
    lines.push_back(std::string("TIME: ") + now.ToString(false, true).c_str());
    lines.push_back(std::string("SOIL: ") + String(sm.GetSoilHumidity()).c_str());
    lines.push_back(std::string("TANK: ") + String(sm.GetWaterTankLevel()).c_str());
    lines.push_back(std::string("TEMP: ") + String(sm.GetTemperature()).c_str());

    TextPage::Render(paint, x, y);
}