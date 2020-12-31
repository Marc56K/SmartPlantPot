#include <Arduino.h>
#include <esp_deep_sleep.h>
#include <SPI.h>
#include "RTClock.h"
#include "AiEsp32RotaryEncoder.h"
#include "epd2in9.h"
#include "epdpaint.h"
#include "fonts.h"
#include "Config.h"
#include "SensorManager.h"
#include "Display.h"

RTClock rtclock;

AiEsp32RotaryEncoder rotaryEncoder(ROTENC_A_PIN, ROTENC_B_PIN, -1, -1);

Display display;

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
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
volatile bool buttonPressed;

void ButtonISR()
{
  buttonPressed = true;
}

bool ButtonPressed()
{
  bool result = buttonPressed;
  buttonPressed = false;
  return result;
}

int countdown = 0;
bool update(SensorManager& sm)
{
  display.Clear();

  auto encoderDelta = rotaryEncoder.encoderChanged();
  if (encoderDelta != 0)
  {
    if (abs(encoderDelta) > 1)
      encoderDelta /= 2;
    counter += encoderDelta;
  }

  digitalWrite(PUMP_VCC_PIN, counter == 5 ? HIGH : LOW);

  auto btnPressed = ButtonPressed();
  if (btnPressed)
  {
    countdown += counter;
  }

  auto now = rtclock.Now();

  display.RenderMainScreen(sm);

  std::vector<String> lines;
  lines.push_back(String("SLEEP_IN: ") + String(countdown));
  lines.push_back(String("BAT: ") + sm.GetBatVoltage() + "V");
  lines.push_back(String("DT: ") + now.ToString(true, false));
  lines.push_back(String("TIME: ") + now.ToString(false, true));  
  lines.push_back(String("BUTTON: ") + btnPressed);
  lines.push_back(String("ROT_ENC: ") + String(counter));
  lines.push_back(String("SOIL: ") + String(sm.GetSoilHumidity()));
  lines.push_back(String("TANK: ") + String(sm.GetWaterTankLevel()));
  lines.push_back(String("TEMP: ") + String(sm.GetTemperature()));

  for (size_t i = 0; i < lines.size(); i++)
  {
    Serial.println(lines[i]);
  }

  display.RenderDebugMessages(lines);

  display.Present();

  return --countdown >= 0;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("starting...");

  print_wakeup_reason();

  rotaryEncoder.begin();
  rotaryEncoder.setup([] { rotaryEncoder.readEncoder_ISR(); });
  rotaryEncoder.setBoundaries(INT16_MIN / 2, INT16_MAX / 2, false);
  pinMode(ROTENC_SW_PIN, INPUT);
  attachInterrupt(ROTENC_SW_PIN, ButtonISR, FALLING);

  pinMode(PUMP_VCC_PIN, OUTPUT);
  digitalWrite(PUMP_VCC_PIN, LOW);

  //rtclock.SetFromString(String("2010251171600x"));

  delay(10);

  display.Init();

  Serial.println("started");

  countdown = 1000;
  {
    SensorManager sm;
    while (update(sm));
  }

  deepSleep(90);
}

void loop()
{
}