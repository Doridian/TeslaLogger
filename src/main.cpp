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

#define LED_ERROR 0x010000
#define LED_WIFI_DISCONNECTED 0x010100
#define LED_WIFI_CONNECTED 0x000100
#define LED_INITIALIZING 0x000001

CRGB leds[1];

static void errorLoop() {
  leds[0] = LED_ERROR;
  FastLED.show();
  while (1) {
    ArduinoOTA.handle();
  }
}

static void sedLed(const CRGB &color) {
  if (leds[0] == color) {
    return;
  }
  leds[0] = color;
  FastLED.show();
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
  digitalWrite(LED_POWER, LOW);
  digitalWrite(LED_DATA, LOW);
  delay(100);
  FastLED.addLeds<WS2812B, LED_DATA, GRB>(leds, 1);
  leds[0] = LED_INITIALIZING;
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
    sedLed(LED_WIFI_CONNECTED);
  } else {
    sedLed(LED_WIFI_DISCONNECTED);
  }
  ArduinoOTA.handle();
}
