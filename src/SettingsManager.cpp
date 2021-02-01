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

void SettingsManager::ClearEEPROM()
{
    EEPROM.begin(EEPROM_SIZE);
    uint8_t* ptr = EEPROM.getDataPtr();
    memset(ptr, 0, EEPROM_SIZE);
    EEPROM.commit();
    EEPROM.end();
}

void SettingsManager::LoadFromEEPROM()
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
    if (!HasValue(TIME_SERVER))
    {
        SetValue(TIME_SERVER, "pool.ntp.org");
    }

    if (!HasValue(TIME_OFFSET))
    {
        SetValue(TIME_OFFSET, "0");
    }

    if (!HasValue(PUMP_DURATION))
    {
        SetValue(PUMP_DURATION, "0.5");
    }

    if (!HasValue(PUMP_TIME_HH))
    {
        SetValue(PUMP_TIME_HH, "7");
    }

    if (!HasValue(PUMP_TIME_MM))
    {
        SetValue(PUMP_TIME_MM, "0");
    }

    SaveToEEPROM();
}

void SettingsManager::SaveToEEPROM()
{
    if (_pendingChanges)
    {
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
    auto it = _settings.find(key);
    if (it != _settings.end())
    {
        return atoi((const char*)it->second.data());
    }
    return 0;
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