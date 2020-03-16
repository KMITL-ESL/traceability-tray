#include <Arduino.h>
#include <Wire.h>
#include "debug.h"

#define STATUS_Success 0x00
#define STATUS_I2C_Start_ERR 0xD0
#define STATUS_I2C_Stop_ERR 0xD1
#define STATUS_I2C_Stuffing_ERR 0xD2
#define STATUS_I2C_Checksum_ERR 0xD3
#define STATUS_I2C_Length_ERR 0xD4
#define STATUS_PARAM_Length_ERR 0xF0

uint8_t exeCMD(uint8_t CMD, uint8_t txBuff[], uint8_t txLen, uint8_t rxBuff[], uint8_t *rxLen, uint8_t rxMaxLen, int addr = 0x12, unsigned int delay = 100);
