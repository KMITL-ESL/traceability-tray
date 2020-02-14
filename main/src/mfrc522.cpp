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
} // End PCD_PerformSelfTest()

/////////////////////////////////////////////////////////////////////////////////////
// Power control
/////////////////////////////////////////////////////////////////////////////////////

//IMPORTANT NOTE!!!!
//Calling any other function that uses CommandReg will disable soft power down mode !!!
//For more details about power control, refer to the datasheet - page 33 (8.6)

void MFRC522::PCD_SoftPowerDown()
{
  exeCMD(CMD_PCD_SoftPowerDown, nullptr, 0, nullptr, nullptr, 0);
}

void MFRC522::PCD_SoftPowerUp()
{
  exeCMD(CMD_PCD_SoftPowerUp, nullptr, 0, nullptr, nullptr, 0);
}

/////////////////////////////////////////////////////////////////////////////////////
// Functions for communicating with PICCs
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Instructs a PICC in state ACTIVE(*) to go to state HALT.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
MFRC522::StatusCode MFRC522::PICC_HaltA()
{
  return (MFRC522::StatusCode)exeCMD(CMD_PICC_Halt, nullptr, 0, nullptr, nullptr, 0);
} // End PICC_HaltA()

/////////////////////////////////////////////////////////////////////////////////////
// Functions for communicating with MIFARE PICCs
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Executes the MFRC522 MFAuthent command.
 * This command manages MIFARE authentication to enable a secure communication to any MIFARE Mini, MIFARE 1K and MIFARE 4K card.
 * The authentication is described in the MFRC522 datasheet section 10.3.1.9 and http://www.nxp.com/documents/data_sheet/MF1S503x.pdf section 10.1.
 * For use with MIFARE Classic PICCs.
 * The PICC must be selected - ie in state ACTIVE(*) - before calling this function.
 * Remember to call PCD_StopCrypto1() after communicating with the authenticated PICC - otherwise no new communications can start.
 * 
 * All keys are set to FFFFFFFFFFFFh at chip delivery.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise. Probably STATUS_TIMEOUT if you supply the wrong key.
 */
MFRC522::StatusCode MFRC522::PCD_Authenticate(byte command,    ///< PICC_CMD_MF_AUTH_KEY_A or PICC_CMD_MF_AUTH_KEY_B
                                              byte blockAddr,  ///< The block number. See numbering in the comments in the .h file.
                                              MIFARE_Key *key, ///< Pointer to the Crypteo1 key to use (6 bytes)
                                              Uid *uid         ///< Pointer to Uid struct. The first 4 bytes of the UID is used.
)
{
  checkAndChangeUID(uid);
  checkAndChangeKey(key);
  checkAndChangeBlockAddr(blockAddr);
  return (MFRC522::StatusCode)exeCMD(CMD_PCD_Authenticate, &command, 1, nullptr, nullptr, 0);
} // End PCD_Authenticate()

/**
 * Used to exit the PCD from its authenticated state.
 * Remember to call this function after communicating with an authenticated PICC - otherwise no new communications can start.
 */
void MFRC522::PCD_StopCrypto1()
{
  exeCMD(CMD_PCD_StopCrypto, nullptr, 0, nullptr, nullptr, 0);
} // End PCD_StopCrypto1()

