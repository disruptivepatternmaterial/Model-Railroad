#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi_Info.h"

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

//String payload = "";
void onConnectionEstablished();
//this is the topic for the inbound config changes
String configTopic = "trains/display/ESP/" + WiFi.macAddress();

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

  Serial.println("OLED FeatherWing test");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  Serial.println("OLED begun");
  Serial.println(WiFi.macAddress());
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  //delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

}

void onConnectionEstablished()
{
  //  String publishJSON = sensorPrefixJSON + WiFi.macAddress() + sensorInFixJSON + "" + sensorSuffixJSON;
  //  client.publish(topic, publishJSON);
  client.subscribe(configTopic, [] (const String &payload)
  {
    //Serial.println("payload " + payload);
    //const size_t bufferSize = JSON_OBJECT_SIZE(6) + 250;
    //DynamicJsonBuffer jsonBuffer(bufferSize);

    //const char* json = "{\"type\":\"display\",\"mac\":\"84:F3:EB:B2:A0:45\",\"f\":1,\"line1\":\"1234567890\",\"line2\":\"1234567890\",\"line3\":\"1234567890\",\"line4\":\"1234567890\"}";

    //JsonObject& root = jsonBuffer.parseObject(payload);
    
    //String type = root["type"]; // "display"
    //String mac = root["mac"]; // "84:F3:EB:0F:26:DD"
    //int fontsize = root["f"];
    //String l1 = root["line1"];
    //Serial.println(l1);
    //String l2 = root["line2"];
    //Serial.println(l2);
   // String l3 = root["line3"];
   // Serial.println(l3);
   // String l4 = root["line4"];
   // Serial.println(l4);
    
  //  if (mac == WiFi.macAddress() & type == "display") {
      display.clearDisplay();
      display.display();
      display.setCursor(0, 0);
      //display.setTextSize(fontsize);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      //display.println(l1);
      //display.println(l2);
      //display.println(l3);
      //display.println(l4);
      display.print(payload);
      display.display(); // actually display all of the above
      Serial.println("MESSAGE PROCESSED");
    //}

  });
}


void loop()
{
  client.loop();
  delay(5);
}
