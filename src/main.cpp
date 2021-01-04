#include <Arduino.h>
#include <esp_deep_sleep.h>
#include <SPI.h>
#include "RTClock.h"
#include "epd2in9.h"
#include "epdpaint.h"
#include "fonts.h"
#include "Config.h"
#include "SensorManager.h"
#include "InputManager.h"
#include "Display.h"
#include "NumberEditor.h"
#include "TimeEditor.h"

RTClock rtclock;

Display display;
PropertyPage settingsPage(Font12);

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

  rtclock.WakeInOneMinute();
  if (Serial)
  {
    Serial.println(String(F("Going to sleep: ")) + seconds);
    Serial.flush();
  }

  esp_deep_sleep_start();
}

int counter = 0;
int countdown = 0;
unsigned long lastUpdate = 0;
bool update(SensorManager &sm)
{
  auto t = millis();
  int updatesPerSecond = 0;
  if (lastUpdate != 0)
  {
    float deltaT = 0.001f * (t - lastUpdate);
    updatesPerSecond = 1.0 / deltaT;
  }
  lastUpdate = t;

  display.Clear();

  auto btnPressed = InputManager::ButtonPressed();
  auto encoderDelta = InputManager::GetRotaryEncoderDelta();

  if (btnPressed)
    settingsPage.Click();

  if (encoderDelta != 0)
    settingsPage.Scroll(encoderDelta);

  //digitalWrite(PUMP_VCC_PIN, counter == 5 ? HIGH : LOW);

  auto now = rtclock.Now();

  display.RenderMainScreen(sm);

  std::vector<String> lines;
  lines.push_back(String("TICKS_PER_SEC: ") + String(updatesPerSecond));
  lines.push_back(String("SLEEP_IN: ") + String(countdown));
  lines.push_back(String("BAT: ") + sm.GetBatVoltage() + "V");
  lines.push_back(String("DT: ") + now.ToString(true, false));
  lines.push_back(String("TIME: ") + now.ToString(false, true));
  lines.push_back(String("BUTTON: ") + btnPressed);
  lines.push_back(String("ROT_ENC: ") + String(encoderDelta));
  lines.push_back(String("SOIL: ") + String(sm.GetSoilHumidity()));
  lines.push_back(String("TANK: ") + String(sm.GetWaterTankLevel()));
  lines.push_back(String("TEMP: ") + String(sm.GetTemperature()));

  for (size_t i = 0; i < lines.size(); i++)
  {
    //Serial.println(lines[i]);
  }

  display.RenderDebugMessages(lines);

  display.RenderSettingsScreen(settingsPage);

  display.Present();

  return --countdown >= 0;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("starting...");

  print_wakeup_reason();

  InputManager::Init();

  pinMode(PUMP_VCC_PIN, OUTPUT);
  digitalWrite(PUMP_VCC_PIN, LOW);

  //rtclock.SetFromString(String("2010251171600x"));

  delay(10);

  display.Init();

  Serial.println("started");

  countdown = 10000;
  {
    SensorManager sm;

    settingsPage.Add(std::make_shared<NumberEditor>("Humidity", "%", 0, 1, 0.0, 100.0, 40.0, [&](const double val) { Serial.println(val); }));
    settingsPage.Add(std::make_shared<NumberEditor>("Pumping", "s", 1, 0.1, 0.1, 5.0, 0.5));
    settingsPage.Add(std::make_shared<NumberEditor>("Schedule HH", "", 0, 1, 0, 23, 7));
    settingsPage.Add(std::make_shared<NumberEditor>("Schedule MM", "", 0, 1, 0, 59, 0));
    settingsPage.Add(std::make_shared<TimeEditor>("Schedule", 8, 30));

    while (update(sm))
      ;
  }

  deepSleep(90);
}

void loop()
{
}