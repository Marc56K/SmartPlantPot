#include <Arduino.h>
#include <esp_deep_sleep.h>
#include <SPI.h>
#include "RTClock.h"
#include "AiEsp32RotaryEncoder.h"
#include "epd2in9.h"
#include "epdpaint.h"
#include "imagedata.h"
#include "Config.h"

#define COLORED 0
#define UNCOLORED 1

RTClock rtclock;

AiEsp32RotaryEncoder rotaryEncoder(ROTENC_A_PIN, ROTENC_B_PIN, -1, -1);

unsigned char image[EPD_WIDTH * EPD_HEIGHT];
Paint paint(image, EPD_WIDTH, EPD_HEIGHT); // width should be the multiple of 8
Epd epd;

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

double GetBatVoltage()
{
  double val = analogRead(BAT_LEVEL_PIN);
  return 2.22 * 3.3 * val / 4095;
}

int countdown = 0;
bool update()
{
  //paint.SetRotate(ROTATE_90);
  paint.Clear(UNCOLORED);

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

  uint16_t s0 = analogRead(SENSOR_0_VALUE_PIN);
  uint16_t s1 = analogRead(SENSOR_1_VALUE_PIN);

  auto now = rtclock.Now();

  std::vector<String> lines;
  lines.push_back(String("BAT_VOLTAGE: ") + GetBatVoltage());
  lines.push_back(String("DATE: ") + now.ToString(true, false));
  lines.push_back(String("TIME: ") + now.ToString(false, true));
  lines.push_back(String("SLEEP_ETA: ") + String(countdown));
  lines.push_back(String("BOTTON: ") + btnPressed);
  lines.push_back(String("ROT_ENC: ") + String(counter));
  lines.push_back(String("SENSOR_0: ") + String(s0));
  lines.push_back(String("SENSOR_1: ") + String(s1));

  for (size_t i = 0; i < lines.size(); i++)
  {
    String& msg = lines[i];
    Serial.println(msg);
    paint.DrawStringAt(2, 2 + i * 15, msg.c_str(), &Font12, COLORED);
  }

  epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
  epd.DisplayFrame();

  return --countdown >= 0;
}

void setup()
{
  pinMode(SENSORS_CLOCK_VCC_PIN, OUTPUT);
  digitalWrite(SENSORS_CLOCK_VCC_PIN, HIGH);

  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("starting...");

  print_wakeup_reason();

  rotaryEncoder.begin();
  rotaryEncoder.setup([] { rotaryEncoder.readEncoder_ISR(); });
  rotaryEncoder.setBoundaries(INT16_MIN / 2, INT16_MAX / 2, false);
  pinMode(ROTENC_SW_PIN, INPUT);
  attachInterrupt(ROTENC_SW_PIN, ButtonISR, FALLING);

  pinMode(BAT_LEVEL_PIN, INPUT);

  pinMode(SENSOR_0_VALUE_PIN, INPUT);
  pinMode(SENSOR_1_VALUE_PIN, INPUT);

  pinMode(PUMP_VCC_PIN, OUTPUT);
  digitalWrite(PUMP_VCC_PIN, LOW);

  //rtclock.SetFromString(String("2010251171600x"));

  delay(10);

  if (epd.Init(lut_full_update) != 0) {
      Serial.print("e-Paper init failed");
      return;
  }

  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();
  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();

  if (epd.Init(lut_partial_update) != 0)
  {
    Serial.println("e-Paper init failed");
    return;
  }

  Serial.println("started");

  countdown = 120;
  while (update());

  deepSleep(90);
}

void loop()
{
}