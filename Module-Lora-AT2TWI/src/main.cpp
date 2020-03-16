#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>

SoftwareSerial serialDebug(2, 3);
SoftwareSerial serialAT(4, 5);

#ifdef SHOW_DEBUG_CMD
#define DEBUG_CMD(msg)                 \
  do                                   \
  {                                    \
    serialDebug.print("DEBUG CMD : "); \
    serialDebug.println(msg);          \
  } while (0)
#else
#define DEBUG_CMD(msg) \
  do                   \
  {                    \
  } while (0)
#endif

#ifdef SHOW_DEBUG_I2C
#define DEBUG_I2C(msg)                 \
  do                                   \
  {                                    \
    serialDebug.print("DEBUG I2C : "); \
    serialDebug.println(msg);          \
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

#define STATUS_OK 0x00
#define STATUS_WRONG_SIZE 0x01
#define STATUS_UNKOWN_CMD 0x02
#define STATUS_FORBIDDEN_CMD 0x03
#define STATUS_NOT_READY_CMD 0x04
#define STATUS_I2C_Start_ERR 0xD0
#define STATUS_I2C_Stop_ERR 0xD1
#define STATUS_I2C_Stuffing_ERR 0xD2
#define STATUS_I2C_Checksum_ERR 0xD3
#define STATUS_I2C_Length_ERR 0xD4
#define STATUS_PARAM_Length_ERR 0xF0

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

uint8_t reqResData[200];
uint8_t reqResLen;

void setResStatus(uint8_t status)
{
  reqResData[0] = status;
  reqResLen = 1;
}

uint8_t execAT(const __FlashStringHelper *CMD, uint8_t *data, uint8_t len, bool canRetry = true)
{
  serialAT.print("AT+");
  serialAT.print(CMD);
  serialAT.print("=");
  if (len == 0)
  {
    serialAT.print("?");
  }
  else
  {
    for (int i = 0; i < len; i++)
    {
      if (data[i] < 16 && len != 1)
        serialAT.print("0");
      serialAT.print(data[i], HEX);
    }
  }

  String str = serialAT.readStringUntil('\n');
  DEBUG_CMD(str);
  if (str.startsWith(F("+ERROR")))
  {
    setResStatus(STATUS_NOT_READY_CMD);
  }
  else if (str.startsWith(F("+OK")))
  {
    str = serialAT.readStringUntil('\n');
    DEBUG_CMD(str);
    setResStatus(STATUS_OK);

    if (len == 0)
    {
      return 0;
    }

    int start = str.indexOf('+') + 1;
    int stop = str.indexOf('(');
    if (stop == -1)
    {
      if (str.endsWith("\r"))
        stop = str.length() - 1;
      else
        stop = str.length();
    }
    str = str.substring(start, stop);

    int length = str.length() / 2;

    reqResData[1] = length;
    reqResLen = length + 2;

    for (int i = 0; i < length; i++)
    {
      uint8_t first = str[i * 2];
      uint8_t second = str[i * 2 + 1];
      first = first <= '9' ? first - '0' : first - 'A' + 10;
      second = second <= '9' ? second - '0' : second - 'A' + 10;

      reqResData[i + 2] = ((first & 0xF) << 4) & (second & 0xF);
    }
  }
  else if (str.startsWith(F("ASR is Waked,LowPower Mode Stopped"), 1))
  {
    if (canRetry)
      execAT(CMD, data, len, false);
    else
      setResStatus(STATUS_NOT_READY_CMD);
    return 1;
  }
  else if (str.startsWith(F("LowPower Mode Stared")))
  {
    setResStatus(STATUS_OK);
  }
  return 0;
}

void prepareData(uint8_t CMD, uint8_t *data, uint8_t len)
{
  serialDebug.print(CMD, HEX);
  switch (CMD)
  {
  /////////////////////////////////////////////////////////////////////////////////////
  // General
  /////////////////////////////////////////////////////////////////////////////////////
  case CMD_Device_Type:
    reqResData[0] = STATUS_OK;
    reqResData[1] = 1;
    reqResData[2] = 0x03;
    reqResLen = 3;
    DEBUG_CMD(F("Get Device Type"));
    break;

  case CMD_Version:
    reqResData[0] = STATUS_OK;
    reqResData[1] = 1;
    reqResData[2] = 0x00;
    reqResLen = 3;
    DEBUG_CMD(F("Get Version"));
    break;

    /////////////////////////////////////////////////////////////////////////////////////
    // General functions for module
    /////////////////////////////////////////////////////////////////////////////////////

  case CMD_LPM:
    execAT(F("LPM"), data, len);
    break;

  case CMD_CHIP_ID:
    execAT(F("ChipID"), data, len);
    break;

  case CMD_LORAWAN:
    execAT(F("LORAWAN"), data, len);
    break;

    /////////////////////////////////////////////////////////////////////////////////////
    // LoRaWan Activation config
    /////////////////////////////////////////////////////////////////////////////////////

  case CMD_DEV_EUI:
    execAT(F("DevEui"), data, len);
    break;

  case CMD_APP_EUI:
    execAT(F("AppEui"), data, len);
    break;

  case CMD_APP_KEY:
    execAT(F("AppKey"), data, len);
    break;

  case CMD_NWK_S_KEY:
    execAT(F("NwkSKey"), data, len);
    break;

  case CMD_APP_S_KEY:
    execAT(F("AppSKey"), data, len);
    break;

  case CMD_DEV_ADDR:
    execAT(F("DevAddr"), data, len);
    break;

  case CMD_OTAA:
    execAT(F("OTAA"), data, len);
    break;

    /////////////////////////////////////////////////////////////////////////////////////
    // LoRaWan communication config
    /////////////////////////////////////////////////////////////////////////////////////

  case CMD_ADR:
    execAT(F("ADR"), data, len);
    break;

  case CMD_JOIN:
    execAT(F("Join"), data, len);
    break;

  case CMD_DUTY_CYCLE:
    execAT(F("DutyCycle"), data, len);
    break;

  case CMD_CLASS:
    execAT(F("Class"), data, len);
    break;

  case CMD_TX_CONFIRMED:
    execAT(F("IsTxConfirmed"), data, len);
    break;

  case CMD_APP_PORT:
    execAT(F("AppPort"), data, len);
    break;

  case CMD_CONFIREMED_NB_TRIALS:
    execAT(F("ConfirmedNbTrials"), data, len);
    break;

    /////////////////////////////////////////////////////////////////////////////////////
    // LoRaWan send
    /////////////////////////////////////////////////////////////////////////////////////

  case CMD_SEND:
    execAT(F("SendHex"), data, len);
    break;
  }
}

void receiveEvent(int n)
{
  DEBUG_I2C(F("IN"));
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

  DEBUG_I2C(F("TAIL"));

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
  serialAT.begin(115200);
  serialDebug.begin(115200);

  Wire.begin(0x13);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  serialDebug.println("HiHi");

  String str;
  do
  {
    str = serialAT.readStringUntil('\n');
  } while (str.length() > 0);
}

void loop()
{
}