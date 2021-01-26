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
    PUMP_TIME_HH,
    PUMP_TIME_MM,
    PUMP_DURATION,
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

    void CreatePropertyPages(PageNavigator& navigator);

private:
    void InitDefaultValues();
    uint32_t ComputeCrc(void* ptr, uint32_t size);

private:
    std::map<Setting, std::vector<uint8_t>> _settings;
    bool _pendingChanges;
};
 