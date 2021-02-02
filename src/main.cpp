#include <Arduino.h>
#include "AppContext.h"

AppContext ctx;

void setup()
{
    Serial.begin(9600);
    ctx.Init();
}

void loop()
{
    ctx.Update();
}