#include "InputManager.h"

AiEsp32RotaryEncoder rotaryEncoder(ROTENC_A_PIN, ROTENC_B_PIN, -1, -1);
void IRAM_ATTR EncoderISR()
{
    rotaryEncoder.readEncoder_ISR();
}

volatile unsigned long buttonPressedTime = 0;
volatile bool buttonPressed = false;
void IRAM_ATTR ButtonISR()
{
    auto now = millis();
    if (now - buttonPressedTime > 500)
    {
        buttonPressedTime = now;
        buttonPressed = true;
    }        
}

InputManager::InputManager()
{
}

InputManager::~InputManager()
{
}

void InputManager::Init()
{        
    rotaryEncoder.begin();
    rotaryEncoder.setBoundaries(INT16_MIN / 2, INT16_MAX / 2, false);
    attachInterrupt(digitalPinToInterrupt(ROTENC_A_PIN), EncoderISR, CHANGE);
	attachInterrupt(digitalPinToInterrupt(ROTENC_B_PIN), EncoderISR, CHANGE);
    
    pinMode(ROTENC_SW_PIN, INPUT_PULLDOWN);
    attachInterrupt(ROTENC_SW_PIN, ButtonISR, RISING);
}

bool InputManager::ButtonPressed()
{
    bool result = buttonPressed;
    buttonPressed = false;
    return result;
}

int InputManager::GetRotaryEncoderDelta()
{
    const int16_t encoderPos = rotaryEncoder.readEncoder();
    if (_lastEncoderPos != encoderPos)
    {
        const int16_t delta = (encoderPos - _lastEncoderPos) / 2;
        _lastEncoderPos = _lastEncoderPos + 2 * delta;
        //Serial.println(String("ROT-ENC: ") + delta);
        return delta;
    }
    return 0;
}