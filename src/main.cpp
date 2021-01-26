#include <Arduino.h>
#include <esp_deep_sleep.h>
#include <SPI.h>
#include "InputManager.h"
#include "AppContext.h"
#include "HomePage.h"
#include "PropertyPage.h"
#include "RTClock.h"
#include "epd2in9.h"
#include "epdpaint.h"
#include "fonts.h"
#include "Config.h"
#include "Display.h"
#include "TextPage.h"

AppContext ctx;

Display display;
std::shared_ptr<HomePage> homePage = std::make_shared<HomePage>(ctx);
std::shared_ptr<PropertyPage> settingsPage = std::make_shared<PropertyPage>();
std::shared_ptr<PropertyPage> wifiPage = std::make_shared<PropertyPage>();
std::shared_ptr<TextPage> debugPage = std::make_shared<TextPage>();

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
    display.Navigator().Click();

  if (encoderDelta != 0)
    display.Navigator().Scroll(encoderDelta);

  //digitalWrite(PUMP_VCC_PIN, counter == 5 ? HIGH : LOW);

  auto now = ctx.Clock().Now();

  std::vector<std::string>& lines = debugPage->Lines();
  lines.clear();
  lines.push_back(std::string("TICKS_PER_SEC: ") + String(updatesPerSecond).c_str());
  lines.push_back(std::string("SLEEP_IN: ") + String(countdown).c_str());
  lines.push_back(std::string("BAT: ") + String(sm.GetBatVoltage()).c_str() + "V");
  lines.push_back(std::string("DT: ") + now.ToString(true, false).c_str());
  lines.push_back(std::string("TIME: ") + now.ToString(false, true).c_str());
  lines.push_back(std::string("BUTTON: ") + String(btnPressed).c_str());
  lines.push_back(std::string("ROT_ENC: ") + String(encoderDelta).c_str());
  lines.push_back(std::string("SOIL: ") + String(sm.GetSoilHumidity()).c_str());
  lines.push_back(std::string("TANK: ") + String(sm.GetWaterTankLevel()).c_str());
  lines.push_back(std::string("TEMP: ") + String(sm.GetTemperature()).c_str());

  display.Render(ctx);

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

    settingsPage->Add(std::make_shared<NumberEditor>("Humidity", "%", 0, 1, 0.0, 100.0, 40.0, [&](const double val) { Serial.println(val); }));
    //settingsPage->Add(std::make_shared<NumberEditor>("Pumping", "s", 1, 0.1, 0.1, 5.0, 0.5));
    settingsPage->Add(std::make_shared<TimeEditor>("Schedule", 8, 30));
    settingsPage->Add(std::make_shared<BoolEditor>("Enabled", false));
    settingsPage->Add(std::make_shared<StringEditor>("Name", "test123foobar"));

    std::vector<std::string> options = { "foo", "bar", "bazz" };
    settingsPage->Add(std::make_shared<OptionEditor>("Options", options, 0));

    wifiPage->Add(std::make_shared<StringEditor>("SSID", "MR 2.4 GHz"));
    wifiPage->Add(std::make_shared<StringEditor>("KEY", "abc123"));

    display.Navigator().AddPage("", homePage);
    display.Navigator().AddPage("Settings", settingsPage);
    display.Navigator().AddPage("WiFi", wifiPage);
    display.Navigator().AddPage("Stats", debugPage);

    while (update(sm))
      ;
  }

  deepSleep(90);
}

void loop()
{
}