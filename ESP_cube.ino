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
#define LED_COUNT 12
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGBW + NEO_KHZ800);

void setup() 
{
  WiFiManager wifiManager;

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.setBrightness(10); // Set BRIGHTNESS (max = 255)
  strip.fill(strip.Color(255, 255, 0, 0)); //set yellow
  strip.show();

  //first parameter is name of access point, second is the password
  wifiManager.autoConnect("ESP-Cube");

  MDNS.begin(host);

  httpUpdater.setup(&httpServer);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
}

void loop() 
{
  strip.clear();
  strip.show();
  httpServer.handleClient();
  MDNS.update();

  //green, red, blue, white?
  //colorWipe(strip.Color(255,   0,   0), 50); // Red
  colorWipe(strip.Color(255,   0,   255,  0), 100);
  delay(250);
  colorWipe(strip.Color(0,   0,   0,  0), 100);
  delay(250);
  
  //colorWipe(strip.Color(  0, 255,   0), 50); // Green
  //colorWipe(strip.Color(  0,   0, 255), 50); // Blue
}

void flashLED(int times, int ts)
{
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
