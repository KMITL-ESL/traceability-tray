#include "i2c_mfrc522.h"

void txData(uint8_t data[], uint8_t length)
{
  Wire.beginTransmission(0x12);
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

uint8_t rxData(uint8_t data[], uint8_t *length, uint8_t maxLength)
{
  Wire.requestFrom(0x12, maxLength * 2 + 5);
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
    memcpy(data, buff + 2, min(len, maxLength));

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

  uint8_t buff[rxMaxLen];
  uint8_t st = rxData(buff, rxLen, rxMaxLen);
  memcpy(rxBuff, buff, min(*rxLen, rxMaxLen));

  return st;
}

/////////////////////////////////////////////////////////////////////////////////////
// Functions for setting up the Arduino
/////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
MFRC522::MFRC522()
{ // SS is defined in pins_arduino.h, UINT8_MAX means there is no connection from Arduino to MFRC522's reset and power down input
} // End constructor

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
                                              MIFARE_Key *key, ///< Pointer to the Crypto1 key to use (6 bytes)
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
  MFRC522::StatusCode status = (MFRC522::StatusCode)exeCMD(CMD_MIFARE_Value, nullptr, 0, buffer, nullptr, 4);
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
// Support functions
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns a __FlashStringHelper pointer to a status code name.
 * 
 * @return const __FlashStringHelper *
 */
const __FlashStringHelper *MFRC522::GetStatusCodeName(MFRC522::StatusCode code ///< One of the StatusCode enums.
)
{
  switch (code)
  {
  case STATUS_OK:
    return F("Success.");
  case STATUS_ERROR:
    return F("Error in communication.");
  case STATUS_COLLISION:
    return F("Collission detected.");
  case STATUS_TIMEOUT:
    return F("Timeout in communication.");
  case STATUS_NO_ROOM:
    return F("A buffer is not big enough.");
  case STATUS_INTERNAL_ERROR:
    return F("Internal error in the code. Should not happen.");
  case STATUS_INVALID:
    return F("Invalid argument.");
  case STATUS_CRC_WRONG:
    return F("The CRC_A does not match.");
  case STATUS_MIFARE_NACK:
    return F("A MIFARE PICC responded with NAK.");
  default:
    return F("Unknown error");
  }
} // End GetStatusCodeName()

/**
 * Translates the SAK (Select Acknowledge) to a PICC type.
 * 
 * @return PICC_Type
 */
MFRC522::PICC_Type MFRC522::PICC_GetType(byte sak ///< The SAK byte returned from PICC_Select().
)
{
  // http://www.nxp.com/documents/application_note/AN10833.pdf
  // 3.2 Coding of Select Acknowledge (SAK)
  // ignore 8-bit (iso14443 starts with LSBit = bit 1)
  // fixes wrong type for manufacturer Infineon (http://nfc-tools.org/index.php?title=ISO14443A)
  sak &= 0x7F;
  switch (sak)
  {
  case 0x04:
    return PICC_TYPE_NOT_COMPLETE; // UID not complete
  case 0x09:
    return PICC_TYPE_MIFARE_MINI;
  case 0x08:
    return PICC_TYPE_MIFARE_1K;
  case 0x18:
    return PICC_TYPE_MIFARE_4K;
  case 0x00:
    return PICC_TYPE_MIFARE_UL;
  case 0x10:
  case 0x11:
    return PICC_TYPE_MIFARE_PLUS;
  case 0x01:
    return PICC_TYPE_TNP3XXX;
  case 0x20:
    return PICC_TYPE_ISO_14443_4;
  case 0x40:
    return PICC_TYPE_ISO_18092;
  default:
    return PICC_TYPE_UNKNOWN;
  }
} // End PICC_GetType()

/**
 * Returns a __FlashStringHelper pointer to the PICC type name.
 * 
 * @return const __FlashStringHelper *
 */
const __FlashStringHelper *MFRC522::PICC_GetTypeName(PICC_Type piccType ///< One of the PICC_Type enums.
)
{
  switch (piccType)
  {
  case PICC_TYPE_ISO_14443_4:
    return F("PICC compliant with ISO/IEC 14443-4");
  case PICC_TYPE_ISO_18092:
    return F("PICC compliant with ISO/IEC 18092 (NFC)");
  case PICC_TYPE_MIFARE_MINI:
    return F("MIFARE Mini, 320 bytes");
  case PICC_TYPE_MIFARE_1K:
    return F("MIFARE 1KB");
  case PICC_TYPE_MIFARE_4K:
    return F("MIFARE 4KB");
  case PICC_TYPE_MIFARE_UL:
    return F("MIFARE Ultralight or Ultralight C");
  case PICC_TYPE_MIFARE_PLUS:
    return F("MIFARE Plus");
  case PICC_TYPE_MIFARE_DESFIRE:
    return F("MIFARE DESFire");
  case PICC_TYPE_TNP3XXX:
    return F("MIFARE TNP3XXX");
  case PICC_TYPE_NOT_COMPLETE:
    return F("SAK indicates UID is not complete.");
  case PICC_TYPE_UNKNOWN:
  default:
    return F("Unknown type");
  }
} // End PICC_GetTypeName()

/**
 * Dumps debug info about the selected PICC to Serial.
 * On success the PICC is halted after dumping the data.
 * For MIFARE Classic the factory default key of 0xFFFFFFFFFFFF is tried.  
 */
void MFRC522::PICC_DumpToSerial(Uid *uid ///< Pointer to Uid struct returned from a successful PICC_Select().
)
{
  MIFARE_Key key;

  // Dump UID, SAK and Type
  PICC_DumpDetailsToSerial(uid);

  // Dump contents
  PICC_Type piccType = PICC_GetType(uid->sak);
  switch (piccType)
  {
  case PICC_TYPE_MIFARE_MINI:
  case PICC_TYPE_MIFARE_1K:
  case PICC_TYPE_MIFARE_4K:
    // All keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
    for (byte i = 0; i < 6; i++)
    {
      key.keyByte[i] = 0xFF;
    }
    PICC_DumpMifareClassicToSerial(uid, piccType, &key);
    break;

  case PICC_TYPE_MIFARE_UL:
    PICC_DumpMifareUltralightToSerial();
    break;

  case PICC_TYPE_ISO_14443_4:
  case PICC_TYPE_MIFARE_DESFIRE:
  case PICC_TYPE_ISO_18092:
  case PICC_TYPE_MIFARE_PLUS:
  case PICC_TYPE_TNP3XXX:
    Serial.println(F("Dumping memory contents not implemented for that PICC type."));
    break;

  case PICC_TYPE_UNKNOWN:
  case PICC_TYPE_NOT_COMPLETE:
  default:
    break; // No memory dump here
  }

  Serial.println();
  PICC_HaltA(); // Already done if it was a MIFARE Classic PICC.
} // End PICC_DumpToSerial()

/**
 * Dumps card info (UID,SAK,Type) about the selected PICC to Serial.
 */
void MFRC522::PICC_DumpDetailsToSerial(Uid *uid ///< Pointer to Uid struct returned from a successful PICC_Select().
)
{
  // UID
  Serial.print(F("Card UID:"));
  for (byte i = 0; i < uid->size; i++)
  {
    if (uid->uidByte[i] < 0x10)
      Serial.print(F(" 0"));
    else
      Serial.print(F(" "));
    Serial.print(uid->uidByte[i], HEX);
  }
  Serial.println();

  // SAK
  Serial.print(F("Card SAK: "));
  if (uid->sak < 0x10)
    Serial.print(F("0"));
  Serial.println(uid->sak, HEX);

  // (suggested) PICC type
  PICC_Type piccType = PICC_GetType(uid->sak);
  Serial.print(F("PICC type: "));
  Serial.println(PICC_GetTypeName(piccType));
} // End PICC_DumpDetailsToSerial()

/**
 * Dumps memory contents of a MIFARE Classic PICC.
 * On success the PICC is halted after dumping the data.
 */
void MFRC522::PICC_DumpMifareClassicToSerial(Uid *uid,           ///< Pointer to Uid struct returned from a successful PICC_Select().
                                             PICC_Type piccType, ///< One of the PICC_Type enums.
                                             MIFARE_Key *key     ///< Key A used for all sectors.
)
{
  byte no_of_sectors = 0;
  switch (piccType)
  {
  case PICC_TYPE_MIFARE_MINI:
    // Has 5 sectors * 4 blocks/sector * 16 bytes/block = 320 bytes.
    no_of_sectors = 5;
    break;

  case PICC_TYPE_MIFARE_1K:
    // Has 16 sectors * 4 blocks/sector * 16 bytes/block = 1024 bytes.
    no_of_sectors = 16;
    break;

  case PICC_TYPE_MIFARE_4K:
    // Has (32 sectors * 4 blocks/sector + 8 sectors * 16 blocks/sector) * 16 bytes/block = 4096 bytes.
    no_of_sectors = 40;
    break;

  default: // Should not happen. Ignore.
    break;
  }

  // Dump sectors, highest address first.
  if (no_of_sectors)
  {
    Serial.println(F("Sector Block   0  1  2  3   4  5  6  7   8  9 10 11  12 13 14 15  AccessBits"));
    for (int8_t i = no_of_sectors - 1; i >= 0; i--)
    {
      PICC_DumpMifareClassicSectorToSerial(uid, key, i);
    }
  }
  PICC_HaltA(); // Halt the PICC before stopping the encrypted session.
  PCD_StopCrypto1();
} // End PICC_DumpMifareClassicToSerial()

/**
 * Dumps memory contents of a sector of a MIFARE Classic PICC.
 * Uses PCD_Authenticate(), MIFARE_Read() and PCD_StopCrypto1.
 * Always uses PICC_CMD_MF_AUTH_KEY_A because only Key A can always read the sector trailer access bits.
 */
void MFRC522::PICC_DumpMifareClassicSectorToSerial(Uid *uid,        ///< Pointer to Uid struct returned from a successful PICC_Select().
                                                   MIFARE_Key *key, ///< Key A for the sector.
                                                   byte sector      ///< The sector to dump, 0..39.
)
{
  MFRC522::StatusCode status;
  byte firstBlock;      // Address of lowest address to dump actually last block dumped)
  byte no_of_blocks;    // Number of blocks in sector
  bool isSectorTrailer; // Set to true while handling the "last" (ie highest address) in the sector.

  // The access bits are stored in a peculiar fashion.
  // There are four groups:
  //		g[3]	Access bits for the sector trailer, block 3 (for sectors 0-31) or block 15 (for sectors 32-39)
  //		g[2]	Access bits for block 2 (for sectors 0-31) or blocks 10-14 (for sectors 32-39)
  //		g[1]	Access bits for block 1 (for sectors 0-31) or blocks 5-9 (for sectors 32-39)
  //		g[0]	Access bits for block 0 (for sectors 0-31) or blocks 0-4 (for sectors 32-39)
  // Each group has access bits [C1 C2 C3]. In this code C1 is MSB and C3 is LSB.
  // The four CX bits are stored together in a nible cx and an inverted nible cx_.
  byte c1, c2, c3;    // Nibbles
  byte c1_, c2_, c3_; // Inverted nibbles
  bool invertedError; // True if one of the inverted nibbles did not match
  byte g[4];          // Access bits for each of the four groups.
  byte group;         // 0-3 - active group for access bits
  bool firstInGroup;  // True for the first block dumped in the group

  // Determine position and size of sector.
  if (sector < 32)
  { // Sectors 0..31 has 4 blocks each
    no_of_blocks = 4;
    firstBlock = sector * no_of_blocks;
  }
  else if (sector < 40)
  { // Sectors 32-39 has 16 blocks each
    no_of_blocks = 16;
    firstBlock = 128 + (sector - 32) * no_of_blocks;
  }
  else
  { // Illegal input, no MIFARE Classic PICC has more than 40 sectors.
    return;
  }

  // Dump blocks, highest address first.
  byte byteCount;
  byte buffer[18];
  byte blockAddr;
  isSectorTrailer = true;
  invertedError = false; // Avoid "unused variable" warning.
  for (int8_t blockOffset = no_of_blocks - 1; blockOffset >= 0; blockOffset--)
  {
    blockAddr = firstBlock + blockOffset;
    // Sector number - only on first line
    if (isSectorTrailer)
    {
      if (sector < 10)
        Serial.print(F("   ")); // Pad with spaces
      else
        Serial.print(F("  ")); // Pad with spaces
      Serial.print(sector);
      Serial.print(F("   "));
    }
    else
    {
      Serial.print(F("       "));
    }
    // Block number
    if (blockAddr < 10)
      Serial.print(F("   ")); // Pad with spaces
    else
    {
      if (blockAddr < 100)
        Serial.print(F("  ")); // Pad with spaces
      else
        Serial.print(F(" ")); // Pad with spaces
    }
    Serial.print(blockAddr);
    Serial.print(F("  "));
    // Establish encrypted communications before reading the first block
    if (isSectorTrailer)
    {
      status = PCD_Authenticate(PICC_CMD_MF_AUTH_KEY_A, firstBlock, key, uid);
      if (status != STATUS_OK)
      {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(GetStatusCodeName(status));
        return;
      }
    }
    // Read block
    byteCount = sizeof(buffer);
    status = MIFARE_Read(blockAddr, buffer, &byteCount);
    if (status != STATUS_OK)
    {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(GetStatusCodeName(status));
      continue;
    }
    // Dump data
    for (byte index = 0; index < 16; index++)
    {
      if (buffer[index] < 0x10)
        Serial.print(F(" 0"));
      else
        Serial.print(F(" "));
      Serial.print(buffer[index], HEX);
      if ((index % 4) == 3)
      {
        Serial.print(F(" "));
      }
    }
    // Parse sector trailer data
    if (isSectorTrailer)
    {
      c1 = buffer[7] >> 4;
      c2 = buffer[8] & 0xF;
      c3 = buffer[8] >> 4;
      c1_ = buffer[6] & 0xF;
      c2_ = buffer[6] >> 4;
      c3_ = buffer[7] & 0xF;
      invertedError = (c1 != (~c1_ & 0xF)) || (c2 != (~c2_ & 0xF)) || (c3 != (~c3_ & 0xF));
      g[0] = ((c1 & 1) << 2) | ((c2 & 1) << 1) | ((c3 & 1) << 0);
      g[1] = ((c1 & 2) << 1) | ((c2 & 2) << 0) | ((c3 & 2) >> 1);
      g[2] = ((c1 & 4) << 0) | ((c2 & 4) >> 1) | ((c3 & 4) >> 2);
      g[3] = ((c1 & 8) >> 1) | ((c2 & 8) >> 2) | ((c3 & 8) >> 3);
      isSectorTrailer = false;
    }

    // Which access group is this block in?
    if (no_of_blocks == 4)
    {
      group = blockOffset;
      firstInGroup = true;
    }
    else
    {
      group = blockOffset / 5;
      firstInGroup = (group == 3) || (group != (blockOffset + 1) / 5);
    }

    if (firstInGroup)
    {
      // Print access bits
      Serial.print(F(" [ "));
      Serial.print((g[group] >> 2) & 1, DEC);
      Serial.print(F(" "));
      Serial.print((g[group] >> 1) & 1, DEC);
      Serial.print(F(" "));
      Serial.print((g[group] >> 0) & 1, DEC);
      Serial.print(F(" ] "));
      if (invertedError)
      {
        Serial.print(F(" Inverted access bits did not match! "));
      }
    }

    if (group != 3 && (g[group] == 1 || g[group] == 6))
    { // Not a sector trailer, a value block
      int32_t value = (int32_t(buffer[3]) << 24) | (int32_t(buffer[2]) << 16) | (int32_t(buffer[1]) << 8) | int32_t(buffer[0]);
      Serial.print(F(" Value=0x"));
      Serial.print(value, HEX);
      Serial.print(F(" Adr=0x"));
      Serial.print(buffer[12], HEX);
    }
    Serial.println();
  }

  return;
} // End PICC_DumpMifareClassicSectorToSerial()

/**
 * Dumps memory contents of a MIFARE Ultralight PICC.
 */
void MFRC522::PICC_DumpMifareUltralightToSerial()
{
  MFRC522::StatusCode status;
  byte byteCount;
  byte buffer[18];
  byte i;

  Serial.println(F("Page  0  1  2  3"));
  // Try the mpages of the original Ultralight. Ultralight C has more pages.
  for (byte page = 0; page < 16; page += 4)
  { // Read returns data for 4 pages at a time.
    // Read pages
    byteCount = sizeof(buffer);
    status = MIFARE_Read(page, buffer, &byteCount);
    if (status != STATUS_OK)
    {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(GetStatusCodeName(status));
      break;
    }
    // Dump data
    for (byte offset = 0; offset < 4; offset++)
    {
      i = page + offset;
      if (i < 10)
        Serial.print(F("  ")); // Pad with spaces
      else
        Serial.print(F(" ")); // Pad with spaces
      Serial.print(i);
      Serial.print(F("  "));
      for (byte index = 0; index < 4; index++)
      {
        i = 4 * offset + index;
        if (buffer[i] < 0x10)
          Serial.print(F(" 0"));
        else
          Serial.print(F(" "));
        Serial.print(buffer[i], HEX);
      }
      Serial.println();
    }
  }
} // End PICC_DumpMifareUltralightToSerial()

/**
 * Calculates the bit pattern needed for the specified access bits. In the [C1 C2 C3] tuples C1 is MSB (=4) and C3 is LSB (=1).
 */
void MFRC522::MIFARE_SetAccessBits(byte *accessBitBuffer, ///< Pointer to byte 6, 7 and 8 in the sector trailer. Bytes [0..2] will be set.
                                   byte g0,               ///< Access bits [C1 C2 C3] for block 0 (for sectors 0-31) or blocks 0-4 (for sectors 32-39)
                                   byte g1,               ///< Access bits C1 C2 C3] for block 1 (for sectors 0-31) or blocks 5-9 (for sectors 32-39)
                                   byte g2,               ///< Access bits C1 C2 C3] for block 2 (for sectors 0-31) or blocks 10-14 (for sectors 32-39)
                                   byte g3                ///< Access bits C1 C2 C3] for the sector trailer, block 3 (for sectors 0-31) or block 15 (for sectors 32-39)
)
{
  byte c1 = ((g3 & 4) << 1) | ((g2 & 4) << 0) | ((g1 & 4) >> 1) | ((g0 & 4) >> 2);
  byte c2 = ((g3 & 2) << 2) | ((g2 & 2) << 1) | ((g1 & 2) << 0) | ((g0 & 2) >> 1);
  byte c3 = ((g3 & 1) << 3) | ((g2 & 1) << 2) | ((g1 & 1) << 1) | ((g0 & 1) << 0);

  accessBitBuffer[0] = (~c2 & 0xF) << 4 | (~c1 & 0xF);
  accessBitBuffer[1] = c1 << 4 | (~c3 & 0xF);
  accessBitBuffer[2] = c3 << 4 | c2;
} // End MIFARE_SetAccessBits()

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
  uint8_t res = 0;
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
  getUidClient();
  getSakClient();
  memcpy(&uid, &clientUid, sizeof(uid));
  return res;
} // End

