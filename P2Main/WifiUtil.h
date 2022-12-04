#include <WiFiNINA.h>
#include "Arduino.h"

#define MAX_INPUT 256


typedef struct WifiHandler{
  int status;
  int keyIndex;
  char inputBuffer[MAX_INPUT];
  unsigned int bufCounter;

  WiFiServer server;
  char *ssid;
  char *pass;  
}WifiHandler;

void initWifi(WifiHandler wh, Stream &_serial, WiFiServer _wifiServer, char* _ssid, char* _pass);

char* getClientLine(WifiHandler wh, Stream &_serial);
char getClientChar(WifiHandler wh, Stream &_serial);

void printWifiStatus(WifiHandler wh, Stream &_serial);

void flushArray(char* arr, size_t _size);
