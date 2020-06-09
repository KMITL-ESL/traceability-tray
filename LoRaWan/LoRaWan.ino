#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Arduino.h>
#include <Wire.h>

#define SHOW_DEBUG_CMD
#define SHOW_DEBUG_I2C

// #include "i2c_module.h"
#include "i2c_mfrc522.h"

MFRC522 rfid; // Instance of the class

MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[4];

/**
* Helper routine to dump a byte array as hex values to Serial.
*/
void printHex(byte *buffer, byte bufferSize)
{
 for (byte i = 0; i < bufferSize; i++)
 {
   Serial.print(buffer[i] < 0x10 ? " 0" : " ");
   Serial.print(buffer[i], HEX);
 }
}

/**
* Helper routine to dump a byte array as dec values to Serial.
*/
void printDec(byte *buffer, byte bufferSize)
{
 for (byte i = 0; i < bufferSize; i++)
 {
   Serial.print(buffer[i] < 0x10 ? " 0" : " ");
   Serial.print(buffer[i], DEC);
 }
}

/*
 * set LoraWan_RGB to Active,the RGB active in loraWan
 * RGB red means sending;
 * RGB purple means joined done;
 * RGB blue means RxWindow1;
 * RGB yellow means RxWindow2;
 * RGB green means received done;
 */

/* OTAA para*/
uint8_t devEui[] = { 0x12, 0x92, 0xD0, 0x31, 0x82, 0x30, 0x12, 0x93 };
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x66, 0x01 };

/* ABP para*/
uint8_t nwkSKey[] = { 0x28, 0xAE, 0xD2, 0x2B, 0x7E, 0x15, 0x16, 0xA6, 0x09, 0xCF, 0xAB, 0xF7, 0x15, 0x88, 0x4F, 0x3C };
uint8_t appSKey[] = { 0x16, 0x28, 0xAE, 0x2B, 0x7E, 0x15, 0xD2, 0xA6, 0xAB, 0xF7, 0xCF, 0x4F, 0x3C, 0x15, 0x88, 0x09 };
uint32_t devAddr =  ( uint32_t )0x81302399;

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t  loraWanClass = LORAWAN_CLASS;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 1000;

/*OTAA or ABP*/
bool overTheAirActivation = LORAWAN_NETMODE;

/*ADR enable*/
bool loraWanAdr = LORAWAN_ADR;

/* set LORAWAN_Net_Reserve ON, the node could save the network info to flash, when node reset not need to join again */
bool keepNet = LORAWAN_NET_RESERVE;

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = LORAWAN_UPLINKMODE;

/* Application port */
uint8_t appPort = 2;
/*!
* Number of trials to transmit the frame, if the LoRaMAC layer did not
* receive an acknowledgment. The MAC performs a datarate adaptation,
* according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
* to the following table:
*
* Transmission nb | Data Rate
* ----------------|-----------
* 1 (first)       | DR
* 2               | DR
* 3               | max(DR-1,0)
* 4               | max(DR-1,0)
* 5               | max(DR-2,0)
* 6               | max(DR-2,0)
* 7               | max(DR-3,0)
* 8               | max(DR-3,0)
*
* Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
* the datarate, in case the LoRaMAC layer did not receive an acknowledgment
*/

#define CHARGE_PIN GPIO1

uint8_t confirmedNbTrials = 8;

void RFIDon() {
  Wire.end();
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW); //SET POWER
  delay(1500);
  Wire.begin();
}

void RFIDoff() {
  pinMode(Vext, ANALOG);
  Wire.end();
}

bool RFIDget() {
  if (!rfid.PICC_IsNewCardPresent()) {
    return false;
  }

  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial()) {
    return false;
  }

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return false;
  }

//  if (rfid.uid.uidByte[0] != nuidPICC[0] ||
//      rfid.uid.uidByte[1] != nuidPICC[1] ||
//      rfid.uid.uidByte[2] != nuidPICC[2] ||
//      rfid.uid.uidByte[3] != nuidPICC[3])
//  {
//    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++)
    {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }

    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();

//  }
//  else
//    Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  return true;
}

/* Prepares the payload of the frame */
static void prepareTxFrame( uint8_t port )
{
	/*appData size is LORAWAN_APP_DATA_MAX_SIZE which is defined in "commissioning.h".
	*appDataSize max value is LORAWAN_APP_DATA_MAX_SIZE.
	*if enabled AT, don't modify LORAWAN_APP_DATA_MAX_SIZE, it may cause system hanging or failure.
	*if disabled AT, LORAWAN_APP_DATA_MAX_SIZE can be modified, the max value is reference to lorawan region and SF.
	*for example, if use REGION_CN470, 
	*the max value for different DR can be found in MaxPayloadOfDatarateCN470 refer to DataratesCN470 and BandwidthsCN470 in "RegionCN470.h".
	*/
//    appDataSize = 4;
//    appData[0] = 0x00;
//    appData[1] = 0x01;
//    appData[2] = 0x02;
//    appData[3] = 0x03;

  delay(2500);

  RFIDon();

  bool canGetCard = false;
  uint32_t st = millis();
  while(!canGetCard) {
    if (st + 1000 < millis()) break;
    canGetCard = RFIDget();
    delay(100);
  }

  RFIDoff();

  bool chargeState = digitalRead(CHARGE_PIN);
  if (canGetCard) {
    Serial.println("Card");
    appDataSize = rfid.uid.size;
    for (byte i = 0; i < appDataSize; i++)
    {
      appData[i] = rfid.uid.uidByte[i];
    }
    appPort = 4 + chargeState;
  } else {
    Serial.println("No Card");
    appDataSize = 1;
    appData[0] = 0xFF;
    appPort = 4 + chargeState;
  }
}

void chargeChange()
{
//  delay(10);
//  if(digitalRead(CHARGE_PIN)==HIGH)
//  {
      deviceState = DEVICE_STATE_SEND;
//  }
}

void setup() {
	boardInitMcu();
	Serial.begin(115200);
#if(AT_SUPPORT)
	enableAt();
#endif
	deviceState = DEVICE_STATE_INIT;
	LoRaWAN.ifskipjoin();

  pinMode(CHARGE_PIN, INPUT);
  attachInterrupt(CHARGE_PIN, chargeChange, BOTH);

  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

  Serial.print(F("RFID : Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}

void loop()
{
	switch( deviceState )
	{
		case DEVICE_STATE_INIT:
		{
#if(AT_SUPPORT)
			getDevParam();
#endif
			printDevParam();
			LoRaWAN.init(loraWanClass,loraWanRegion);
			deviceState = DEVICE_STATE_JOIN;
			break;
		}
		case DEVICE_STATE_JOIN:
		{
			LoRaWAN.join();
			break;
		}
		case DEVICE_STATE_SEND:
		{
      prepareTxFrame( appPort );
      LoRaWAN.send();
			deviceState = DEVICE_STATE_SLEEP; // not use cycle use only interupt
			break;
		}
		case DEVICE_STATE_CYCLE:
		{
			// Schedule next packet transmission
			txDutyCycleTime = appTxDutyCycle + randr( 0, APP_TX_DUTYCYCLE_RND );
			LoRaWAN.cycle(txDutyCycleTime);
			deviceState = DEVICE_STATE_SLEEP;
			break;
		}
		case DEVICE_STATE_SLEEP:
		{
      LoRaWAN.sleep();
			break;
		}
		default:
		{
      deviceState = DEVICE_STATE_INIT;
			break;
		}
	}
}
