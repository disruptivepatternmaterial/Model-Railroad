#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi_Info.h"

void onConnectionEstablished();
String configTopic = "trains/sensors/ESP#";
String myPayload;
int sensor1;
int sensor2;
int sensor3;
int sensor4;
int sensor5;
int sensor6;

const  byte clockS88 =  2 ;   // S88 bus clock pin = 2
int clockCounter = 0 ;        // clock tops counter
const  byte PSS88 =  3 ;      // PS signal of bus S88 pin = 3
long loopCounter = 0 ;        // reset proper to ECOS
const  byte dataIn = 0 ;       // input data from another Arduino in
// the S88 chain pin = 0
const  byte dataOut = 1 ;      // output data to another Arduino in
// the string or to the central pin = 1
unsigned  int sensors = 0 ;    // 16-bit buffer for sensors
unsigned  int data = 0xffff ;  // the shift register


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

void onConnectionEstablished()
{
  client.subscribe(configTopic, [] (const String & myPayload)
  {
    const size_t capacity = JSON_OBJECT_SIZE(4) + 100;
    DynamicJsonBuffer jsonBuffer(capacity);
    JsonObject& root = jsonBuffer.parseObject(myPayload);
    if (!root.success()) {
      Serial.println("parseObject() failed");
    } else {
      const String sensorName = root["data"]["name"];
      const String sensorState = root["data"]["state"];
      if (sensorState == "ON") {
        mqttMessageToFakePin(sensorName, 1);
      } else {
        mqttMessageToFakePin(sensorName, 0);
      }
      Serial.println("MESSAGE PROCESSED");
    }
  });
}

void setup ( )  {
  Serial.begin(115200);
  delay(500);
  S88_Setup ( ) ;         // we give the number sensor 8 or 16
}

void loop ( )  {
  client.loop();
  S88_Loop ( ) ;            // loop on the sensors
}

void mqttMessageToFakePin (String sensorName, int sensorState) {
  if (sensorName == "84:F3:EB:0F:26:0D.14") {
    sensor1 = sensorState;
  } else if (sensorName == "84:F3:EB:0F:26:0D.12") {
    sensor2 = sensorState;
  } else if (sensorName == "84:F3:EB:0F:26:0D.13") {
    sensor3 = sensorState;
  } else if (sensorName == "84:F3:EB:0F:26:0D.4") {
    sensor4 = sensorState;
  } else if (sensorName == "84:F3:EB:0F:26:0D.5") {
    sensor5 = sensorState;
  } else if (sensorName == "84:F3:EB:0F:26:0D.2") {
    sensor6 = sensorState;
  }
  Serial.println("Message mapped to a fake pin");
}

// PS signal interrupt routine
// (triggering a new clock cycle)

void PS ( )  {
  clockCounter = 0 ;        // reset the counter
  data = sensors ;          // we empty the buffer of the sensors in the
  // shift register
  sensors = 0 ;             // reset the sensor buffer
  loopCounter ++;         // increment the number of clock ticks
}

// interrupt routine of S88 clock

void clock ( )  {
  digitalWrite ( dataOut , bitRead ( data , clockCounter ) ) ;  // we move 1 bit out
  delayMicroseconds ( 16 ) ;                                    // delay for the offset
  bitWrite ( data , clockCounter , digitalRead ( dataIn ) ) ;   // we move 1 bit in input
  clockCounter = ( clockCounter + 1 )  % 6   ;                  //// need to change this to the number of messagesnbsensors ;       // modulo the number of sensors
  // (8 or 16)
}

// the S88 setup

void S88_Setup ( )  {
  pinMode ( clockS88 ,  INPUT_PULLUP ) ;  // init of the pin for the clock
  attachInterrupt ( 0 , clock , RISING ) ;  // clock on int 0 on pin 2
  pinMode ( PSS88 ,  INPUT_PULLUP ) ;     // init of the PS signal pin
  attachInterrupt ( 1 , PS , RISING ) ;     // PS on int1 on pin 3
  pinMode ( dataIn , INPUT_PULLUP ) ;     // pin 0 = data entry from a
  pinMode ( dataOut ,  OUTPUT ) ;         // pin 1 = output data to the
}

// the loop

void S88_Loop ( )  {
  if  ( loopCounter == 20 )  {
    bitSet ( sensors , 0 ) ;              // reset sensor buffers for ECOS
  }

  if (sensor1 == 1) {
    bitSet ( sensors , 1 ) ;
  } else if (sensor2 == 1) {
    bitSet ( sensors , 2 ) ;
  } else if (sensor3 == 1) {
    bitSet ( sensors , 3 ) ;
  } else if (sensor4 == 1) {
    bitSet ( sensors , 4 ) ;
  } else if (sensor5 == 1) {
    bitSet ( sensors , 5 ) ;
  } else if (sensor6 == 1) {
    bitSet ( sensors , 6 ) ;
  }
}