/**
 * Reads 16 bytes (+ 2 bytes CRC_A) from the active PICC.
 * 
 * For MIFARE Classic the sector containing the block must be authenticated before calling this function.
 * 
 * For MIFARE Ultralight only addresses 00h to 0Fh are decoded.
 * The MF0ICU1 returns a NAK for higher addresses.
 * The MF0ICU1 responds to the READ command by sending 16 bytes starting from the page address defined by the command argument.
 * For example; if blockAddr is 03h then pages 03h, 04h, 05h, 06h are returned.
 * A roll-back is implemented: If blockAddr is 0Eh, then the contents of pages 0Eh, 0Fh, 00h and 01h are returned.
 * 
 * The buffer must be at least 18 bytes because a CRC_A is also returned.
 * Checks the CRC_A before returning STATUS_OK.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
MFRC522::StatusCode MFRC522::MIFARE_Read(byte blockAddr,  ///< MIFARE Classic: The block (0-0xff) number. MIFARE Ultralight: The first page to return data from.
                                         byte *buffer,    ///< The buffer to store the data in
                                         byte *bufferSize ///< Buffer size, at least 18 bytes. Also number of bytes returned if STATUS_OK.
)
{
  checkAndChangeBlockAddr(blockAddr);
  return (MFRC522::StatusCode)exeCMD(CMD_MIFARE_Read, nullptr, 0, buffer, bufferSize, 16);
} // End MIFARE_Read()

/**
 * Writes 16 bytes to the active PICC.
 * 
 * For MIFARE Classic the sector containing the block must be authenticated before calling this function.
 * 
 * For MIFARE Ultralight the operation is called "COMPATIBILITY WRITE".
 * Even though 16 bytes are transferred to the Ultralight PICC, only the least significant 4 bytes (bytes 0 to 3)
 * are written to the specified address. It is recommended to set the remaining bytes 04h to 0Fh to all logic 0.
 * * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
MFRC522::StatusCode MFRC522::MIFARE_Write(byte blockAddr, ///< MIFARE Classic: The block (0-0xff) number. MIFARE Ultralight: The page (2-15) to write to.
                                          byte *buffer,   ///< The 16 bytes to write to the PICC
                                          byte bufferSize ///< Buffer size, must be at least 16 bytes. Exactly 16 bytes are written.
)
{
  checkAndChangeBlockAddr(blockAddr);
  return (MFRC522::StatusCode)exeCMD(CMD_MIFARE_Write, buffer, bufferSize, nullptr, nullptr, 0);
} // End MIFARE_Write()

/**
 * Writes a 4 byte page to the active MIFARE Ultralight PICC.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
MFRC522::StatusCode MFRC522::MIFARE_Ultralight_Write(byte page,      ///< The page (2-15) to write to.
                                                     byte *buffer,   ///< The 4 bytes to write to the PICC
                                                     byte bufferSize ///< Buffer size, must be at least 4 bytes. Exactly 4 bytes are written.
)
{
  checkAndChangeBlockAddr(page);
  return (MFRC522::StatusCode)exeCMD(CMD_MIFARE_Ultralight_Write, buffer, bufferSize, nullptr, nullptr, 0);
} // End MIFARE_Ultralight_Write()

/**
 * MIFARE Decrement subtracts the delta from the value of the addressed block, and stores the result in a volatile memory.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 * Use MIFARE_Transfer() to store the result in a block.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
MFRC522::StatusCode MFRC522::MIFARE_Decrement(byte blockAddr, ///< The block (0-0xff) number.
                                              int32_t delta   ///< This number is subtracted from the value of block blockAddr.
)
{
  uint8_t buffer[4];
  buffer[0] = (delta >> 24) & 0xFF;
  buffer[1] = (delta >> 16) & 0xFF;
  buffer[2] = (delta >> 8) & 0xFF;
  buffer[3] = delta & 0xFF;
  checkAndChangeBlockAddr(blockAddr);
  return (MFRC522::StatusCode)exeCMD(CMD_MIFARE_Decrement, buffer, 4, nullptr, nullptr, 0);
} // End MIFARE_Decrement()

/**
 * MIFARE Increment adds the delta to the value of the addressed block, and stores the result in a volatile memory.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 * Use MIFARE_Transfer() to store the result in a block.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
MFRC522::StatusCode MFRC522::MIFARE_Increment(byte blockAddr, ///< The block (0-0xff) number.
                                              int32_t delta   ///< This number is added to the value of block blockAddr.
)
{
  uint8_t buffer[4];
  buffer[0] = (delta >> 24) & 0xFF;
  buffer[1] = (delta >> 16) & 0xFF;
  buffer[2] = (delta >> 8) & 0xFF;
  buffer[3] = delta & 0xFF;
  checkAndChangeBlockAddr(blockAddr);
  return (MFRC522::StatusCode)exeCMD(CMD_MIFARE_Increment, buffer, 4, nullptr, nullptr, 0);
} // End MIFARE_Increment()

/**
 * MIFARE Restore copies the value of the addressed block into a volatile memory.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 * Use MIFARE_Transfer() to store the result in a block.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
MFRC522::StatusCode MFRC522::MIFARE_Restore(byte blockAddr ///< The block (0-0xff) number.
)
{
  checkAndChangeBlockAddr(blockAddr);
  return (MFRC522::StatusCode)exeCMD(CMD_MIFARE_Restore, nullptr, 0, nullptr, nullptr, 0);
} // End MIFARE_Restore()

/**
 * MIFARE Transfer writes the value stored in the volatile memory into one MIFARE Classic block.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
MFRC522::StatusCode MFRC522::MIFARE_Transfer(byte blockAddr ///< The block (0-0xff) number.
)
{
  checkAndChangeBlockAddr(blockAddr);
  return (MFRC522::StatusCode)exeCMD(CMD_MIFARE_Transfer, nullptr, 0, nullptr, nullptr, 0);
} // End MIFARE_Transfer()

/**
 * Helper routine to read the current value from a Value Block.
 * 
 * Only for MIFARE Classic and only for blocks in "value block" mode, that
 * is: with access bits [C1 C2 C3] = [110] or [001]. The sector containing
 * the block must be authenticated before calling this function. 
 * 
 * @param[in]   blockAddr   The block (0x00-0xff) number.
 * @param[out]  value       Current value of the Value Block.
 * @return STATUS_OK on success, STATUS_??? otherwise.
  */
