#include "InputManager.h"
#include <Arduino.h>
#include "Config.h"
#include "AiEsp32RotaryEncoder.h"

namespace InputManager
{
    AiEsp32RotaryEncoder rotaryEncoder(ROTENC_A_PIN, ROTENC_B_PIN, -1, -1);
    volatile int16_t encoderDelta = 0;
    volatile int16_t lastEncoderPos = 0;
    volatile unsigned long lastEncoderTime = 0;
    void EncoderISR()
    {
        rotaryEncoder.readEncoder_ISR();
        int16_t encoderPos = rotaryEncoder.readEncoder();
        if (lastEncoderPos != encoderPos)
        {
            auto now = millis();
            unsigned long deltaT = now - lastEncoderTime;
            lastEncoderTime = now;
            int16_t delta = encoderPos - lastEncoderPos;
            lastEncoderPos = encoderPos;
            if (deltaT > 10 && deltaT < 200)
            {
                encoderDelta += delta;
                //Serial.println(String("RAW ENC: ") + deltaT + " - > " + delta);
            }
        }
    }

    unsigned long buttonPressedTime = 0;
    volatile bool buttonPressed = false;
    void ButtonISR()
    {
        auto now = millis();
        if (now - buttonPressedTime > 500)
        {
            buttonPressedTime = now;
            buttonPressed = true;
        }        
    }

    void Init()
    {        
        rotaryEncoder.begin();
        rotaryEncoder.setup(EncoderISR);
        rotaryEncoder.setBoundaries(INT16_MIN / 2, INT16_MAX / 2, false);
        pinMode(ROTENC_SW_PIN, INPUT);
        attachInterrupt(ROTENC_SW_PIN, ButtonISR, FALLING);
    }
    
    bool ButtonPressed()
    {
        bool result = buttonPressed;
        buttonPressed = false;
        return result;
    }

    int GetRotaryEncoderDelta()
    {
        auto result = encoderDelta;
        encoderDelta = 0;
        return result;
    }
}