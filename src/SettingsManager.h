#pragma once
#include "PageNavigator.h"
#include "PropertyPage.h"
#include <map>
#include <vector>
#include <string>

enum Setting : uint8_t
{
    WIFI_SSID = 0,
    WIFI_KEY,
    TIME_SERVER,
    TIME_OFFSET,
    SCHEDULE_DAY_MO,
    SCHEDULE_DAY_TU,
    SCHEDULE_DAY_WE,
    SCHEDULE_DAY_TH,
    SCHEDULE_DAY_FR,
    SCHEDULE_DAY_SA,
    SCHEDULE_DAY_SU,
    SCHEDULE_TIME_HH,
    SCHEDULE_TIME_MM,
    PUMP_IMPULSE,
    PUMP_CYCLES,
    SEEPAGE_DURATION,
    SOIL_HUMIDITY,
    NUM_SETTINGS
};

class SettingsManager
{
public:
    SettingsManager();
    ~SettingsManager();

    void ClearEEPROM();
    void LoadFromEEPROM();
    void SaveToEEPROM();

    bool HasValue(Setting key);
    float GetFloatValue(Setting key);
    int GetIntValue(Setting key);
    std::string GetStringValue(Setting key);
    void SetValue(Setting key, float value);
    void SetValue(Setting key, const std::string& value);

private:
    void InitDefaultValues();
    uint32_t ComputeCrc(void* ptr, uint32_t size);

private:
    std::map<Setting, std::vector<uint8_t>> _settings;
    bool _pendingChanges;
};
 