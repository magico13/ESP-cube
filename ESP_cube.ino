#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <Adafruit_NeoPixel.h> //neopixel control

//HTTP Server
const char* host = "esp-cube";

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

//neopixels
#define LED_PIN    2
#define LED_COUNT 4
#define BRIGHTNESS 10
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

//Tapping
#define TAP_IN 0
bool _wasTapped = false;

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

void setup() 
{
  WiFiManager wifiManager;

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TAP_IN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TAP_IN), handleTap, FALLING);

  Serial.begin(115200);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS (max = 255)
  strip.fill(yellow); //set yellow
  strip.show();

  //first parameter is name of access point, second is the password
  wifiManager.autoConnect("ESP-Cube");

  MDNS.begin(host);

  httpUpdater.setup(&httpServer);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);

  strip.fill(red);
  strip.show();
}

void loop() 
{
  httpServer.handleClient();
  MDNS.update();

  if (_wasTapped)
  {
    Serial.println("Was tapped!");
    //flashRandom(150, 6);
    anim_blink(5, 100, blue, off);
    //anim_breathe(3, 3000, 0, 0, 255);
    strip.fill(red);
    strip.show();
    delay(1000);
    anim_blink(1, 250, yellow, red);
    _wasTapped = false;
  }
  
  //green, red, blue, white?
  //colorWipe(strip.Color(255,   0,   0), 50); // green
  // colorWipe(strip.Color(255,   0,   255,  0), 100); //cyan
  // delay(250);
  // colorWipe(strip.Color(0,   0,   0,  0), 100);
  // delay(250);
  
  // colorWipe(strip.Color(  0, 255,   0), 50); // Red
  // colorWipe(strip.Color(  0,   0, 255), 50); // Blue
}



void flashLED(int times, int ts)
{
  Serial.printf("Flashing %d times with %d delay\n", times, ts);
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
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void flashRandom(int wait, uint8_t count) {
  //randomly change the brightness of the strip
  Serial.printf("Flashing %d times with %d delay\n", count, wait);
  for(uint8_t i=0; i<count; i++) {
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

// Blink the LEDs between the given colors, count times, with a delay
void anim_blink(uint8_t count, int wait, uint32_t color, uint32_t secondColor) {
  Serial.printf("Blinking %d times with %d delay\n", count, wait);
  for(uint8_t i=0; i<count; i++) {
    strip.fill(color);
    strip.show();
    delay(wait);
    strip.fill(secondColor);
    strip.show();
    delay(wait);
  }
}

// Slowly ramp up/down the color as if breathing
void anim_breathe(uint8_t count, int length, uint8_t red, uint8_t green, uint8_t blue) {
  Serial.printf("Breathing %d times over %d ms each with color (%d, %d, %d)\n", count, length, red, green, blue);
  //we want to do this as smoothly as possible so lets change it once per ms
  
  int wait = 1; //1ms per step
  int steps = length / wait; 
  for (uint8_t j=0; j<count; j++) {
    //get brighter
    for (int i=0; i<steps; i++) {
      float modifier = sin(i * PI/steps);
      if (modifier <= 0) { modifier = 0; } 
      uint8_t modRed = red * modifier;
      uint8_t modGreen = green * modifier;
      uint8_t modBlue = blue * modifier;
      strip.fill(strip.Color(modRed, modGreen, modBlue));
      strip.show();
      delay(wait);
    }
  }
}