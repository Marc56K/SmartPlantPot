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
    unsigned long GetLastInputTime();

private:
    void EncoderIsrTaskProc();

private:
    AiEsp32RotaryEncoder _rotaryEncoder;
    SemaphoreHandle_t _mutex;
    TaskHandle_t _encoderIsrTask = nullptr;    
    volatile int16_t _encoderDelta = 0;
    volatile int16_t _lastEncoderPos = 0;
    volatile unsigned long _lastEncoderTime = 0;
};