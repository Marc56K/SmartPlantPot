#pragma once
#include <Arduino.h>
#include "Config.h"
#include "AiEsp32RotaryEncoder.h"

class InputManager
{
public:
    InputManager();
    ~InputManager();
    void Init();
    bool ButtonPressed();
    int GetRotaryEncoderDelta();

private:
    void EncoderIsrTaskProc();

private:
    int16_t _lastEncoderPos = 0;
};