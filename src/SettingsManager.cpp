#include "SettingsManager.h"
#include <EEPROM.h>
#include <stdlib.h>
#include <sstream>

#define EEPROM_SIZE 512

SettingsManager::SettingsManager()
    : _pendingChanges(false)
{
}

SettingsManager::~SettingsManager()
{    
}

void SettingsManager::Init()
{
    _settings.clear();
    EEPROM.begin(EEPROM_SIZE);
    uint8_t* ptr = EEPROM.getDataPtr();

    uint32_t crc = *((uint32_t*)ptr);
    ptr += sizeof(uint32_t);
    uint32_t actualCrc = ComputeCrc(ptr, EEPROM_SIZE - sizeof(uint32_t));
    if (crc == actualCrc)
    {
        uint8_t numEntries = *ptr;
        ptr += sizeof(uint8_t);
        for (uint8_t i = 0; i < numEntries; i++)
        {
            Setting key = *((Setting*)ptr);
            ptr += sizeof(Setting);

            uint16_t size = *((uint16_t*)ptr);
            ptr += sizeof(uint16_t);

            std::vector<uint8_t> value(size);
            memcpy(value.data(), ptr, size);
            ptr += size;

            _settings[key] = value;
        }
    }
    EEPROM.end();

    _pendingChanges = false;

    InitDefaultValues();
}

void SettingsManager::InitDefaultValues()
{
    auto init = [&](const Setting s, const char* val)
    {
        if (!HasValue(s)) SetValue(s, val);
    };

    init(TIME_SERVER, "pool.ntp.org");
    init(TIME_OFFSET_HOURES, "1");
    init(SCHEDULE_DAY_MO, "0");
    init(SCHEDULE_DAY_TU, "0");
    init(SCHEDULE_DAY_WE, "1");
    init(SCHEDULE_DAY_TH, "0");
    init(SCHEDULE_DAY_FR, "0");
    init(SCHEDULE_DAY_SA, "0");
    init(SCHEDULE_DAY_SU, "1");
    init(SCHEDULE_TIME_HH, "12");
    init(SCHEDULE_TIME_MM, "0");
    init(PUMP_IMPULSE_SEC, "0.5");
    init(MAX_PUMP_IMPULSES, "5");
    init(SEEPAGE_DURATION_MINUTES, "5");
    init(SOIL_MOISTURE_PERCENT, "30");
    init(SLEEP_DURATION_MINUTES, "60");
    init(MQTT_SERVER, "io.adafruit.com");
    init(MQTT_PORT, "1883");
    init(MQTT_USER, "Unknown");
    init(MQTT_KEY, "");
    init(MQTT_TOPIC, "Unknown/feeds/smartplantpot.");
    init(MQTT_ENABLED, "0");
    init(PUMP_ENABLED, "1");

    SaveToEEPROM();
}

void SettingsManager::ClearEEPROM()
{
    EEPROM.begin(EEPROM_SIZE);
    uint8_t* ptr = EEPROM.getDataPtr();
    memset(ptr, 0, EEPROM_SIZE);
    EEPROM.commit();
    EEPROM.end();
}

void SettingsManager::SaveToEEPROM()
{
    if (_pendingChanges)
    {
        Serial.println("writing EEPROM");
        EEPROM.begin(EEPROM_SIZE);
        uint8_t numEntries = (uint8_t)_settings.size();
        uint8_t* ptr = EEPROM.getDataPtr();

        uint32_t* crcPtr = (uint32_t*)ptr;
        ptr += sizeof(uint32_t);

        *ptr = numEntries;
        ptr += sizeof(uint8_t);
        for (auto& it : _settings)
        {
            *((Setting*)ptr) = it.first;
            ptr += sizeof(Setting);

            *((uint16_t*)ptr) = (uint16_t)it.second.size();
            ptr += sizeof(uint16_t);

            memcpy(ptr, it.second.data(), it.second.size());
            ptr += it.second.size();
        }

        *crcPtr = ComputeCrc(EEPROM.getDataPtr() + sizeof(uint32_t), EEPROM_SIZE - sizeof(uint32_t));
        EEPROM.commit();
        EEPROM.end();

        _pendingChanges = false;
    }
}

bool SettingsManager::HasPendingChanges()
{
    return _pendingChanges;
}

bool SettingsManager::HasValue(Setting key)
{
    return _settings.find(key) != _settings.end();
}

float SettingsManager::GetFloatValue(Setting key)
{
    auto it = _settings.find(key);
    if (it != _settings.end())
    {
        return (float)atof((const char*)it->second.data());
    }
    return 0;
}

int SettingsManager::GetIntValue(Setting key)
{
    return (int)GetFloatValue(key);
}

std::string SettingsManager::GetStringValue(Setting key)
{
    auto it = _settings.find(key);
    if (it != _settings.end())
    {
        return std::string((const char*)it->second.data());
    }
    return "";
}

void SettingsManager::SetValue(Setting key, float value)
{
    std::stringstream ss;
    ss << value;
    SetValue(key, ss.str());
}

void SettingsManager::SetValue(Setting key, const std::string& value)
{
    std::vector<uint8_t> val(value.length() + 1);
    memcpy(val.data(), value.c_str(), value.length());
    _settings[key] = val;
    _pendingChanges = true;
}

uint32_t SettingsManager::ComputeCrc(void* ptr, uint32_t size)
{
    uint32_t result = 0;
    for (uint32_t i = 0; i < size / sizeof(uint32_t); i++)
    {
        result += ((uint32_t*)ptr)[i];
    }
    return result;
}