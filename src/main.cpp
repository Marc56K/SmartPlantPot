#include <Arduino.h>
#include <esp_deep_sleep.h>
#include "AppContext.h"

AppContext ctx;

void print_wakeup_reason()
{
    esp_sleep_wakeup_cause_t wakeup_reason;

    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;
    }
}

void deepSleep(int seconds)
{
#define uS_TO_S_FACTOR 1000000ULL
    esp_sleep_enable_ext0_wakeup(ROTENC_SW_PIN, 0);
    //esp_sleep_enable_ext1_wakeup(0x8000, ESP_EXT1_WAKEUP_ALL_LOW); // clock interrupt at pin[15]
    //esp_deep_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * seconds);

    if (Serial)
    {
        Serial.println(String(F("Going to sleep: ")) + seconds);
        Serial.flush();
    }

    esp_deep_sleep_start();
}

int countdown = 0;
bool update()
{
    auto &ui = ctx.GetUserInterface();

    if (ui.HandleInput())
    {
        countdown = 10000;
    }

    ui.UpdateDisplay();

    return --countdown >= 0;
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.println("starting...");

    print_wakeup_reason();

    pinMode(PUMP_VCC_PIN, OUTPUT);
    digitalWrite(PUMP_VCC_PIN, LOW);

    ctx.Init();

    Serial.println("started");

    countdown = 10000;
    while (update())
        ;

    deepSleep(90);
}

void loop()
{
}