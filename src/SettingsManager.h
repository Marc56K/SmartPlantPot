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
    TIME_OFFSET_HOURES,
    SCHEDULE_DAY_MO,
    SCHEDULE_DAY_TU,
    SCHEDULE_DAY_WE,
    SCHEDULE_DAY_TH,
    SCHEDULE_DAY_FR,
    SCHEDULE_DAY_SA,
    SCHEDULE_DAY_SU,
    SCHEDULE_TIME_HH,
    SCHEDULE_TIME_MM,
    PUMP_IMPULSE_SEC,
    MAX_PUMP_IMPULSES,
    SEEPAGE_DURATION_MINUTES,
    SOIL_MOISTURE_PERCENT,
    SLEEP_DURATION_MINUTES,    
    MQTT_SERVER,
    MQTT_PORT,
    MQTT_USER,
    MQTT_KEY,
    MQTT_TOPIC,
    MQTT_ENABLED,
    PUMP_ENABLED,
    WAKE_DURATION_SEC,
    NUM_SETTINGS
};

class SettingsManager
{
public:
    SettingsManager();
    ~SettingsManager();

    void Init();
    void ClearEEPROM();    
    void SaveToEEPROM();

    bool HasPendingChanges();

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
 