MFRC522::StatusCode MFRC522::MIFARE_GetValue(byte blockAddr, int32_t *value)
{
  uint8_t buffer[4];
  checkAndChangeBlockAddr(blockAddr);
  MFRC522::StatusCode status = (MFRC522::StatusCode)exeCMD(CMD_MIFARE_Value, nullptr, 0, buffer, nullptr, 0);
  *value = (int32_t)buffer[0] << 24 | (int32_t)buffer[1] << 16 | (int32_t)buffer[2] << 8 | buffer[3];
  return status;
} // End MIFARE_GetValue()

/**
 * Helper routine to write a specific value into a Value Block.
 * 
 * Only for MIFARE Classic and only for blocks in "value block" mode, that
 * is: with access bits [C1 C2 C3] = [110] or [001]. The sector containing
 * the block must be authenticated before calling this function. 
 * 
 * @param[in]   blockAddr   The block (0x00-0xff) number.
 * @param[in]   value       New value of the Value Block.
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
MFRC522::StatusCode MFRC522::MIFARE_SetValue(byte blockAddr, int32_t value)
{
  uint8_t buffer[4];
  buffer[0] = (value >> 24) & 0xFF;
  buffer[1] = (value >> 16) & 0xFF;
  buffer[2] = (value >> 8) & 0xFF;
  buffer[3] = value & 0xFF;
  checkAndChangeBlockAddr(blockAddr);
  return (MFRC522::StatusCode)exeCMD(CMD_MIFARE_Value, buffer, 4, nullptr, nullptr, 0);
} // End MIFARE_SetValue()

/////////////////////////////////////////////////////////////////////////////////////
// Convenience functions - does not add extra functionality
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns true if a PICC responds to PICC_CMD_REQA.
 * Only "new" cards in state IDLE are invited. Sleeping cards in state HALT are ignored.
 * 
 * @return bool
 */
bool MFRC522::PICC_IsNewCardPresent()
{
  byte res = 0;
  exeCMD(CMD_PICC_IsNewCardPresent, nullptr, 0, &res, nullptr, 1);
  return res;
} // End PICC_IsNewCardPresent()

/**
 * Simple wrapper around PICC_Select.
 * Returns true if a UID could be read.
 * Remember to call PICC_IsNewCardPresent(), PICC_RequestA() or PICC_WakeupA() first.
 * The read UID is available in the class variable uid.
 * 
 * @return bool
 */
bool MFRC522::PICC_ReadCardSerial()
{
  byte res = 0;
  exeCMD(CMD_PICC_ReadCardSerial, nullptr, 0, &res, nullptr, 1);
  return res;
} // End
