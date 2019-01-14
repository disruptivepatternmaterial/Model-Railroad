/*
   Nathan end of 2018

   Adafruit Feather HUZZAH ESP8266 pins: #0, #2, #4, #5, #12, #13, #14, #15, #16
   {"type":"sensor","data":{"name":"IS2","state":4}}

   bounce = # of loops to skip before reporting an ON moving to OFF

  this might not really be needed since JMRI can do this.

  I'm not sure if this is just to chatty, if it is, change the loop time in the code or via an MQTT msg

*/


#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi_Info.h"

String payload = "";
int lastreading = 0;
String topicPrefix = "trains/sensors/";
void onConnectionEstablished();
String sensorPrefixJSON = "{\"type\":\"sensor\",\"data\":{\"name\":\"";
String sensorInFixJSON = "\",\"state\":\"";
String sensorSuffixJSON = "\"}}";
int usablePins[] = {14, 12, 13, 4, 5, 2};
//so I have all these complex ideas on how to track the bounces, and in the end I realized I should just make an array...it should have all the position = to the number of pins
int debouncePins[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int loopDelay = 250;
//this is the topic for the inbound config changes
String configTopic = "trains/sensors/ESP/" + WiFi.macAddress();
String topic = "trains/sensors/ESP";
int bounce = 4;



EspMQTTClient client(
  wifi_ssid,               // ssid
  wifi_pass,               // password
  "Punxbergbahn-Infrastruktur.punxbergbahn.lan",         // MQTT ip
  1883,                    // MQTT broker port
  "",                      // MQTT username
  "",                      // MQTT password
  WiFi.macAddress(),       // Client name
  onConnectionEstablished, // Connection established callback
  true,                    // Enable web updater
  true                     // Enable debug messages
);

void setup()
{
  Serial.begin(115200);
  delay(1000);
}


void onConnectionEstablished()
{
  String publishJSON = sensorPrefixJSON + WiFi.macAddress() + sensorInFixJSON + "" + sensorSuffixJSON;
  client.publish(topic, publishJSON);
  client.subscribe(configTopic, [] (const String & payload)
  {
    Serial.println("payload " + payload);
    //loopDelay = payload.toInt();

    const size_t bufferSize = JSON_OBJECT_SIZE(4) + 60;
    DynamicJsonBuffer jsonBuffer(bufferSize);

    //const char* json = "{\"type\":\"sensor\",\"mac\":\"84:F3:EB:0F:26:DD\",\"delay\":500,\"bounce\":\"3\"}";

    JsonObject& root = jsonBuffer.parseObject(payload);

    const String type = root["type"];
    const String mac = root["mac"];

    if (mac == WiFi.macAddress() & type == "sensor") {
      loopDelay = root["delay"];
      bounce = root["bounce"];
      Serial.println("MESSAGE PROCESSED");
    }

  });
}

void loop()
{
  client.loop();
  int arraySize = sizeof(usablePins) / sizeof(usablePins[0]);
  for (int x = 0; x < arraySize; x++) {
    int pinToUse = usablePins[x];
    int reading = digitalRead(pinToUse);
    String topic = topicPrefix + WiFi.macAddress() + "/" + pinToUse;
    //Serial.println(debouncePins[pinToUse]);
    if (reading == 1) {
      if (debouncePins[pinToUse] == 1) {
        String publishJSON = sensorPrefixJSON + WiFi.macAddress() + "." + pinToUse + sensorInFixJSON + "OFF" + sensorSuffixJSON;
        client.publish(topic, publishJSON);
        debouncePins[pinToUse] = 0;
      }
      if (debouncePins[pinToUse] > 0) {
        debouncePins[pinToUse] = debouncePins[pinToUse] - 1;
      }
    } else if (reading == 0) {
      String publishJSON = sensorPrefixJSON + WiFi.macAddress() + "." + pinToUse + sensorInFixJSON + "ON" + sensorSuffixJSON;
      client.publish(topic, publishJSON);
      if (debouncePins[pinToUse] <= bounce) {
        debouncePins[pinToUse] = bounce;
      }
    }
  }
  delay(loopDelay);
}
