#include <Arduino.h>

#include <SoftwareSerial.h>
SoftwareSerial serialDebug(2, 3);

void setup()
{
  serialDebug.begin(115200);
}

void loop()
{
  serialDebug.println("B");
  delay(1000);
}