MFRC522::StatusCode MFRC522::getUidClient()
{
  return (MFRC522::StatusCode)exeCMD(CMD_DATA_UID, nullptr, 0, clientUid.uidByte, &clientUid.size, 10);
}
MFRC522::StatusCode MFRC522::setUidClient()
{
  return (MFRC522::StatusCode)exeCMD(CMD_DATA_UID, clientUid.uidByte, clientUid.size, nullptr, nullptr, 0);
}
MFRC522::StatusCode MFRC522::getSakClient()
{
  return (MFRC522::StatusCode)exeCMD(CMD_DATA_SAK, nullptr, 0, &clientUid.sak, nullptr, 1);
}
MFRC522::StatusCode MFRC522::setSakClient()
{
  return (MFRC522::StatusCode)exeCMD(CMD_DATA_SAK, &clientUid.sak, 1, nullptr, nullptr, 0);
}
MFRC522::StatusCode MFRC522::getKeyClient()
{
  return (MFRC522::StatusCode)exeCMD(CMD_DATA_KEY, nullptr, 0, clientKey.keyByte, nullptr, MF_KEY_SIZE);
}
MFRC522::StatusCode MFRC522::setKeyClient()
{
  return (MFRC522::StatusCode)exeCMD(CMD_DATA_KEY, clientKey.keyByte, MF_KEY_SIZE, nullptr, nullptr, 0);
}
MFRC522::StatusCode MFRC522::getBlockAddrClient()
{
  return (MFRC522::StatusCode)exeCMD(CMD_DATA_BLK_ADDR, nullptr, 0, &clientBlockAddr, nullptr, 1);
}
MFRC522::StatusCode MFRC522::setBlockAddrClient()
{
  return (MFRC522::StatusCode)exeCMD(CMD_DATA_BLK_ADDR, &clientBlockAddr, 1, nullptr, nullptr, 0);
}

