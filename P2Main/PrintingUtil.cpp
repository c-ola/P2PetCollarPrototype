#include "Arduino.h"
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

static void smartDelay(unsigned long ms, SoftwareSerial _ss, TinyGPSPlus _gps)
{
  unsigned long start = millis();
  do
  {
    while (_ss.available())
      _gps.encode(_ss.read());
  } while (millis() - start < ms);
}

static void printInt(unsigned long val, bool valid, int len, SoftwareSerial _ss, TinyGPSPlus _gps)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);
  //smartDelay(0, _ss, _gps);
}
static void printFloat(float val, bool valid, int len, int prec, SoftwareSerial _ss, TinyGPSPlus _gps)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(' ');
  }
  //smartDelay(0, _ss, _gps);
}
