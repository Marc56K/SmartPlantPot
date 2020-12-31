#pragma once
#include <map>
#include <vector>
#include <string>

enum Setting : uint8_t
{
    PUMP_DURATION = 0,
    TARGET_SOIL_MOISTURE,    
    NUM_SETTINGS
};

const char SettingName[][32] =
{
    "PUMP_DURATION",
    "TARGET_SOIL_MOISTURE",
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
};
 