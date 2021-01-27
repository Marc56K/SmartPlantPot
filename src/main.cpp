#include <Arduino.h>
#include "AppContext.h"

AppContext ctx;

void setup()
{
    Serial.begin(9600);
    Serial.println("starting...");

    pinMode(PUMP_VCC_PIN, OUTPUT);
    digitalWrite(PUMP_VCC_PIN, LOW);

    ctx.Init();

    Serial.println("started");

    while (true)
    {
        auto &ui = ctx.GetUserInterface();
        ui.HandleInput();
        ui.UpdateDisplay();
        ctx.GetDeepSleepMgr().Update();
    }
}

void loop()
{
}