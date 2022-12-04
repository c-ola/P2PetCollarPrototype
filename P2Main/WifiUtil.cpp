#pragma once
#include <WiFiNINA.h>
#include "WifiUtil.h"

boolean alreadyConnected;

void initWifi(WifiHandler wh, Stream& _serial, WiFiServer _wifiServer, char* _ssid, char* _pass){
  wh.status = WL_IDLE_STATUS;
  alreadyConnected = false;
  wh.keyIndex = 0;
  wh.bufCounter = 0;
  wh.server = _wifiServer;
  wh.ssid = _ssid;
  wh.pass = _pass;

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    _serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    _serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (wh.status != WL_CONNECTED) {
    _serial.print("Attempting to connect to SSID: ");
    _serial.println(wh.ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    wh.status = WiFi.begin(wh.ssid, wh.pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // start the server:
  wh.server.begin();
  // you're connected now, so print out the status:
  printWifiStatus(wh, _serial);
}

char getClientChar(WifiHandler wh, Stream &_serial){
  // wait for a new client:
  WiFiClient client = wh.server.available();

  // when the client sends the first byte, say hello:
  if (client) {
    if (!alreadyConnected) {
      // clear out the input buffer:
      client.flush();
      _serial.println("We have a new client");
      client.println("Hello, client!");
      alreadyConnected = true;
    }
    if (client.available() > 0) {
      // read the bytes incoming from the client:
      return client.read();
      // echo the bytes back to the client:
      //server.write(thisChar);
      // echo the bytes to the server as well:
      //serial.write(thisChar);
    }
  }
  return '\0';
}

void flushArray(char* arr, size_t _size){
  for(int i = 0; i < _size; i++){
    arr[i] = '\0';    
  }
}

char* getClientLine(WifiHandler wh, Stream &_serial){

  char buf = getClientChar(wh, _serial);

  if(wh.bufCounter == 0){
      flushArray(wh.inputBuffer, MAX_INPUT);    
  }
  if(buf == 8){
    if(wh.bufCounter > 0){
      wh.inputBuffer[wh.bufCounter - 1] = '\0';
      wh.bufCounter--;      
    }
  }
  else if(buf == '\n'){
    wh.inputBuffer[wh.bufCounter] == '\0';
    _serial.println(wh.bufCounter);
    _serial.print("Acquired Input: ");
    _serial.print(wh.inputBuffer);
    _serial.println();
    wh.bufCounter = 0;
    return wh.inputBuffer;
  }else if(buf != '\0' && buf != 'NUL'){//if there is input, add to the string
    wh.inputBuffer[wh.bufCounter++] = buf;
  }

  if(wh.bufCounter == MAX_INPUT){   
    _serial.print("Max input reached. Inputted characters are: ");
    _serial.println(wh.inputBuffer);
    flushArray(wh.inputBuffer, MAX_INPUT);
    wh.bufCounter = 0;
  }
  return NULL;
}

void printWifiStatus(WifiHandler wh, Stream &_serial) {
  // print the SSID of the network you're attached to:
  _serial.print("SSID: ");
  _serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  _serial.print("IP Address: ");
  _serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  _serial.print("signal strength (RSSI):");
  _serial.print(rssi);
  _serial.println(" dBm");
}
