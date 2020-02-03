#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Arduino.h>
#include <Wire.h>

#include "onEvent.h"

#define CMD_DTYPE 0x00
#define CMD_VER 0x01
#define CMD_RESET 0x02

#define CMD_APPEUI 0x10
#define CMD_APPKEY 0x11
#define CMD_APPSKEY 0x12
#define CMD_DADDR 0x13
#define CMD_DEUI 0x14
#define CMD_NWKID 0x15
#define CMD_NWKSKEY 0x16

#define CMD_CFM 0x20
#define CMD_CFS 0x21
#define CMD_JOIN 0x22
#define CMD_NJM 0x23
#define CMD_NJS 0x24
#define CMD_RECVP 0x25
#define CMD_RECVD 0x26
#define CMD_SENP 0x27
#define CMD_SEND 0x28

#define CMD_ADR 0x30
#define CMD_CLASS 0x31
#define CMD_DCS 0x32
#define CMD_DR 0x33
#define CMD_JN1DL 0x34
#define CMD_JN2DL 0x35
#define CMD_PNM 0x36
#define CMD_RX1DL 0x37
#define CMD_RX2DL 0x38
#define CMD_RX2DR 0x39
#define CMD_RX2FQ 0x3A
#define CMD_TXP 0x3B

static const PROGMEM u1_t NWKSKEY[16] = {0x28, 0xAE, 0xD2, 0x2B, 0x7E, 0x15, 0x16, 0xA6, 0x09, 0xCF, 0xAB, 0xF7, 0x15, 0x88, 0x4F, 0x3C};
static const u1_t PROGMEM APPSKEY[16] = {0x16, 0x28, 0xAE, 0x2B, 0x7E, 0x15, 0xD2, 0xA6, 0xAB, 0xF7, 0xCF, 0x4F, 0x3C, 0x15, 0x88, 0x09};
static const u4_t DEVADDR = 0x81302399; // <-- Change this address for every node!

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10, // chip select
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9, // reset pin
    .dio = {2, 3, 4},
    .rxtx_rx_active = 0,
    .rssi_cal = 10,
};

uint8_t reqResData[100];
uint8_t reqResLen;

void prepareData(uint8_t CMD)
{
  switch (CMD)
  {
  }
}

void writeData(uint8_t CMD, uint8_t *data, uint8_t len)
{
  switch (CMD)
  {
  case CMD_SEND:
    if (LMIC.opmode & OP_TXRXPEND)
    {
      Serial.println(F("OP_TXRXPEND, not sending"));
      reqResData[0] = 0x04;
      reqResLen = 1;
    }
    else
    {
      // Prepare upstream data transmission at the next possible time.
      LMIC_setTxData2(1, data, len, 0);
      Serial.println(F("Packet queued"));
      reqResData[0] = 0x00;
      reqResLen = 1;
    }
    break;
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
  while (Wire.peek() != 0x2E)
  { // loop untill stop byte
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
  { // sum all data with checksum shoud be 0xFF
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
  while (!Serial)
    ; // wait for Serial to be initialized
  Serial.begin(115200);

  Wire.begin(0x11);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  delay(100); // per sample code on RF_95 test
  Serial.println(F("Starting"));

  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

// Set static session parameters. Instead of dynamically establishing a session
// by joining the network, precomputed session parameters are be provided.
#ifdef PROGMEM
  // On AVR, these values are stored in flash and only copied to RAM
  // once. Copy them to a temporary buffer here, LMIC_setSession will
  // copy them into a buffer of its own again.
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession(0x1, DEVADDR, nwkskey, appskey);
#else
  // If not running an AVR with PROGMEM, just use the arrays directly
  LMIC_setSession(0x13, DEVADDR, NWKSKEY, APPSKEY);
#endif

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink
  // varite data rate is SF8, SF9, SF10 & SF11
  LMIC_setDrTxpow(DR_SF12, 14);
}

void loop()
{
  os_runloop_once();
}