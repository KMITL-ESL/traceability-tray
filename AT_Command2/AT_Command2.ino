#include "LoRaWan_APP.h"
#include "LoRa_APP.h"
#include "Arduino.h"

/*
 * set LoraWan_RGB to Active,the RGB active in loraWan
 * RGB red means sending;
 * RGB purple means joined done;
 * RGB blue means RxWindow1;
 * RGB yellow means RxWindow2;
 * RGB green means received done;
 */

/* OTAA para*/
uint8_t devEui[] = { 0x00, 0x46, 0xA7, 0xD6, 0x71, 0xF8, 0x20, 0x05 };
uint8_t appEui[] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x02, 0xD1, 0x66 };
uint8_t appKey[] = { 0xCC, 0x94, 0xCA, 0xF8, 0x79, 0x2B, 0xD5, 0xD2, 0xDD, 0x6E, 0x90, 0x84, 0x0D, 0x0F, 0xFD, 0x11 };

/* ABP para*/
uint8_t nwkSKey[] = { 0x28, 0xAE, 0xD2, 0x2B, 0x7E, 0x15, 0x16, 0xA6, 0x09, 0xCF, 0xAB, 0xF7, 0x15, 0x88, 0x4F, 0x3C };
uint8_t appSKey[] = { 0x16, 0x28, 0xAE, 0x2B, 0x7E, 0x15, 0xD2, 0xA6, 0xAB, 0xF7, 0xCF, 0x4F, 0x3C, 0x15, 0x88, 0x09 };
uint32_t devAddr =  ( uint32_t )0x81302399;

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t  loraWanClass = LORAWAN_CLASS;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 15000;

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
uint8_t confirmedNbTrials = 8;

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
    appDataSize = 4;
    appData[0] = 0x00;
    appData[1] = 0x01;
    appData[2] = 0x02;
    appData[3] = 0x03;
}


void setup() {
	boardInitMcu();
	Serial.begin(115200);
	passthroughMode = true;
	enableAt(); 
	getDevParam();
	printDevParam();
	deviceState = DEVICE_STATE_SLEEP;
	LoRaWAN.ifskipjoin();
	deviceState_lora = LORA_INIT;
}

void loop()
{
	if(modeLoraWan)//loraWan mode(defalt;)
	{
	  switch( deviceState )
	  {
	    case DEVICE_STATE_INIT:
	    {
	      //Serial.printf("LoRaWan PassthroughMode start!\r\n");
	      //Serial.printf("please start with command\r\n");
	      getDevParam();
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
	      LoRaWAN.send();
	      deviceState = DEVICE_STATE_SLEEP;
	      break;
	    }
	    case DEVICE_STATE_CYCLE:
	    {
	      // only used while joining
	      txDutyCycleTime = appTxDutyCycle + randr( 0, APP_TX_DUTYCYCLE_RND );
	      deviceState = DEVICE_STATE_SLEEP;
	      LoRaWAN.cycle(txDutyCycleTime);
	      break;
	    }
	    case DEVICE_STATE_SLEEP:
	    {
	      LoRaWAN.sleep();
	      break;
	    }
	    default:
	    {
	      deviceState = DEVICE_STATE_SLEEP;
	      break;
	    }
	  }
	}
	else//lora mode
	{
	  switch( deviceState_lora )
	  {
	    case LORA_INIT:
	    {
	      getDevParam();
	      LoRa.init(Lora_FREQ, Lora_TXPW, Lora_SF);
	      deviceState_lora = MCU_SLEEP;
	      break;
	    }
	    case LORA_SEND:
	    {
	      LoRa.send();
	      deviceState_lora = MCU_SLEEP;
	      break;
	    }
	    case LORA_RECEIVE:
	    {
	      LoRa.receive();
	      deviceState_lora = MCU_SLEEP;
	      break;
	    }
	    case MCU_SLEEP:
	    {
	      LoRa.lowpower();
	      break;
	    }
	    default:
	    {
	      deviceState_lora = MCU_SLEEP;
	      break;
	    }
	  }
	}
}
