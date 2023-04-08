#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <FastLED.h>

#include "config_private.h"

#define CAN_TX 5 // ESP32 -> CAN
#define CAN_RX 4 // CAN -> ESP32
#define CAN_SILENT 21 // S

#define LED_POWER 13
#define LED_DATA 2 // WS2812B

CRGB leds[1];

void tryConnectWifi() {
  leds[0] = CRGB::Blue;
  FastLED.show();

  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    ArduinoOTA.begin();
    leds[0] = CRGB::Green;
    FastLED.show();
    Serial.println("WiFi connected");
    return;
  }

  Serial.println("WiFi connection failed!");
  leds[0] = CRGB::Red;
  FastLED.show();
  delay(5000);
  ESP.restart();
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

  FastLED.addLeds<WS2812B, LED_DATA, GRB>(leds, 1);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  ArduinoOTA.setPassword((const char *)OTA_PASSWORD);

  digitalWrite(LED_POWER, LOW);
  Serial.println("Initialization complete");
  delay(100);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    tryConnectWifi();
  }
  ArduinoOTA.handle();
}