MFRC522::StatusCode MFRC522::checkAndChangeUID(MFRC522::Uid *new_uid)
{
  MFRC522::StatusCode status = STATUS_OK;
  bool willChange = false;
  if (new_uid->size != clientUid.size)
    willChange = true;
  else
  {
    for (uint8_t i = 0; i < clientUid.size; i++)
    {
      if (new_uid->uidByte[i] != clientUid.uidByte[i])
      {
        willChange = true;
        break;
      }
    }
  }
  if (willChange)
  {
    memcpy(clientUid.uidByte, new_uid->uidByte, new_uid->size);
    clientUid.size = new_uid->size;

    status = setUidClient();
  }

  if (status != STATUS_OK)
    return status;

  if (new_uid->sak != clientUid.sak)
  {
    clientUid.sak = new_uid->sak;
    status = setSakClient();
  }

  return status;
}
MFRC522::StatusCode MFRC522::checkAndChangeKey(MFRC522::MIFARE_Key *new_key)
{
  for (uint8_t i = 0; i < MF_KEY_SIZE; i++)
  {
    if (new_key->keyByte[i] != clientKey.keyByte[i])
    {
      memcpy(clientKey.keyByte, new_key->keyByte, MF_KEY_SIZE);

      return setKeyClient();
    }
  }
  return STATUS_OK;
}
MFRC522::StatusCode MFRC522::checkAndChangeBlockAddr(byte new_blockAddr)
{
  if (new_blockAddr != clientBlockAddr)
  {
    clientBlockAddr = new_blockAddr;

    return setBlockAddrClient();
  }
  return STATUS_OK;
}