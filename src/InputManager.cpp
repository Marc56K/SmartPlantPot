#include "InputManager.h"

QueueHandle_t encoderIsrQueue = xQueueCreate(3, sizeof(bool));
void IRAM_ATTR EncoderISR()
{
    bool val = true;
    xQueueSend(encoderIsrQueue, &val, 0);
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

InputManager::InputManager() :
    _rotaryEncoder(ROTENC_A_PIN, ROTENC_B_PIN, -1, -1),
    _mutex(xSemaphoreCreateMutex())
{
}

InputManager::~InputManager()
{
    bool val = false;
    xQueueSend(encoderIsrQueue, &val, 0);
}

void InputManager::Init()
{        
    _rotaryEncoder.begin();

    attachInterrupt(digitalPinToInterrupt(ROTENC_A_PIN), EncoderISR, CHANGE);
	attachInterrupt(digitalPinToInterrupt(ROTENC_B_PIN), EncoderISR, CHANGE);

    _rotaryEncoder.setBoundaries(INT16_MIN / 2, INT16_MAX / 2, false);
    pinMode(ROTENC_SW_PIN, INPUT_PULLDOWN);
    attachInterrupt(ROTENC_SW_PIN, ButtonISR, RISING);

    xTaskCreatePinnedToCore(
      [](void* p) { ((InputManager*)p)->EncoderIsrTaskProc(); },
      "EncoderISR", /* Name of the task */
      10000,  /* Stack size in words */
      this,  /* Task input parameter */
      1,  /* Priority of the task */
      &_encoderIsrTask,  /* Task handle. */
      0); /* Core where the task should run */
}

void InputManager::EncoderIsrTaskProc()
{
    bool run = true;
    while(run)
    {
        xQueueReceive(encoderIsrQueue, &run, portMAX_DELAY);
        if (run)
        {
            _rotaryEncoder.readEncoder_ISR();
            int16_t encoderPos = _rotaryEncoder.readEncoder();
            if (_lastEncoderPos != encoderPos)
            {
                auto now = millis();
                unsigned long deltaT = now - _lastEncoderTime;
                _lastEncoderTime = now;
                int16_t delta = encoderPos - _lastEncoderPos;
                _lastEncoderPos = encoderPos;
                if (deltaT < 50)
                {
                    xSemaphoreTake(_mutex, portMAX_DELAY);
                    {
                        _encoderDelta += delta;
                    }
                    xSemaphoreGive(_mutex); 
                    Serial.println(String("RAW ENC: ") + deltaT + " - > " + delta);
                }
            }
        }
    }
    vTaskDelete(nullptr);
}

bool InputManager::ButtonPressed()
{
    bool result = buttonPressed;
    buttonPressed = false;
    return result;
}

int InputManager::GetRotaryEncoderDelta()
{
    auto result = 0;
    xSemaphoreTake(_mutex, portMAX_DELAY);
    {
        result = _encoderDelta;
        _encoderDelta = 0;
    }
    xSemaphoreGive(_mutex); 
    return result;
}

unsigned long InputManager::GetLastInputTime()
{
    return std::max(_lastEncoderTime, buttonPressedTime);
}