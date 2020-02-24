#include <Arduino.h>

#include <SoftwareSerial.h>
SoftwareSerial serialDebug(2, 3);

#define STATUS_OK 0x00
#define STATUS_WRONG_SIZE 0x01
#define STATUS_UNKOWN_CMD 0x02
#define STATUS_FORBIDDEN_CMD 0x03
#define STATUS_NOT_READY_CMD 0x04

#define CMD_DTYPE 0x00
#define CMD_VER 0x01
#define CMD_RESET 0x02

#define CMD_MODULE_MANUFACTURER 0x10
#define CMD_MODULE_IDENTIFIER 0x11
#define CMD_MODULE_VERSION 0x12
#define CMD_MODULE_SERIAL_NUMBER 0x13
#define CMD_MODULE_BAUDRATE 0x14

#define CMD_JOIN_MODE 0x20
#define CMD_OTAA_DEVICE_EUI 0x21
#define CMD_OTAA_APP_EUI 0x22
#define CMD_OTAA_APP_KEY 0x23
#define CMD_ABP_DEVICE_EUI 0x24
#define CMD_ABP_APP_SESS_KEY 0x25
#define CMD_ABP_NETWORK_SESS_KEY 0x26

#define CMD_FREQ_MARK 0x30
#define CMD_UL_DL_MODE 0x31
#define CMD_WORK_MODE 0x32
#define CMD_CLASS 0x33
#define CMD_BATTERY_LEVEL 0x34
#define CMD_STATUS 0x35
#define CMD_JOIN 0x36
#define CMD_DATA_FRAME_TX_RX 0x37
#define CMD_DATA_RX 0x38

#define CMD_IS_CONFIRM 0x40
#define CMD_APP_PORT 0x41
#define CMD_DATA_RATE 0x42
#define CMD_RSSI 0x43
#define CMD_NBTRANS 0x44
#define CMD_REPORT_MODE 0x45
#define CMD_TX_POWER 0x46
#define CMD_LINK_CHECK 0x47
#define CMD_ADR 0x48
#define CMD_RX_WIN 0x49
#define CMD_RX1_DELAY 0x4A

#define CMD_SAVE 0x50
#define CMD_RESTORE 0x51
#define CMD_REBOOT 0x52
#define CMD_LOW_POWER 0x53
#define CMD_ECHO 0x54

void setup()
{
  serialDebug.begin(115200);
}

void loop()
{
  serialDebug.println("B");
  delay(1000);
}