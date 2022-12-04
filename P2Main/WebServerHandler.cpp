#include "WebServerHandler.h"

void WebServerHandler::printWifiStatus() {
  // print the SSID of the network you're attached to:
  serial->print("SSID: ");
  serial->println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  serial->print("IP Address: ");
  serial->println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  serial->print("signal strength (RSSI):");
  serial->print(rssi);
  serial->println(" dBm");
}

WebServerHandler::WebServerHandler(WiFiServer _server, int _status, HardwareSerial& _serial) :
  server(_server),
  status(_status),
  serial(&_serial)
{
}

//returns 1 if succeeded, returns 0 if failed
int WebServerHandler::init(char* ssid, char* pass, char* user) {
  while (!serial);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    serial->println("Communication with WiFi module failed!");
    // don't continue
    return 0;
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    serial->print("Attempting to connect to SSID: ");
    serial->println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:

    if(user[0] == '\0'){
      status = WiFi.begin(ssid, pass);
    }else{
      status = WiFi.beginEnterprise(ssid, user, pass);
    }
    
      

    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();
  printWifiStatus();

}

int WebServerHandler::doWebSiteStuff() {
  // listen for incoming clients
  int rslt = 0;
  WiFiClient client = server.available();
  if (client) {                             // if you get a client,
    serial->println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        serial->write(c);                    // print it out the serial monitor
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
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
            rslt = TURN_ON_LIGHT;            // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
            rslt = MAKE_SOUND;                // GET /L turns the LED off
        }
        if (currentLine.endsWith("GET /G")) {
            rslt = SEND_GPS_DATA;                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
  return rslt;
}
