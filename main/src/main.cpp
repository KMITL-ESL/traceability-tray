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
      Serial.println("hihi");
      Wire.beginTransmission(0x11);
      Wire.write(0x2E);
      uint8_t buff[20];
      buff[0] = 0x28;
      buff[1] = 5;
      buff[2] = 'h';
      buff[3] = 'e';
      buff[4] = 'l';
      buff[5] = 'l';
      buff[6] = 'o';

      buff[7] = 0;
      for (int i = 0; i < 8; i++)
      {
        char now = buff[i];
        if (i != 7)
        {
          buff[7] += now;
        }
        else
        {
          buff[7] = ~buff[7];
          now = buff[7];
        }
        if (now == 0x2E)
        {
          Wire.write(0x2D);
          Wire.write(0xAE);
        }
        else if (now == 0x2D)
        {
          Wire.write(0x2D);
          Wire.write(0xAD);
        }
        else
        {
          Wire.write(now);
        }
      }
      Wire.write(0x2E);
      Wire.endTransmission();
    }
  }
}