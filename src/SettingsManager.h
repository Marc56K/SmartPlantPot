#pragma once
#include "PageNavigator.h"
#include "PropertyPage.h"
#include <map>
#include <vector>
#include <string>

#define SETTINGS(setting) \
    setting(WIFI_SSID) \
    setting(WIFI_KEY) \
    setting(TIME_SERVER) \
    setting(TIME_OFFSET_HOURES) \
    setting(WAKE_DURATION_SEC) \
    setting(SLEEP_DURATION_MINUTES) \
    setting(SEEPAGE_DURATION_MINUTES) \
    setting(SOIL_MOISTURE_PERCENT) \
    setting(PUMP_ENABLED) \
    setting(WATERING_TIME_HH) \
    setting(WATERING_TIME_MM) \
    setting(WATERING_INTERVAL_DAYS) \
    setting(PUMPING_DURATION_SEC) \
    setting(PUMPING_POWER_PERCENT) \
    setting(MAX_PUMPING_REPEATS) \
    setting(MQTT_ENABLED) \
    setting(MQTT_SERVER) \
    setting(MQTT_PORT) \
    setting(MQTT_USER) \
    setting(MQTT_KEY) \
    setting(MQTT_TOPIC) \
    setting(SERIAL_CTRL_ENABLED) \
    setting(SOIL_SENSOR_MAX_VALUE) \
    setting(SOIL_SENSOR_MIN_VALUE) \
    setting(TANK_SENSOR_MAX_VALUE) \
    setting(TANK_SENSOR_MIN_VALUE) \
    setting(NUM_SETTINGS)

#define CREATE_SETTINGS_ENUM(name) name,
#define CREATE_SETTINGS_STRINGS(name) #name,
enum Setting : uint8_t
{ 
    SETTINGS(CREATE_SETTINGS_ENUM)
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
    bool SetValue(const std::string& key, const std::string& value);

    std::map<std::string, std::string> GetKeyValuesAsString();

private:
    void InitDefaultValues();
    uint32_t ComputeHash(void* ptr, uint32_t size);

private:
    std::map<Setting, std::vector<uint8_t>> _settings;
    bool _pendingChanges;
};
 