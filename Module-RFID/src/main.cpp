#include <Arduino.h>

#include <Wire.h>

#include <SPI.h>
#include <MFRC522.h>

#ifdef SHOW_DEBUG_CMD
#define DEBUG_CMD(msg)            \
  do                              \
  {                               \
    Serial.print("DEBUG CMD : "); \
    Serial.println(msg);          \
  } while (0)
#else
#define DEBUG_CMD(msg) \
  do                   \
  {                    \
  } while (0)
#endif

#ifdef SHOW_DEBUG_I2C
#define DEBUG_I2C(msg)            \
  do                              \
  {                               \
    Serial.print("DEBUG I2C : "); \
    Serial.println(msg);          \
  } while (0)
#else
#define DEBUG_I2C(msg) \
  do                   \
  {                    \
  } while (0)
#endif

#define CHECK_DATA_SIZE(LEN)               \
  if (len != LEN)                          \
  {                                        \
    setResStatus(STATUS_PARAM_Length_ERR); \
    DEBUG_CMD(F("PARAM_Length_ERR"));      \
    break;                                 \
  }

#define RST_PIN 9 // Configurable, see typical pin layout above
#define SS_PIN 10 // Configurable, see typical pin layout above

#define STATUS_Success 0x00
#define STATUS_I2C_Start_ERR 0xD0
#define STATUS_I2C_Stop_ERR 0xD1
#define STATUS_I2C_Stuffing_ERR 0xD2
#define STATUS_I2C_Checksum_ERR 0xD3
#define STATUS_I2C_Length_ERR 0xD4
#define STATUS_PARAM_Length_ERR 0xF0

#define CMD_Device_Type 0x00
#define CMD_Version 0x01
#define CMD_Reset 0x02

#define CMD_PCD_Init 0x10
#define CMD_PCD_Reset 0x11
#define CMD_PCD_AntennaOn 0x12
#define CMD_PCD_AntennaOff 0x13
#define CMD_PCD_AntennaGain 0x14
#define CMD_PCD_PerformSelfTest 0x15

#define CMD_PCD_SoftPowerDown 0x20
#define CMD_PCD_SoftPowerUp 0x21

#define CMD_PICC_Halt 0x30

#define CMD_PCD_Authenticate 0x40
#define CMD_PCD_StopCrypto 0x41
#define CMD_MIFARE_Read 0x42
#define CMD_MIFARE_Write 0x43
#define CMD_MIFARE_Ultralight_Write 0x44
#define CMD_MIFARE_Decrement 0x45
#define CMD_MIFARE_Increment 0x46
#define CMD_MIFARE_Restore 0x47
#define CMD_MIFARE_Transfer 0x48
#define CMD_MIFARE_Value 0x49
#define CMD_PCD_NTAG216_AUTH 0x4A

#define CMD_StatusCodeName 0x50
#define CMD_PICC_Type 0x51
#define CMD_PICC_TypeName 0x52
#define CMD_MIFARE_SetAccessbits 0x53
#define CMD_MIFARE_OpenUidBackdoor 0x54
#define CMD_MIFARE_SetUid 0x55
#define CMD_MIFARE_UnbrickUidSector 0x56

#define CMD_PICC_IsNewCardPresent 0x60
#define CMD_PICC_ReadCardSerial 0x61
#define CMD_GET_UID 0x62
#define CMD_GET_SAK 0x63
#define CMD_SET_KEY 0x64
#define CMD_SET_BLK_ADDR 0x65

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

byte blockAddr;

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

uint8_t reqResData[200];
uint8_t reqResLen;

void setResStatus(uint8_t status)
{
  reqResData[0] = status;
  reqResLen = 1;
}

