#pragma once
#include "Arduino.h"
#include <SPI.h>
#include <WiFiNINA.h>

#define DEFAULT_STATE 1
#define SEND_GPS_DATA 10
#define TURN_ON_LIGHT 11
#define MAKE_SOUND 12


class WebServerHandler{
  public:
    WiFiServer server;
    HardwareSerial* serial;
    int status;
    
    WebServerHandler(WiFiServer _server, int _status, HardwareSerial& _serial);
    int init(char* ssid, char* pass, char* user);
    int doWebSiteStuff();
    void printWifiStatus();
};
