#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi_Info.h"
//#include "fonts.h"
//#include "font2.h"


/*
   take in more lines and scroll them, and somehow clear them
   maybe make a fotmatted display - weather on the left, train info scrolling up the main part
*/

#define OLED_RESET 16 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire, OLED_RESET);

void onConnectionEstablished();
//this is the topic for the inbound config changes
String configTopic = "trains/display/punxberg";
String myPayload;

EspMQTTClient client(
  wifi_ssid,               // ssid
  wifi_pass,               // password
  "Punxbergbahn-Infrastruktur.punxbergbahn.lan",         // MQTT ip
  1883,                    // MQTT broker port
  "",                      // MQTT username
  "",                      // MQTT password
  WiFi.macAddress(),       // Client name
  onConnectionEstablished, // Connection established callback
  false,                    // Enable web updater
  true                     // Enable debug messages
);

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  display.clearDisplay();
  display.display();
}

void onConnectionEstablished()
{
  client.subscribe(configTopic, [] (const String & myPayload)
  {
    const size_t capacity = JSON_OBJECT_SIZE(5) + 180;
    DynamicJsonBuffer jsonBuffer(capacity);
    JsonObject& root = jsonBuffer.parseObject(myPayload);

    if (!root.success()) {
      Serial.println("parseObject() failed");
    }

    Serial.println(myPayload.length());

    const char* type = root["type"]; // "display"
    const int font = root["font"] || 1; // "display"
    const char* line1 = root["line1"]; // "111111111111111111111111111111"
    const char* line2 = root["line2"]; // "111111111111111111111111111111"
    const char* line3 = root["line3"]; // "111111111111111111111111111111"
    const char* line4 = root["line4"]; // "111111111111111111111111111111"
    writeDisplay(line1, line2, line3, line4, font);
    Serial.println("MESSAGE PROCESSED");
  });
}

void writeDisplay (String line1, String line2, String line3, String line4, int font) {
  display.clearDisplay();
  display.display();
  display.setCursor(0,0);
  display.setTextSize(font);
  display.setTextColor(WHITE);
  display.println(line1);
  display.println(line2);
  display.println(line3);
  display.println(line4);
  display.display();
  return;
}

void loop()
{
  client.loop();
  delay(5);
}
