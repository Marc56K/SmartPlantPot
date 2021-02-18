#include "SerialInterface.h"
#include "AppContext.h"

SerialInterface::SerialInterface(AppContext &ctx) : _ctx(ctx)
{
}

SerialInterface::~SerialInterface()
{
}

void SerialInterface::Init()
{
    PrintHelp();
}

void SerialInterface::Update()
{
    String cmd, arg;
    while (ReadInput(cmd, arg))
    {
        _ctx.GetPowerMgr().ResetAutoSleepTimer(true);
        Serial.println(String(F("CMD: ")) + cmd + (arg.length() > 0 ? String(F(" ARG: ")) + arg : String("")));
        if (cmd == "settings")
        {
            PrintSettings();
        }
        else if (cmd == "set")
        {
            SetSetting(arg);
        }
        else if (cmd == "save")
        {
            _ctx.GetSettingsMgr().SaveToEEPROM();
        }
        else if (cmd == "runpump")
        {
            _ctx.GetPowerMgr().StartPumping();
        }
        else
        {
            PrintHelp();
        }
    }
}

void SerialInterface::PrintHelp()
{
    Serial.println(F("*************************************************************"));
    Serial.println(F("USAGE:"));
    Serial.println(F("  help              : prints this message"));
    Serial.println(F("  settings          : prints current settings"));
    Serial.println(F("  set               : change value of setting (e.g set WIFI_SSID=My WiFi)"));
    Serial.println(F("  save              : write settings to EEPROM"));
    Serial.println(F("  runpump           : runs the pump now"));
    Serial.println(F("*************************************************************"));
}

void SerialInterface::PrintSettings()
{
    Serial.println(F("*************************************************************"));
    Serial.println(F("SETTINGS:"));
    const auto settings = _ctx.GetSettingsMgr().GetKeyValuesAsString();
    for (auto& s : settings)
    {
        Serial.println(String(" ") + s.first.c_str() + "=" + s.second.c_str());
    }
    Serial.println("*************************************************************");
}

bool SerialInterface::ReadInput(String &cmd, String &arg)
{
    String buff = "";
    while (Serial.available())
    {
        char c = Serial.read();
        if (c == '\r' || c == '\n')
        {
            continue;
        }

        buff += c;
        delay(1);
    }

    buff.trim();

    if (buff.length() == 0)
    {
        cmd = "";
        arg = "";
        return false;
    }

    SplitKeyValue(buff, " ", cmd, arg);
    return true;
}

void SerialInterface::SetSetting(const String& arg)
{
    String key, value;
    if (SplitKeyValue(arg, "=", key, value))
    {
        if (_ctx.GetSettingsMgr().SetValue(key.c_str(), value.c_str()))
        {
            Serial.println(String("") + key.c_str() + "=" + value.c_str());
        }
        else
        {
            Serial.println(String("Setting not found: ") + key);
        }
    }
    else
    {
        Serial.println(String("Syntax error: ") + arg);
    }
}

bool SerialInterface::SplitKeyValue(const String &input, const String& delimiter, String &key, String &value)
{
    int idx = input.indexOf(delimiter);
    if (idx > -1)
    {
        key = input.substring(0, idx);
        value = (idx + 1 < input.length()) ? input.substring(idx + 1) : "";
        return true;
    }

    key = input;
    value = "";
    return false;
}