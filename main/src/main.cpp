#include <Arduino.h>
#include <Wire.h>
#include "i2c_module.h"

// MFRC522 rfid; // Instance of the class

// MFRC522::MIFARE_Key key;

// // Init array that will store new NUID
// byte nuidPICC[4];

// /**
//  * Helper routine to dump a byte array as hex values to Serial.
//  */
// void printHex(byte *buffer, byte bufferSize)
// {
//   for (byte i = 0; i < bufferSize; i++)
//   {
//     Serial.print(buffer[i] < 0x10 ? " 0" : " ");
//     Serial.print(buffer[i], HEX);
//   }
// }

// /**
//  * Helper routine to dump a byte array as dec values to Serial.
//  */
// void printDec(byte *buffer, byte bufferSize)
// {
//   for (byte i = 0; i < bufferSize; i++)
//   {
//     Serial.print(buffer[i] < 0x10 ? " 0" : " ");
//     Serial.print(buffer[i], DEC);
//   }
// }

#define STATUS_OK 0x00
#define STATUS_WRONG_SIZE 0x01
#define STATUS_UNKOWN_CMD 0x02
#define STATUS_FORBIDDEN_CMD 0x03
#define STATUS_NOT_READY_CMD 0x04

#define CMD_Device_Type 0x00
#define CMD_Version 0x01
#define CMD_Reset 0x02

#define CMD_LPM 0x10
#define CMD_CHIP_ID 0x11
#define CMD_LORAWAN 0x12

#define CMD_DEV_EUI 0x20
#define CMD_APP_EUI 0x21
#define CMD_APP_KEY 0x22
#define CMD_NWK_S_KEY 0x23
#define CMD_APP_S_KEY 0x24
#define CMD_DEV_ADDR 0x25
#define CMD_OTAA 0x26

#define CMD_ADR 0x30
#define CMD_JOIN 0x31
#define CMD_DUTY_CYCLE 0x32
#define CMD_CLASS 0x33
#define CMD_TX_CONFIRMED 0x34
#define CMD_APP_PORT 0x35
#define CMD_CONFIREMED_NB_TRIALS 0x36

#define CMD_SEND 0x40

void setup()
{
  Serial.begin(115200);
  Serial.println("OnOn");
  Wire.begin();

  uint8_t buff[50];
  uint8_t len;

  exeCMD(CMD_CHIP_ID, nullptr, 0, buff, &len, 50, 0x13, 1000);

  for (int i = 0; i < len; i++)
  {
    Serial.print(buff[i]);
  }

  Serial.println();

  // rfid.PCD_Init(); // Init MFRC522

  // for (byte i = 0; i < 6; i++)
  // {
  //   key.keyByte[i] = 0xFF;
  // }

  // Serial.println(F("This code scan the MIFARE Classsic NUID."));
  // Serial.print(F("Using the following key:"));
  // printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}

void loop()
{
  // // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  // if (!rfid.PICC_IsNewCardPresent())
  //   return;

  // // Verify if the NUID has been readed
  // if (!rfid.PICC_ReadCardSerial())
  //   return;

  // Serial.print(F("PICC type: "));
  // MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  // Serial.println(rfid.PICC_GetTypeName(piccType));

  // // Check is the PICC of Classic MIFARE type
  // if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
  //     piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
  //     piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  // {
  //   Serial.println(F("Your tag is not of type MIFARE Classic."));
  //   return;
  // }

  // if (rfid.uid.uidByte[0] != nuidPICC[0] ||
  //     rfid.uid.uidByte[1] != nuidPICC[1] ||
  //     rfid.uid.uidByte[2] != nuidPICC[2] ||
  //     rfid.uid.uidByte[3] != nuidPICC[3])
  // {
  //   Serial.println(F("A new card has been detected."));

  //   // Store NUID into nuidPICC array
  //   for (byte i = 0; i < 4; i++)
  //   {
  //     nuidPICC[i] = rfid.uid.uidByte[i];
  //   }

  //   Serial.println(F("The NUID tag is:"));
  //   Serial.print(F("In hex: "));
  //   printHex(rfid.uid.uidByte, rfid.uid.size);
  //   Serial.println();
  //   Serial.print(F("In dec: "));
  //   printDec(rfid.uid.uidByte, rfid.uid.size);
  //   Serial.println();
  // }
  // else
  //   Serial.println(F("Card read previously."));

  // // Halt PICC
  // rfid.PICC_HaltA();

  // // Stop encryption on PCD
  // rfid.PCD_StopCrypto1();
}