void prepareData(uint8_t CMD, uint8_t *data, uint8_t len)
{
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);
  int32_t value;

  switch (CMD)
  {
  /////////////////////////////////////////////////////////////////////////////////////
  // General
  /////////////////////////////////////////////////////////////////////////////////////
  case CMD_Device_Type:
    reqResData[0] = STATUS_Success;
    reqResData[1] = 1;
    reqResData[2] = 0x02;
    reqResLen = 3;
    DEBUG_CMD(F("Get Device Type"));
    break;

  case CMD_Version:
    reqResData[0] = STATUS_Success;
    reqResData[1] = 1;
    reqResData[2] = 0x00;
    reqResLen = 3;
    DEBUG_CMD(F("Get Version"));
    break;

  /////////////////////////////////////////////////////////////////////////////////////
  // Functions for manipulating the MFRC522
  /////////////////////////////////////////////////////////////////////////////////////
  case CMD_PCD_Init:
    mfrc522.PCD_Init();
    setResStatus(STATUS_Success);
    DEBUG_CMD(F("PCD_Init"));
    break;

  case CMD_PCD_Reset:
    mfrc522.PCD_Reset();
    setResStatus(STATUS_Success);
    DEBUG_CMD(F("PCD_Reset"));
    break;

  case CMD_PCD_AntennaOn:
    mfrc522.PCD_AntennaOn();
    setResStatus(STATUS_Success);
    DEBUG_CMD(F("PCD_AntennaOn"));
    break;

  case CMD_PCD_AntennaOff:
    mfrc522.PCD_AntennaOff();
    setResStatus(STATUS_Success);
    DEBUG_CMD(F("PCD_AntennaOff"));
    break;

  case CMD_PCD_AntennaGain:
    switch (len)
    {
    case 0:
      reqResData[0] = STATUS_Success;
      reqResData[1] = 1;
      reqResData[2] = mfrc522.PCD_GetAntennaGain();
      reqResLen = 3;
      DEBUG_CMD(F("PCD_GetAntennaGain"));
      break;
    case 1:
      mfrc522.PCD_SetAntennaGain(data[0]);
      setResStatus(STATUS_Success);
      DEBUG_CMD(F("PCD_SetAntennaGain"));
      break;
    default:
      setResStatus(STATUS_PARAM_Length_ERR);
      DEBUG_CMD(F("PCD_AntennaGain -> PARAM_Length_ERR"));
      break;
    }
    break;

  case CMD_PCD_PerformSelfTest:
    reqResData[0] = STATUS_Success;
    reqResData[1] = 1;
    reqResData[2] = mfrc522.PCD_PerformSelfTest();
    reqResLen = 3;
    DEBUG_CMD(F("PCD_PerformSelfTest"));
    break;

  /////////////////////////////////////////////////////////////////////////////////////
  // Power control functions
  /////////////////////////////////////////////////////////////////////////////////////
  case CMD_PCD_SoftPowerDown:
    mfrc522.PCD_SoftPowerDown();
    setResStatus(STATUS_Success);
    DEBUG_CMD(F("PCD_SoftPowerDown"));
    break;

  case CMD_PCD_SoftPowerUp:
    mfrc522.PCD_SoftPowerUp();
    setResStatus(STATUS_Success);
    DEBUG_CMD(F("PCD_SoftPowerUp"));
    break;

  /////////////////////////////////////////////////////////////////////////////////////
  // Functions for communicating with PICCs
  /////////////////////////////////////////////////////////////////////////////////////
  case CMD_PICC_Halt:
    setResStatus(mfrc522.PICC_HaltA());
    DEBUG_CMD(F("PICC_HaltA"));
    break;

  /////////////////////////////////////////////////////////////////////////////////////
  // Functions for communicating with MIFARE PICCs
  /////////////////////////////////////////////////////////////////////////////////////
  case CMD_PCD_Authenticate:
    // <Command>
    //  <1 byte>
    CHECK_DATA_SIZE(1);
    status = mfrc522.PCD_Authenticate(data[0], blockAddr, &key, &(mfrc522.uid));
    setResStatus(status);
    DEBUG_CMD(F("PCD_Authenticate"));
    break;

  case CMD_PCD_StopCrypto:
    mfrc522.PCD_StopCrypto1();
    setResStatus(STATUS_Success);
    DEBUG_CMD(F("PCD_StopCrypto"));
    break;

  case CMD_MIFARE_Read:
    status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    reqResData[0] = status;
    reqResData[1] = size;
    memcpy(&reqResData[2], buffer, size);
    reqResLen = size + 2;
    DEBUG_CMD(F("MIFARE_Read"));
    break;

  case CMD_MIFARE_Write:
    CHECK_DATA_SIZE(16);
    status = mfrc522.MIFARE_Write(blockAddr, data, len);
    setResStatus(status);
    DEBUG_CMD(F("MIFARE_Write"));
    break;

  case CMD_MIFARE_Ultralight_Write:
    CHECK_DATA_SIZE(4);
    status = mfrc522.MIFARE_Ultralight_Write(blockAddr, data, len);
    setResStatus(status);
    DEBUG_CMD(F("MIFARE_Ultralight_Write"));
    break;

  case CMD_MIFARE_Decrement:
    CHECK_DATA_SIZE(4);
    // use Big-endian
    value = (int32_t)data[0] << 24 | (int32_t)data[1] << 16 | (int32_t)data[2] << 8 | data[3];
    status = mfrc522.MIFARE_Decrement(blockAddr, value);
    setResStatus(status);
    DEBUG_CMD(F("MIFARE_Decrement"));
    break;

  case CMD_MIFARE_Increment:
    CHECK_DATA_SIZE(4);
    // use Big-endian
    value = (int32_t)data[0] << 24 | (int32_t)data[1] << 16 | (int32_t)data[2] << 8 | data[3];
    status = mfrc522.MIFARE_Increment(blockAddr, value);
    setResStatus(status);
    DEBUG_CMD(F("MIFARE_Increment"));
    break;

  case CMD_MIFARE_Restore:
    status = mfrc522.MIFARE_Restore(blockAddr);
    setResStatus(status);
    DEBUG_CMD(F("MIFARE_Restore"));
    break;

  case CMD_MIFARE_Transfer:
    status = mfrc522.MIFARE_Transfer(blockAddr);
    setResStatus(status);
    DEBUG_CMD(F("MIFARE_Transfer"));
    break;

  case CMD_MIFARE_Value:
    switch (len)
    {
    case 0:
      status = mfrc522.MIFARE_GetValue(blockAddr, &value);
      reqResData[0] = status;
      reqResData[1] = 4;
      reqResData[2] = (value >> 24) & 0xFF;
      reqResData[3] = (value >> 16) & 0xFF;
      reqResData[4] = (value >> 8) & 0xFF;
      reqResData[5] = value & 0xFF;
      reqResLen = 2 + 4;
      DEBUG_CMD(F("MIFARE_GetValue"));
      break;
    case 4:
      CHECK_DATA_SIZE(4);
      // use Big-endian
      value = (int32_t)data[0] << 24 | (int32_t)data[1] << 16 | (int32_t)data[2] << 8 | data[3];
      status = mfrc522.MIFARE_SetValue(blockAddr, value);
      setResStatus(status);
      DEBUG_CMD(F("MIFARE_SetValue"));
      break;
    default:
      setResStatus(STATUS_PARAM_Length_ERR);
      DEBUG_CMD(F("MIFARE_SetValue -> PARAM_Length_ERR"));
      break;
    }
    break;

  /////////////////////////////////////////////////////////////////////////////////////
  // Convenience functions
  /////////////////////////////////////////////////////////////////////////////////////
  case CMD_PICC_IsNewCardPresent:
    reqResData[0] = STATUS_Success;
    reqResData[1] = 1;
    reqResData[2] = mfrc522.PICC_IsNewCardPresent();
    reqResLen = 3;
    DEBUG_CMD(F("PICC_IsNewCardPresent"));
    break;

  case CMD_PICC_ReadCardSerial:
    reqResData[0] = STATUS_Success;
    reqResData[1] = 1;
    reqResData[2] = mfrc522.PICC_ReadCardSerial();
    reqResLen = 3;
    DEBUG_CMD(F("PICC_ReadCardSerial"));
    break;

  case CMD_GET_UID:
    reqResData[0] = STATUS_Success;
    reqResData[1] = mfrc522.uid.size;
    memcpy(&reqResData[2], mfrc522.uid.uidByte, mfrc522.uid.size);
    reqResLen = mfrc522.uid.size + 2;
    DEBUG_CMD(F("GET_UID"));
    break;

  case CMD_GET_SAK:
    reqResData[0] = STATUS_Success;
    reqResData[1] = 1;
    reqResData[2] = mfrc522.uid.sak;
    reqResLen = 3;
    DEBUG_CMD(F("GET_UID"));
    break;

  case CMD_SET_KEY:
    CHECK_DATA_SIZE(6);
    for (byte i = 0; i < 6; i++)
    {
      key.keyByte[i] = data[i];
    }
    setResStatus(STATUS_Success);
    DEBUG_CMD(F("SET_KEY"));
    break;

  case CMD_SET_BLK_ADDR:
    CHECK_DATA_SIZE(1);
    blockAddr = data[0];
    setResStatus(STATUS_Success);
    DEBUG_CMD(F("SET_BLK_ADDR"));
    break;
  }
}

