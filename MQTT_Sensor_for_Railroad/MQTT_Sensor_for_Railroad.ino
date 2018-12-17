/*
   Nathan end of 2018
   Idea is to make code that can be pushed to any esp 2886
   that auto forms a sensor net for model rail
   the first part is not done:
   have a zero conf mqtt config q that the devices get their infor from to have them act normal
   instead I am just hard coding it all

   other todos
   some kind of debouncing - not realy debouncing, but I have not yet though about how I want the fliping of state to handled
   as of right now these messages are managed by node-red and then go into JMRI since JMRI doenst have a uniform interface
   ead the device type and make a list of input pins (the idea is to make this thing super automatic) right now I am just hard coding them


   Adafruit Feather HUZZAH ESP8266 pins: #0, #2, #4, #5, #12, #13, #14, #15, #16
   {"type":"sensor","data":{"name":"IS2","state":4}}

   this is designed to run every loopDelay ms and report all the sensors on pins in usablePins[]
   I have thought that I should only report changes, but it seems to be more reliable to report all and often, I need to think about this
   which is why debouncePins is there - starting to think through how I only report changes.
   and a third idea is an API to request a read using the q this thing is susbribed to they way I can control the polling from node red
*/


#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi_Info.h"

String payload = "";
int lastreading = 0;
String topicPrefix = "trains/sensors/";
void onConnectionEstablished();
String sensorPrefixJSON = "{\"type\":\"sensor\",\"data\":{\"name\":\"";
String sensorInFixJSON = "\",\"state\":";
String sensorSuffixJSON = "}}";
int usablePins[] = {14};
int debouncePins[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int loopDelay = 250;
String configTopic = "trains/sensors/ESP/" + WiFi.macAddress();
int bounce = 4;



EspMQTTClient client(
  wifi_ssid,               // ssid
  wifi_pass,               // password
  "192.168.20.78",         // MQTT ip
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
    Serial.println(debouncePins[pinToUse]);
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
