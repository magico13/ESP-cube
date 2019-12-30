#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <ArduinoJson.h>          //JSON en/decoding

#include <Adafruit_NeoPixel.h> //neopixel control

//HTTP Server
const char* host = "esp-cube";

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

//neopixels
#define LED_PIN    2
#define LED_COUNT 4
#define BRIGHTNESS 64
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

//Tapping
#define TAP_IN 0
bool _wasTapped = false;
String _tapAction = "{\"animation\": \"breathe\",\"count\": 1,\"length\": 500,\"color\": [0, 0, 255]}";

void ICACHE_RAM_ATTR handleTap() { 
  _wasTapped = true;
}

//colors
uint32_t yellow = strip.Color(255, 255, 0);
uint32_t red    = strip.Color(0, 255, 0);
uint32_t green  = strip.Color(255, 0, 0);
uint32_t blue   = strip.Color(0, 0, 255);
uint32_t cyan   = strip.Color(255, 0, 255);
uint32_t off    = strip.Color(0, 0, 0);

uint32_t baseColor = red;

void setup() 
{
  WiFiManager wifiManager;

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TAP_IN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TAP_IN), handleTap, FALLING);

  Serial.begin(115200);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS (max = 255)
  strip.fill(cyan); //set pending color
  strip.show();

  //first parameter is name of access point, second is the password
  wifiManager.autoConnect("ESP-Cube");

  MDNS.begin(host);

  httpUpdater.setup(&httpServer);
  configure_routing();
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);

  anim_blink(2, 100, green, off);
  reset_base();
}

void loop() 
{
  httpServer.handleClient();
  MDNS.update();

  if (_wasTapped)
  {
    Serial.println("Was tapped!");
    animate_json(_tapAction, false);
    _wasTapped = false;
  }
}

//Converts a uint32_t (like a strip.Color) to 4 bytes (_GRB)
void uint32_to_byte_array(uint32_t src, byte* output)
{
  //read through the bits, one byte at a time, from most to least significant
  for (int by=3; by>=0; by--)
  {
    output[3-by] = 0;
    //8 bits in a byte
    for (int bi=0; bi<8; bi++)
    {
      int val = bitRead(src, bi+(8*by));
      Serial.print(val);
      output[3-by] += val * pow(2, bi);
    }
    Serial.print(" ");
  }
  Serial.println();
}

#pragma region REST Server
//Configure the routes for the REST server
void configure_routing() 
{
    httpServer.on("/", HTTP_GET, []() 
    {
      String page = "<a href=\"http://esp-cube.local/color\">Current Color</a><br><a href=\"http://esp-cube.local/tap\">Current Tap Action</a><br><a href=\"http://esp-cube.local/update\">OTA Update</a>";
      httpServer.send(200, "text/html", page);
    });
    httpServer.on("/color", HTTP_GET, rest_get_color);
    httpServer.on("/color", HTTP_POST, rest_post_color);
    httpServer.on("/color", HTTP_PUT, rest_post_color);

    httpServer.on("/animate", HTTP_POST, rest_post_animate);
    httpServer.on("/animate", HTTP_PUT, rest_post_animate);

    httpServer.on("/tap", HTTP_GET, rest_get_tap_action);
    httpServer.on("/tap", HTTP_POST, rest_post_tap_action);
}

//Returns the current color (currently decimal, intending on RGB later)
void rest_get_color() 
{
  DynamicJsonDocument doc(64);
  byte rgb[4];
  uint32_to_byte_array(baseColor, rgb);
  doc["red"] = rgb[2];
  doc["green"] = rgb[1];
  doc["blue"] = rgb[3];
  doc["raw"] = baseColor;
  String output = ""; 
  serializeJson(doc, output);
  httpServer.send(200, "application/json", output);
}

//Sets the base color
void rest_post_color() 
{
  String body = httpServer.arg("plain");
  Serial.println(body);

  DynamicJsonDocument doc(64);
  DeserializationError error = deserializeJson(doc, body);

  if (error) 
  {
    httpServer.send(400);
  }
  else
  {
    uint32_t color = strip.Color(doc["green"], doc["red"], doc["blue"]); //todo: extract and error handle
    baseColor = color;
    reset_base();
    httpServer.send(200);
  }
}

