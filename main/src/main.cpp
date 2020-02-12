#include <Arduino.h>
#include <Wire.h>

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  Serial.println("on");
}

void loop()
{
  if (Serial.available())
  {
    char in = Serial.read();
    if (in == 's')
    {
    }
  }
}