void receiveEvent(int n)
{
  if (Wire.read() != 0x2E)
  { // Start byte
    while (Wire.available())
      Wire.read();
    setResStatus(STATUS_I2C_Start_ERR);
    DEBUG_I2C(F("Start byte not match"));
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
        setResStatus(STATUS_I2C_Stuffing_ERR);
        DEBUG_I2C(F("Byte-stuffing Error"));
        return;
      }
    }
    sum += buff[len];
    len++;
  }
  Wire.read(); // read stop byte

  if (sum != 0xFF)
  { // sum all data with checksum should be 0xFF
    setResStatus(STATUS_I2C_Checksum_ERR);
    DEBUG_I2C(F("Checksum error"));
    return;
  }

  if (len > 2)
  {
    len = len - 3; // length not with CMD, length and checksum
    if (buff[1] != len)
    {
      setResStatus(STATUS_I2C_Length_ERR);
      DEBUG_I2C(F("Length field error"));
      return;
    }
  }
  else if (len == 2) // much have CMD and checksum
    len = 0;
  else
  {
    setResStatus(STATUS_I2C_Length_ERR);
    DEBUG_I2C(F("Length less error"));
    return;
  }

  reqResLen = 0;
  prepareData(buff[0], &buff[2], len);
}

void requestEvent()
{
  bool isCHK = 0;
  uint8_t sum = 0;
  Wire.write(0x2E);
  for (uint8_t n = 0; n < reqResLen; n++)
  {
    sum += reqResData[n];
    if (reqResData[n] == 0x2E)
    {
      Wire.write(0x2D);
      Wire.write(0xAE);
    }
    else if (reqResData[n] == 0x2D)
    {
      Wire.write(0x2D);
      Wire.write(0xAD);
    }
    else
    {
      Wire.write(reqResData[n]);
    }

    if (isCHK == 0 && n + 1 == reqResLen)
    { // another one loop for checksum
      isCHK = 1;
      reqResLen++;
      reqResData[n + 1] = ~sum;
    }
  }
  Wire.write(0x2E);
}

void setup()
{
  Serial.begin(2000000); // Initialize serial communications with the PC
  while (!Serial)
    ;                 // Do nothing if no serial port is opened (added for Arduino based on ATMEGA32U4)
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card

  Wire.begin(0x12);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  // Prepare the key (used both as key A and as key B)
  // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
}

void loop()
{
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
    return;

  // Show some details of the PICC (that is: the tag/card)
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
}