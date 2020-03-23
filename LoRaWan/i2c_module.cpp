#include "i2c_module.h"

#define nullptr NULL

void txData(uint8_t data[], uint8_t length, int addr = 0x12)
{
    Wire.beginTransmission(addr);
    Serial.print("DEBUG I2C : ");
    Serial.println(addr);
    Wire.write(0x2E); // start byte
    data[length] = 0;
    for (int i = 0; i <= length; i++) // last loop for checksum
    {
        uint8_t now = data[i];
        if (i != length) // not checksum
        {
            data[length] += now; // sum data to checksum
        }
        else
        {
            data[length] = ~data[length]; // invert checksum
            now = data[length];
        }

        // Byte-stuffing
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
    Wire.write(0x2E); // stop byte
    Wire.endTransmission();
}

uint8_t rxData(uint8_t data[], uint8_t *length, uint8_t maxLength, int addr = 0x12)
{
    Wire.requestFrom(addr, maxLength * 2 + 5);
    // In case all of byte stuffing plus start stop byte
    uint32_t start = millis();
    while (!Wire.available())
    {
        if (millis() - start > 100)
        {
            DEBUG_I2C(F("Timeout"));
            return 0;
        }
    }
    if (Wire.read() != 0x2E)
    { // Start byte
        while (Wire.available())
            Wire.read();
        DEBUG_I2C(F("Start byte not match"));
        return STATUS_I2C_Start_ERR;
    }

    uint8_t buff[200];
    uint8_t len = 0;
    uint8_t sum = 0;

    while (Wire.peek() != 0x2E && Wire.available())
    { // loop until stop byte
        buff[len] = Wire.read();
        if (buff[len] == 0x2D)
        { // byte-stuffing
            uint8_t temp = Wire.read();
            if (temp == 0xAE)
                buff[len] = 0x2E;
            else if (temp == 0xAD)
                buff[len] = 0x2D;
            else
            {
                while (Wire.available())
                    Wire.read();

                DEBUG_I2C(F("Byte-stuffing Error"));
                return STATUS_I2C_Stuffing_ERR;
            }
        }
        sum += buff[len];
        len++;
    }

    if (Wire.read() != 0x2E)
    { // stop byte
        while (Wire.available())
            Wire.read();
        DEBUG_I2C(F("Stop byte not match"));
        return STATUS_I2C_Stop_ERR;
    }

    if (sum != 0xFF)
    { // sum all data with checksum should be 0xFF
        DEBUG_I2C(F("Checksum error"));
        return STATUS_I2C_Checksum_ERR;
    }

    if (len > 2)
    {
        len = len - 3; // length not with status, length and checksum
        if (buff[1] != len)
        {
            DEBUG_I2C(F("Length field error"));
            return STATUS_I2C_Length_ERR;
        }
    }
    else if (len == 2) // much have status and checksum
        len = 0;
    else
    {
        DEBUG_I2C(F("Length less error"));
        return STATUS_I2C_Length_ERR;
    }

    if (data != nullptr)
        memcpy(data, buff + 2, min(len, maxLength));

    if (len > maxLength)
        DEBUG_I2C(F("Length is too long then max size"));

    if (length != nullptr)
        *length = len;

    return buff[0];
}

uint8_t exeCMD(uint8_t CMD, uint8_t txBuff[], uint8_t txLen, uint8_t rxBuff[], uint8_t *rxLen, uint8_t rxMaxLen, int addr, unsigned int delay)
{
    uint8_t cmd[txLen + 2];
    cmd[0] = CMD;
    cmd[1] = txLen;
    memcpy(cmd + 2, txBuff, txLen);
    txData(cmd, txLen + 2, addr);

    delayMicroseconds(delay);

    uint8_t buff[rxMaxLen];
    uint8_t st = rxData(buff, rxLen, rxMaxLen, addr);
    memcpy(rxBuff, buff, min(*rxLen, rxMaxLen));

    return st;
}
