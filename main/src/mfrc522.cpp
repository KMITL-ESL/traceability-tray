#include "mfrc522.h"

void txData(uint8_t data[], uint8_t length)
{
  Wire.beginTransmission(0x12);
  Wire.write(0x2E);                 // start byte
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

uint8_t rxData(uint8_t data[], uint8_t *length, uint8_t maxLength)
{
  Wire.requestFrom(0x12, maxLength * 2 + 2);
  // In case all of byte stuffing plus start stop byte

  if (Wire.read() != 0x2E)
  { // Start byte
    while (Wire.available())
      Wire.read();
    DEBUG_I2C(F("Start byte not match"));
    return STATUS_I2C_Start_ERR;
    return;
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
    memcpy(data, &buff[2], min(len, maxLength));

  if (len > maxLength)
    DEBUG_I2C(F("Length is too long then max size"));

  if (length != nullptr)
    *length = len;

  return buff[0];
}

uint8_t exeCMD(uint8_t CMD, uint8_t txBuff[], uint8_t txLen, uint8_t rxBuff[], uint8_t *rxLen, uint8_t rxMaxLen)
{
  uint8_t cmd[txLen + 2];
  cmd[0] = CMD;
  cmd[1] = txLen;
  memcpy(cmd + 2, txBuff, txLen);
  txData(cmd, txLen + 2);
  return rxData(rxBuff, rxLen, rxMaxLen);
}

/////////////////////////////////////////////////////////////////////////////////////
// Functions for manipulating the MFRC522
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Initializes the MFRC522 chip.
 */
void MFRC522::PCD_Init()
{
  exeCMD(CMD_PCD_Init, nullptr, 0, nullptr, nullptr, 0);
} // End PCD_Init()

/**
 * Performs a soft reset on the MFRC522 chip and waits for it to be ready again.
 */
void MFRC522::PCD_Reset()
{
  exeCMD(CMD_PCD_Reset, nullptr, 0, nullptr, nullptr, 0);
} // End PCD_Reset()

/**
 * Turns the antenna on by enabling pins TX1 and TX2.
 * After a reset these pins are disabled.
 */
void MFRC522::PCD_AntennaOn()
{
  exeCMD(CMD_PCD_AntennaOn, nullptr, 0, nullptr, nullptr, 0);
} // End PCD_AntennaOn()

/**
 * Turns the antenna off by disabling pins TX1 and TX2.
 */
void MFRC522::PCD_AntennaOff()
{
  exeCMD(CMD_PCD_AntennaOff, nullptr, 0, nullptr, nullptr, 0);
} // End PCD_AntennaOff()

/**
 * Get the current MFRC522 Receiver Gain (RxGain[2:0]) value.
 * See 9.3.3.6 / table 98 in http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 * NOTE: Return value scrubbed with (0x07<<4)=01110000b as RCFfgReg may use reserved bits.
 * 
 * @return Value of the RxGain, scrubbed to the 3 bits used.
 */
byte MFRC522::PCD_GetAntennaGain()
{
  byte gain = 0;
  exeCMD(CMD_PCD_AntennaGain, nullptr, 0, &gain, nullptr, 1);
  return gain;
} // End PCD_GetAntennaGain()

/**
 * Set the MFRC522 Receiver Gain (RxGain) to value specified by given mask.
 * See 9.3.3.6 / table 98 in http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 * NOTE: Given mask is scrubbed with (0x07<<4)=01110000b as RCFfgReg may use reserved bits.
 */
void MFRC522::PCD_SetAntennaGain(byte mask)
{
  exeCMD(CMD_PCD_AntennaGain, &mask, 1, nullptr, nullptr, 0);
} // End PCD_SetAntennaGain()

/**
 * Performs a self-test of the MFRC522
 * See 16.1.1 in http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 * 
 * @return Whether or not the test passed. Or false if no firmware reference is available.
 */
bool MFRC522::PCD_PerformSelfTest()
{
  byte res = 0;
  exeCMD(CMD_PCD_PerformSelfTest, nullptr, 0, &res, nullptr, 1);
  return res;
}