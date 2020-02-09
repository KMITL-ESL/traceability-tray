#include <Arduino.h>

#include <Wire.h>

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9 // Configurable, see typical pin layout above
#define SS_PIN 10 // Configurable, see typical pin layout above

#define STATUS_Success 0x00
#define STATUS_I2C_Start_ERR 0xD1
#define STATUS_I2C_Stuffing_ERR 0xD2
#define STATUS_I2C_Checksum_ERR 0xD3

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

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

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

void prepareData(uint8_t CMD)
{
  switch (CMD)
  {
  case CMD_Device_Type:
    reqResData[0] = STATUS_Success;
    reqResData[1] = 0x02;
    reqResLen = 2;
    Serial.println("Get Device Type");
    break;
  case CMD_Version:
    reqResData[0] = STATUS_Success;
    reqResData[1] = 0x00;
    reqResLen = 2;
    Serial.println("Get Version");
    break;
  }
}

void writeData(uint8_t CMD, uint8_t *data, uint8_t len)
{
  switch (CMD)
  {
  }
}

void receiveEvent(int n)
{
  if (Wire.read() != 0x2E)
  { // Start byte
    while (Wire.available())
      continue;
    Serial.println("I2C:Start byte not match");
    return;
  }
  uint8_t buff[200];
  uint8_t i = 0;
  uint8_t sum = 0;
  while (Wire.peek() != 0x2E && Wire.available())
  { // loop until stop byte
    buff[i] = Wire.read();
    if (buff[i] == 0x2D)
    { // byte-stuffing
      uint8_t temp = Wire.read();
      if (temp == 0xAE)
        buff[i] = 0x2E;
      else if (temp == 0xAD)
        buff[i] = 0x2D;
      else
        Serial.println("I2C:Byte-stuffing Error");
    }
    sum += buff[i];
    i++;
  }
  Wire.read(); // read stop byte
  if (sum != 0xFF)
  { // sum all data with checksum should be 0xFF
    Serial.println("I2C:Checksum error");
    return;
  }
  reqResLen = 0;
  if (i == 2)
  { // master want to read data
    prepareData(buff[0]);
    return;
  }
  writeData(buff[0], &buff[2], buff[1]);
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