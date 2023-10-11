#include <ArduinoWiFiServer.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiGratuitous.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>

#include <Adafruit_NeoPixel.h>

#include <ESP8266WebServer.h>

#define LED_PIN    12
#define LED_SWITCH 14

#define LED_COUNT 60

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "esp";
const char* password = "Karolina2137";

const IPAddress staticIP(192, 168, 2, 12);
const IPAddress gateway(192, 168, 2, 1);
const IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);

unsigned long startTime;

int currentDataPoint = 0;
float simple_moving_average_previous = 0;
float random_moving_average_previous = 0;

float (*functionPtrs[10])(); //the array of function pointers
int NUM_FUNCTIONS = 2;

int NUM_Y_VALUES = 17;
float yValues[] = {  0,  7,  10,  9,  7.1,  7.5,  7.4,  12,  15,  10,  0,  3,  3.5,  4,  1,  7,  1};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(LED_SWITCH, OUTPUT);
  digitalWrite(LED_SWITCH, HIGH);

  strip.begin();
  strip.show();

    // initializes the array of function pointers.
  functionPtrs[0] = simple_moving_average;
  functionPtrs[1] = random_moving_average;

  startTime = millis();
  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Łączenie z WiFi...");
  }

  server.on("/lighton", handleLightOn);
  server.on("/lightoff", handleLightOff);
  server.onNotFound(handleRoot);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handleLightOn(){
  digitalWrite(LED_SWITCH, LOW);
  for (int i = 0; i < 10; i++) {
    // Use this line if you want the lightning to spread out among multiple LEDs.
    lightningStrike(random(LED_COUNT));
  }
  turnAllPixelsOff();
}

void handleLightOff(){
  turnAllPixelsOff();
  digitalWrite(LED_SWITCH,HIGH);
}

void handleRoot() {
  String html = R"=====(
    <!DOCTYPE html>
    <html>
    <form action="/lightoff">
    <input type="submit" value="Most down" />
    </form>
    <form action="/lighton">
    <input type="submit" value="Most up" />
    </form>

    </form>
    </body>
    </html>
  )=====";
  
  server.send(200, "text/html", html);
}

void lightningStrike(int pixel) {
  float brightness = callFunction(random(NUM_FUNCTIONS));
  float scaledWhite = abs(brightness*500);
  for (int i = pixel;i<pixel+6;i++){
    strip.setPixelColor(i, strip.Color(scaledWhite, scaledWhite, scaledWhite));
  }
  strip.show();
  delay(random(5, 100));
  currentDataPoint++;
  currentDataPoint = currentDataPoint%NUM_Y_VALUES;
}

void turnAllPixelsOff() {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 0);
  }
  strip.show();
}

float callFunction(int index) {
  return (*functionPtrs[index])(); //calls the function at the index of `index` in the array
}

float simple_moving_average() {
  uint32_t startingValue = currentDataPoint;
  uint32_t endingValue = (currentDataPoint+1)%NUM_Y_VALUES;
  float simple_moving_average_current = simple_moving_average_previous + 
                                  (yValues[startingValue])/NUM_Y_VALUES - 
                                  (yValues[endingValue])/NUM_Y_VALUES;

  simple_moving_average_previous = simple_moving_average_current;
  return simple_moving_average_current;
}

float random_moving_average() {
  float firstValue = random(1, 10);
  float secondValue = random(1, 10);
  float random_moving_average_current = random_moving_average_previous +
                                  firstValue/18 -
                                  secondValue/18;
  random_moving_average_previous = random_moving_average_current;

  return random_moving_average_current;
}

