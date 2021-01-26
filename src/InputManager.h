#pragma once

class InputManager
{
public:
    void Init();
    bool ButtonPressed();
    int GetRotaryEncoderDelta();
};