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
   {"type":"sensor","data":{"name":"IS2","state":4}}

   this is designed to run every loopDelay ms and report all the sensors on pins in usablePins[]
   I have thought that I should only report changes, but it seems to be more reliable to report all and often, I need to think about this
   which is why debouncePins is there - starting to think through how I only report changes.
   and a third idea is an API to request a read using the q this thing is susbribed to they way I can control the polling from node red
*/


#include <EspMQTTClient.h>
#include <ArduinoJson.h>

String payload = "";
int lastreading = 0;
String topicPrefix = "trains/sensors/";
void onConnectionEstablished();
String sensorPrefixJSON = "{\"type\":\"sensor\",\"data\":{\"name\":\"";
String sensorInFixJSON = "\",\"state\":";
String sensorSuffixJSON = "}}";
int usablePins[] = {14};
int debouncePins[] = {};
int loopDelay = 250;
String configTopic = "trains/sensors/ESP/" + WiFi.macAddress();



EspMQTTClient client(
  "Mer de Glace IoT",      // ssid
  "pencilhill",            // password
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
}


void onConnectionEstablished()
{
  client.subscribe(configTopic, [] (const String & payload)
  {
    Serial.println("payload " + payload);
    //loopDelay = payload.toInt();
    
    const size_t bufferSize = JSON_OBJECT_SIZE(4) + 60;
    DynamicJsonBuffer jsonBuffer(bufferSize);

    //const char* json = "{\"type\":\"sensor\",\"mac\":\"adfads\",\"delay\":444,\"bounce\":\"asdfad\"}";

    JsonObject& root = jsonBuffer.parseObject(payload);
    
    const String type = root["type"]; // "sensor"
    const String mac = root["mac"]; // "adfads"
    //loopDelay = root["delay"]; // 444
    const char* bounce = root["bounce"]; // "asdfad"

    if (mac == WiFi.macAddress() & type == "sensor") {
      loopDelay = root["delay"];
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
    if (reading == 1) {
      String publishJSON = sensorPrefixJSON + WiFi.macAddress() + "." + pinToUse + sensorInFixJSON + "OFF" + sensorSuffixJSON;
      client.publish(topic, publishJSON);
    } else if (reading == 0) {
      String publishJSON = sensorPrefixJSON + WiFi.macAddress() + "." + pinToUse + sensorInFixJSON + "ON" + sensorSuffixJSON;
      client.publish(topic, publishJSON);
    }
  }
  delay(loopDelay);
}
