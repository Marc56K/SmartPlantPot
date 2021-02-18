#pragma once
#include <Arduino.h>

class AppContext;
class SerialInterface
{
public:
    SerialInterface(AppContext& ctx);
    ~SerialInterface();

    void Init();
    void Update();

private:
    void PrintHelp();
    void PrintSettings();
    bool ReadInput(String &cmd, String &arg);
    void SetSetting(const String& arg);
    bool SplitKeyValue(const String &input, const String& delimiter, String &key, String &value);

private:
    AppContext& _ctx;
};