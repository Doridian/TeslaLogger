#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <FastLED.h>
#include <ACAN_ESP32.h>

#include "config_private.h"

#define CAN_TX GPIO_NUM_5 // ESP32 -> CAN
#define CAN_RX GPIO_NUM_4 // CAN -> ESP32
#define CAN_SILENT 21 // S

#define LED_POWER 13
#define LED_DATA 2 // WS2812B

CRGB leds[1];

static void errorLoop() {
  leds[0] = CRGB::Red;
  FastLED.show();
  while (1) {
    ArduinoOTA.handle();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(CAN_TX, OUTPUT);
  pinMode(CAN_RX, INPUT);
  pinMode(CAN_SILENT, OUTPUT);

  digitalWrite(CAN_SILENT, HIGH);
  digitalWrite(CAN_TX, HIGH);

  pinMode(LED_POWER, OUTPUT);
  pinMode(LED_DATA, OUTPUT);
  digitalWrite(LED_POWER, HIGH);
  digitalWrite(LED_DATA, LOW);

  digitalWrite(LED_POWER, LOW);
  FastLED.addLeds<WS2812B, LED_DATA, GRB>(leds, 1);
  leds[0] = CRGB::Blue;
  FastLED.show();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  ArduinoOTA.setPassword((const char *)OTA_PASSWORD);
  ArduinoOTA.begin();

  ACAN_ESP32_Settings settings(1000 * 1000); // 1 Mbit/s
  settings.mRxPin = CAN_RX;
  settings.mTxPin = CAN_TX;
  settings.mRequestedCANMode = ACAN_ESP32_Settings::ListenOnlyMode;
  const uint32_t errorCode = ACAN_ESP32::can.begin(settings);
  if (errorCode) {
    Serial.print("CAN begin error: 0x");
    Serial.println(errorCode, HEX);
    errorLoop();
  }

  Serial.println("Initialization complete");
  delay(100);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if ((int)leds[0] != (int)CRGB::Green) {
      leds[0] = CRGB::Green;
      FastLED.show();
    }
  } else if ((int)leds[0] != (int)CRGB::Yellow) {
    leds[0] = CRGB::Yellow;
    FastLED.show();
  }
  ArduinoOTA.handle();
}
