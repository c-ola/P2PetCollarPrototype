/*
  Connecting the sensor to a NodeMCU ESP8266:
  VCC  <-> 3V3
  GND  <-> GND
  SDA  <-> D2
  SCL  <-> D1
  Connecting the sensor to a Arduino UNO:
  VCC  <-> 3V3
  GND  <-> GND
  SDA  <-> A4/SDA
  SCL  <-> A5/SCL
  Connecting the sensor to a Arduino DUE:
  VCC  <-> 3V
  GND  <-> GND
  SDA  <-> D20/SDA
  SCL  <-> D21/SCL
*/
//used example code from
//https://github.com/PeterEmbedded/BH1750FVI/blob/master/examples/BH1750FVI_4Wire/BH1750FVI_4Wire.ino
//https://dronebotworkshop.com/usi57600ng-gps-mo#define SECRET_SSID2 ""dules/
//test 

#include <BH1750FVI.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "arduino_secrets.h"
#include "WifiUtil.h"
#include "string.h"
#include "WebServerHandler.h"
#include "PrintingUtil.cpp"
#include "string.h"

//WIFI
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
char user[] = SECRET_USER;
//WifiHandler* wifiHandler = (WifiHandler*)malloc(sizeof(WifiHandler));
WebServerHandler webServer(WiFiServer(80), WL_IDLE_STATUS, Serial);
int rslt = 0;

//LIGHT
// Create the Lightsensor instance
BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);

const uint8_t LIGHT = 13; //pin that is connected to the light
uint8_t lightState = LOW; //controls if the light is on/off
uint16_t onThreshold = 100; //declares what lux level the light will turn on under
unsigned long lightAccumulator = 0;
unsigned long lightTime = 1000;

//logging
unsigned long logAccumulator = 0;
unsigned long logTime = 500;
const boolean doLogging = true;

//General
unsigned long last, start, deltaTime;
boolean doWifi = true, doLightSensing = true, doHeartBeat = false, doGPS = true;
static const uint32_t baudRate = 57600;

//GPS
static const int RXPin = 7, TXPin = 6;
static const uint32_t GPSBaud = 9600;
float lattitude = 0, longitude = 0;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

//heartbeat sensing stuff
const uint8_t LOP = 6;
const uint8_t LON = 7;
int heartBeatOutput = A5;
int analyzedHearBeatOrSomething = 0;

//sound
#include "pitches.h"

const uint8_t BUZZER = 12; //pin that is connected to the light
unsigned long soundAccumulator = 0;
int melodyIndex = 0;

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, 0
};
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4
};

