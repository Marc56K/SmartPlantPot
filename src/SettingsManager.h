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
    WAKE_DURATION_SEC,
    SLEEP_DURATION_MINUTES,
    SEEPAGE_DURATION_MINUTES,
    SOIL_MOISTURE_PERCENT,   
    PUMP_ENABLED,
    WATERING_TIME_HH,
    WATERING_TIME_MM,
    WATERING_INTERVAL_DAYS,
    PUMPING_DURATION_SEC,
    MAX_PUMPING_REPEATS, 
    MQTT_ENABLED,
    MQTT_SERVER,
    MQTT_PORT,
    MQTT_USER,
    MQTT_KEY,
    MQTT_TOPIC,
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
    uint32_t ComputeHash(void* ptr, uint32_t size);

private:
    std::map<Setting, std::vector<uint8_t>> _settings;
    bool _pendingChanges;
};
 