//Triggers an animation
void rest_post_animate()
{
  String body = httpServer.arg("plain");
  Serial.println(body);

  animate_json(body, true);
}

//Sends the current tap action to the client
void rest_get_tap_action()
{
  httpServer.send(200, "application/json", _tapAction);
}

//Takes in an animation to use when tapping
void rest_post_tap_action()
{
  String body = httpServer.arg("plain");
  Serial.println(body);

  //test the action
  if (animate_json(body, true))
  {
    //store the action
    _tapAction = body;
  }
}
#pragma endregion

#pragma region Animations
bool animate_json(String json, bool isWeb)
{
  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, json);

  if (error) 
  {
    if (isWeb) httpServer.send(400);
    return false;
  }
  else
  {
    //switch on the animation type
    String animType = doc["animation"];
    if (animType == "blink")
    {
      //count, wait, color, secondColor
      uint8_t count = doc["count"];
      int wait = doc["wait"];
      uint8_t red = doc["color"][0];
      uint8_t green = doc["color"][1];
      uint8_t blue = doc["color"][2];
      uint32_t color = strip.Color(green, red, blue);
      red = doc["color2"][0];
      green = doc["color2"][1];
      blue = doc["color2"][2];
      uint32_t color2 = strip.Color(green, red, blue);

      if (isWeb) httpServer.send(200);

      anim_blink(count, wait, color, color2);
    }
    else if (animType == "breathe")
    {
      //count, length, red, green, blue
      uint8_t count = doc["count"];
      int length = doc["length"];
      uint8_t red = doc["color"][0];
      uint8_t green = doc["color"][1];
      uint8_t blue = doc["color"][2];

      if (isWeb) httpServer.send(200);

      anim_breathe(count, length, red, green, blue);
    }
    else
    {
      if (isWeb) httpServer.send(400);
      return false;
    }
    
    reset_base();
  }
  return true;
}

void flashLED(int times, int ts)
{
  Serial.printf("Flashing LED_BUILTIN %d times with %d delay\n", times, ts);
  for (int i=0; i<times; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(ts);
    digitalWrite(LED_BUILTIN, LOW);
    delay(ts);
  }
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) 
{
  for(int i=0; i<strip.numPixels(); i++) 
  { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void flashRandom(int wait, uint8_t count) 
{
  //randomly change the brightness of the strip
  Serial.printf("Flashing %d times with %d delay\n", count, wait);
  for(uint8_t i=0; i<count; i++) 
  {
    // get a random pixel from the list
    //uint8_t j = random(strip.numPixels());
    uint8_t j = random(BRIGHTNESS * 2);
    if (j < 1) { j = 1; }
    //strip.setBrightness(j);
    //strip.setPixelColor(j, 0); //turn it off
    strip.show();
    delay(wait);
  }
  //strip.setBrightness(BRIGHTNESS);
  Serial.printf("Resetting to brightness of %d\n", BRIGHTNESS);
  strip.show();
}

void reset_base()
{
  strip.fill(baseColor);
  strip.show();
}

// Blink the LEDs between the given colors, count times, with a delay
void anim_blink(uint8_t count, int wait, uint32_t color, uint32_t secondColor) 
{
  Serial.printf("Blinking %d times with %d delay\n", count, wait);
  for(uint8_t i=0; i<count; i++) 
  {
    strip.fill(color);
    strip.show();
    delay(wait);
    strip.fill(secondColor);
    strip.show();
    delay(wait);
  }
}

// Slowly ramp up/down the color as if breathing
void anim_breathe(uint8_t count, int length, uint8_t red, uint8_t green, uint8_t blue) 
{
  Serial.printf("Breathing %d times over %d ms each with color (%d, %d, %d)\n", count, length, red, green, blue);
  //we want to do this as smoothly as possible so lets change it once per ms
  
  int wait = 1; //1ms per step
  int steps = length / wait; 
  for (uint8_t j=0; j<count; j++) 
  {
    //get brighter
    for (int i=0; i<steps; i++) 
    {
      float modifier = sin(i * PI/steps);
      if (modifier <= 0) { modifier = 0; } 
      uint8_t modRed = red * modifier;
      uint8_t modGreen = green * modifier;
      uint8_t modBlue = blue * modifier;
      strip.fill(strip.Color(modGreen, modRed, modBlue));
      strip.show();
      delay(wait);
    }
  }
  #pragma endregion
}