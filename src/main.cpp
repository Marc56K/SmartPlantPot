#include <Arduino.h>
#include "AppContext.h"

AppContext ctx;

void setup()
{
    Serial.begin(9600);

    pinMode(PUMP_VCC_PIN, OUTPUT);
    digitalWrite(PUMP_VCC_PIN, LOW);

    ctx.Init();
}

void loop()
{
    ctx.Update();
}