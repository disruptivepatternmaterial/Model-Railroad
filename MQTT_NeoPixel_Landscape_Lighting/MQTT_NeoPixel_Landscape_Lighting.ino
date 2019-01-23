#include <SPI.h>
#include <Wire.h>
#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi_Info.h"
#include <Adafruit_NeoPixel.h>

// data pin
#define PIN D6
// led count
#define CNT 8

int running = 0;
String mode;

// Parameter 1 = number of pixels in pixels
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED pixels)

void onConnectionEstablished();
//this is the topic for the inbound config changes
String configTopic = "trains/places/punxberg/grossepunxberg/hut";
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


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(CNT, PIN, NEO_GRB + NEO_KHZ800);

uint32_t fire_color   = pixels.Color ( 80,  35,  00);
uint32_t off_color    = pixels.Color (  0,  0,  0);

///
/// Fire simulator
///
class NeoFire
{
    Adafruit_NeoPixel &pixels;
  public:

    NeoFire(Adafruit_NeoPixel&);
    void Draw();
    void Clear();
    void AddColor(uint8_t position, uint32_t color);
    void SubstractColor(uint8_t position, uint32_t color);
    uint32_t Blend(uint32_t color1, uint32_t color2);
    uint32_t Substract(uint32_t color1, uint32_t color2);
};

///
/// Constructor
///
NeoFire::NeoFire(Adafruit_NeoPixel& n_pixels)
  : pixels (n_pixels)
{
}

///
/// Set all colors
///
void NeoFire::Draw()
{
  Clear();

  for (int i = 0; i < CNT; i++)
  {
    AddColor(i, fire_color);
    int r = random(80);
    uint32_t diff_color = pixels.Color ( r, r / 2, r / 2);
    SubstractColor(i, diff_color);
  }

  pixels.show();
}

///
/// Set color of LED
///
void NeoFire::AddColor(uint8_t position, uint32_t color)
{
  uint32_t blended_color = Blend(pixels.getPixelColor(position), color);
  pixels.setPixelColor(position, blended_color);
}

///
/// Set color of LED
///
void NeoFire::SubstractColor(uint8_t position, uint32_t color)
{
  uint32_t blended_color = Substract(pixels.getPixelColor(position), color);
  pixels.setPixelColor(position, blended_color);
}

///
/// Color blending
///
uint32_t NeoFire::Blend(uint32_t color1, uint32_t color2)
{
  uint8_t r1, g1, b1;
  uint8_t r2, g2, b2;
  uint8_t r3, g3, b3;

  r1 = (uint8_t)(color1 >> 16),
  g1 = (uint8_t)(color1 >>  8),
  b1 = (uint8_t)(color1 >>  0);

  r2 = (uint8_t)(color2 >> 16),
  g2 = (uint8_t)(color2 >>  8),
  b2 = (uint8_t)(color2 >>  0);

  return pixels.Color(constrain(r1 + r2, 0, 255), constrain(g1 + g2, 0, 255), constrain(b1 + b2, 0, 255));
}

///
/// Color blending
///
uint32_t NeoFire::Substract(uint32_t color1, uint32_t color2)
{
  uint8_t r1, g1, b1;
  uint8_t r2, g2, b2;
  uint8_t r3, g3, b3;
  int16_t r, g, b;

  r1 = (uint8_t)(color1 >> 16),
  g1 = (uint8_t)(color1 >>  8),
  b1 = (uint8_t)(color1 >>  0);

  r2 = (uint8_t)(color2 >> 16),
  g2 = (uint8_t)(color2 >>  8),
  b2 = (uint8_t)(color2 >>  0);

  r = (int16_t)r1 - (int16_t)r2;
  g = (int16_t)g1 - (int16_t)g2;
  b = (int16_t)b1 - (int16_t)b2;
  if (r < 0) r = 0;
  if (g < 0) g = 0;
  if (b < 0) b = 0;

  return pixels.Color(r, g, b);
}

///
/// Every LED to black
///
void NeoFire::Clear()
{
  for (uint16_t i = 0; i < pixels.numPixels (); i++)
    pixels.setPixelColor(i, off_color);
}

NeoFire fire(pixels);

///
/// Setup
///
void doFire(String data_mode) {

  if (data_mode == "fireplace") {
    running = 1;
  } else {
    running = 0;
    pixels.show();
  }

  if (running == 1) {
    fire.Draw();
    delay(random(50, 150));
  } else {
    pixels.show();
    fire.Clear();
    Serial.println("CLEARED");
  }

}


void onConnectionEstablished()
{
  client.subscribe(configTopic, [] (const String & myPayload)
  {
    const size_t capacity = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 70;
    DynamicJsonBuffer jsonBuffer(capacity);

    //const char* json = "{\"type\":\"lighting\",\"data\":{\"name\":\"hut\",\"mode\":\"fireplace\",\"state\":1,\"bright\":255}}";

    JsonObject& root = jsonBuffer.parseObject(myPayload);

    const char* type = root["type"]; // "lighting"

    JsonObject& data = root["data"];
    const char* data_name = data["name"]; // "hut"
    const char* data_mode = data["mode"]; // "fireplace"
    int data_state = data["state"]; // 1
    int data_bright = data["bright"]; // 255   mode = data_mode;
    mode = data_mode;
    pixels.setBrightness(data_bright);
    doFire(data_mode);
    Serial.println("MESSAGE PROCESSED");
  });

}

void setup()
{
  Serial.begin(9600);
  pixels.begin();
  pixels.setBrightness(255);
  pixels.show(); // Initialize all pixels to 'off'
  Serial.println("READY");
}

///
/// Main loop
///
void loop()
{
  client.loop();
  doFire(mode);
}
