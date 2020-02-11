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
      Serial.println("hi hi");
      Wire.beginTransmission(0x12);
      Wire.write(0x2E);
      uint8_t buff[20];
      buff[0] = 0x00;
      buff[1] = 0;
      for (int i = 0; i < 2; i++)
      {
        char now = buff[i];
        if (i != 1)
        {
          buff[1] += now;
        }
        else
        {
          buff[1] = ~buff[1];
          now = buff[1];
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

      Serial.print("Get : ");
      Wire.requestFrom(0x12, 10); // request 6 bytes from slave device #8

      while (Wire.available())
      {                      // slave may send less than requested
        int c = Wire.read(); // receive a byte as character
        if (c != -1)
        {
          Serial.print(c); // print the character
          Serial.print(" ");
        }
      }
      Serial.println();
    }
  }
}