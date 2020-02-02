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

void do_send(osjob_t *j);

static uint8_t mydata[] = "Hello, world!";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 300;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10, // chip select
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9, // reset pin
    .dio = {2, 3, 4},
    .rxtx_rx_active = 0,
    .rssi_cal = 10,
};

void do_send(osjob_t *j)
{
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND)
  {
    Serial.println(F("OP_TXRXPEND, not sending"));
  }
  else
  {
    // Prepare upstream data transmission at the next possible time.
    LMIC_setTxData2(1, mydata, sizeof(mydata) - 1, 0);
    Serial.println(F("Packet queued"));
  }

  os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
}

void receiveEvent(int n)
{
}

void requestEvent()
{
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

  // Start job
  do_send(&sendjob);
}

void loop()
{
  os_runloop_once();
}