static void smartDelay2(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void setup()
{
  Serial.begin(baudRate);
  ss.begin(GPSBaud);

  while (!Serial) {
    ;
  }

  if (doWifi) {
    //initWifi(*wifiHandler, Serial, WiFiServer(23), SECRET_SSID, SECRET_PASS);
    webServer.init(SECRET_SSID, SECRET_PASS, SECRET_USER);
  }
  else
    Serial.println("Not setting up a Wifi Server");

  //lightsensor
  if (doLightSensing)
    LightSensor.begin();
  else
    Serial.println("Not detecting light levels");

  //heartbeat setup -- not sure why but this sends an error sometimes
//    pinMode(LOP, INPUT);
//    pinMode(LON, INPUT);

  //setting up the LED & Buzzer
  pinMode(BUZZER, OUTPUT);
  pinMode(LIGHT, OUTPUT);

  //timing
  start = 0;
  last = 0;
  deltaTime = 0;
  Serial.println("Finished Setup...");

}

void logger(uint16_t lux, unsigned long deltaTime) {

  if (logAccumulator > logTime) {


    if(doWifi){
      Serial.print("WiFi Status: ");
      Serial.println(webServer.status);  
    }
    
    if(doLightSensing){
      Serial.print("Light: ");
      Serial.print(lightState);
      Serial.print(", ");
      Serial.println(lux);
    }
    
    if(doGPS){
      Serial.println("GPS Stats: sat, lat, lng, chksm");
      printInt(gps.satellites.value(), gps.satellites.isValid(), 5, ss, gps);
      printFloat(lattitude, gps.location.isValid(), 11, 6, ss, gps);
      printFloat(longitude, gps.location.isValid(), 12, 6, ss, gps);
      printInt(gps.failedChecksum(), true, 9, ss, gps);
      smartDelay2(0);
      Serial.println(); 
    }
    
    if(doHeartBeat){
      Serial.print("Heart Beat Data: ");
      Serial.println(analyzedHearBeatOrSomething);
      Serial.println();
    }

    logAccumulator = 0;
  } else {
    logAccumulator += deltaTime;
  }
}

void soundStuff() {
  int noteDuration = 1000 / noteDurations[melodyIndex];

  tone(BUZZER, melody[melodyIndex++], noteDuration);

  //check for out of bounds
  if (melodyIndex == sizeof(melody) / sizeof(int)) {
    melodyIndex = 0;
    rslt = DEFAULT_STATE;
  }

  int pauseBetweenNotes = noteDuration * 1.30;
  delay(pauseBetweenNotes);

  noTone(BUZZER);
}

void doLight(uint16_t lux, unsigned long deltaTime) {
  if (lux < onThreshold) {
    lightAccumulator += deltaTime;
  } else {
    lightAccumulator = 0;
  }

  if (lightAccumulator > lightTime) {
    lightState = HIGH;
  } else {
    lightState = LOW;
  }
  digitalWrite(LIGHT, lightState);
}

void gatherLocationData() {
  if (gps.location.isValid() && gps.satellites.value() >= 1) {
    lattitude = gps.location.lat();
    longitude = gps.location.lng();
  } else {
  }
  ;
  smartDelay2(1);
}

void heartBeatSensing() {
  analyzedHearBeatOrSomething = analogRead(heartBeatOutput);

//  //for serial plotter
//  Serial.print(0); // To freeze the lower limit
//  Serial.print(" ");
//  Serial.print(1000); // To freeze the upper limit
//  Serial.print(" ");
//
//    Serial.println('!');

    //Serial.println(analyzedHearBeatOrSomething);
  //Wait for a bit to keep serial data from saturating
  delay(1);
}

void loop() {
  start = millis();
  deltaTime = start - last;
  uint16_t lux = LightSensor.GetLightIntensity(); //fetches the light intensity of the sensor

  if(doHeartBeat)
    heartBeatSensing();

  //if(doGPS)
    gatherLocationData();

  if (doLightSensing)
    doLight(lux, deltaTime);

  if (doWifi) {
    //char* someString = getClientLine(*wifiHandler, Serial);
    //webServer.doWebSiteStuff();
    WiFiClient client = webServer.server.available();
    if (client) {                             // if you get a client,
      Serial.println("new client");           // print a message out the serial port
      String currentLine = "";                // make a String to hold incoming data from the client
      while (client.connected()) {            // loop while the client's connected
        if (client.available()) {             // if there's bytes to read from the client,
          char c = client.read();             // read a byte, then
          Serial.write(c);                    // print it out the serial monitor
          if (c == '\n') {                    // if the byte is a newline character

            // if the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() == 0) {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();

              // the content of the HTTP response follows the header:
              client.print("Click <a href=\"/H\">here</a> Turn on the light<br>");
              client.print("Click <a href=\"/L\">here</a> Make a sound<br>");
              client.print("Click <a href=\"/G\">here</a> Get GPS data<br>");
              client.println();

              if (rslt == TURN_ON_LIGHT) {
                client.print("Light is on");
                client.println();
              }
              if (rslt == MAKE_SOUND) {
                client.print("Made a noise");
                client.println();
              }
              if (rslt == SEND_GPS_DATA) {
                //char* gpsOut;
                //sprintf(gpsOut, "%lat: .5f, lng: %.5f", lattitude, longitude);
                //client.println(gpsOut);
                client.print(lattitude);
                client.print(", ");
                client.print(longitude);
                client.println();
              }
              // break out of the while loop:
              break;
            } else {    // if you got a newline, then clear currentLine:
              currentLine = "";
            }
          } else if (c != '\r') {  // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
          }
          if (currentLine.endsWith("GET /H")) {
            rslt = TURN_ON_LIGHT;            // GET /H turns the LED on
          }
          if (currentLine.endsWith("GET /L")) {
            rslt = MAKE_SOUND;                // GET /L turns the LED off
          }
          if (currentLine.endsWith("GET /G")) {
            rslt = SEND_GPS_DATA;                // GET /G turns the LED off
          }

        }
      }
      // close the connection:
      client.stop();
      Serial.println("client disconnected");
    }
  }

  if (rslt == TURN_ON_LIGHT)
    lightState = HIGH;
    
  //turns LED on/off
  digitalWrite(LIGHT, lightState);

  //makes a noise
  if (rslt == MAKE_SOUND)
    soundStuff();

  //prints debugging info
  if (doLogging)
    logger(lux, deltaTime);

  last = start;
}
