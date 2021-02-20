#include "SerialInterface.h"
#include "AppContext.h"

SerialInterface::SerialInterface(AppContext &ctx) :
    _ctx(ctx), _lastRemainingSeconds(-1)
{
}

SerialInterface::~SerialInterface()
{
}

void SerialInterface::Init()
{
    if (IsEnabled())
    {
        Serial.println("serial interface is enabled!");
        PrintHelp();
    }
}

void SerialInterface::Update()
{
    if (IsEnabled())
    {
        const auto s = _ctx.GetPowerMgr().GetSecondsUntilSleep();
        if (_lastRemainingSeconds != s && s <= 10)
        {
            Serial.println(String("sleeping in ") + s + "s");
        }
        _lastRemainingSeconds = s;

        String cmd, arg;
        if (ReadInput(cmd, arg))
        {
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
                _ctx.GetPowerMgr().RunWaterPump(false);
            }
            else if (cmd == "restart")
            {
                ESP.restart();
            }
            else if (cmd == "sleep")
            {
                _ctx.GetPowerMgr().RequestDeepSleep();
            }
            else if (cmd == "help")
            {
                PrintHelp();
            }
            else
            {
                return;
            }

            _ctx.GetPowerMgr().ResetAutoSleepTimer(true);        
        }
    }
}

bool SerialInterface::IsEnabled()
{
    return _ctx.GetSettingsMgr().GetIntValue(Setting::SERIAL_INPUT_ENABLED) != 0;
}

void SerialInterface::PrintHelp()
{
    Serial.println(F("*************************************************************"));
    Serial.println(F("USAGE:"));
    Serial.println(F("  help      : prints this message"));
    Serial.println(F("  settings  : prints current settings"));
    Serial.println(F("  set       : change setting (e.g set WIFI_SSID=My WiFi)"));
    Serial.println(F("  runpump   : runs the pump now"));
    Serial.println(F("  restart   : restart ESP"));
    Serial.println(F("  sleep     : enter deep-sleep mode"));
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
    Serial.println(F("*************************************************************"));
}

bool SerialInterface::ReadInput(String &cmd, String &arg)
{
    cmd = "";
    arg = "";
    String buff = "";
    while (Serial.available())
    {
        char c = Serial.read();
        if (c == '\r' || c == '\n')
        {
            continue;
        }

        if (c > 127)
        {
            // HACK: ignore garbage data that is sometimes received after usb cable was disconnected
            Serial.flush();
            return false;
        }

        buff += c;
        if (Serial.available() == 0)
        {
            delay(1);
        }
    }

    buff.trim();

    if (buff.length() > 0)
    {
        SplitKeyValue(buff, " ", cmd, arg);
    }

    return cmd.length() > 0;
}

void SerialInterface::SetSetting(const String& arg)
{
    String key, value;
    if (SplitKeyValue(arg, "=", key, value))
    {
        if (_ctx.GetSettingsMgr().SetValue(key.c_str(), value.c_str()))
        {
            Serial.println(String("") + key.c_str() + "=" + value.c_str());
            _ctx.GetSettingsMgr().SaveToEEPROM();
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