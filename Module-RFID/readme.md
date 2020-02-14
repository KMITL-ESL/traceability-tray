## Frame

### write data
```
i2c write
┌─┬───────┬─┲━┱────────┲━┱────────┲━┱────────┲━┱────────┲━┓
│s│address│w┃A┃  0x2E  ┃A┃  CMD   ┃A┃  size  ┃A┃ Data 0 ┃A┃   ···
└─┴───────┴─┺━┹────────┺━┹────────┺━┹────────┺━┹────────┺━┹───
     ┌────────┲━┱────────┲━┱────────┲━┱────────┲━┱─┐
···  │Data n-1┃A┃ Data n ┃A┃chk sum ┃A┃  0x2E  ┃A┃P│   ···
   ──┴────────┺━┹────────┺━┹────────┺━┹────────┺━┹─┴───

i2c read
     ┌─┬───────┬─┲━┳━━━━━━━━┱─┲━━━━━━━━┱─┲━━━━━━━━┱─┲━━━━━━━━┱─┬─┐
···  │s│address│r┃A┃  0x2E  ┃A┃ status ┃A┃chk sum ┃A┃  0x2E  ┃Ã│P│
   ──┴─┴───────┴─┺━┻━━━━━━━━┹─┺━━━━━━━━┹─┺━━━━━━━━┹─┺━━━━━━━━┹─┴─┘
```

### read data
```
i2c write
┌─┬───────┬─┲━┱────────┲━┱────────┲━┱────────┲━┱────────┲━┱─┐
│s│address│w┃A┃  0x2E  ┃A┃  CMD   ┃A┃chk sum ┃A┃  0x2E  ┃A┃P│   ···
└─┴───────┴─┺━┹────────┺━┹────────┺━┹────────┺━┹────────┺━┹─┴───

i2c read
     ┌─┬───────┬─┲━┳━━━━━━━━┱─┲━━━━━━━━┱─┲━━━━━━━━┱─┲━━━━━━━━┱─┐
···  │s│address│r┃A┃  0x2E  ┃A┃ status ┃A┃  size  ┃A┃ Data 0 ┃A│   ···
   ──┴─┴───────┴─┺━┻━━━━━━━━┹─┺━━━━━━━━┹─┺━━━━━━━━┹─┺━━━━━━━━┹─┴───
     ┏━━━━━━━━┱─┲━━━━━━━━┱─┲━━━━━━━━┱─┲━━━━━━━━┱─┬─┐
···  ┃Data n-1┃A┃ Data n ┃A┃chk sum ┃A┃  0x2E  ┃Ã│P│
   ──┺━━━━━━━━┹─┺━━━━━━━━┹─┺━━━━━━━━┹─┺━━━━━━━━┹─┴─┘
```

### Start and Stop Byte (0x7E)
The 0x2E character is sent at the beginning and at the end of the frame to 
signalize frame start and stop. If this byte (0x2E) occurs anywhere else in 
the frame, it must be replaced by two other bytes (byte-stuffing).

| Original data byte | Transferred data bytes |
| :----------------: | :--------------------: |
|        0x2E        |       0x2D, 0xAE       |
|        0x2D        |       0x2D, 0xAD       |

### Size
Size is length of data to send (before byte-stuffing). If size is 0 then can send only status data.

### Checksum
The checksum is built before byte-stuffing and checked after removing stuffed 
bytes from the frame. The checksum is defined as follows:
1. Sum all bytes between start and stop (without start and stop bytes).
2. Take the LSB of the result and invert it. This will be the checksum.

#### Example (without address, start/stop and byte-stuffing)
|  CMD  | size  |   Data    |  CHK  |
| :---: | :---: | :-------: | :---: |
| 0x00  | 0x02  | 0x11,0xF2 | 0xFA  |

```
Sum = 0x00 + 0x02 + 0x11 + 0x02 = 0x105
LSB of Sum = 0x05
Inverted (=Checksum) = 0xFA
```

### Status code
| status | des                                            |
| :----: | ---------------------------------------------- |
|  0x00  | Success.                                       |
|  0x01  | Error in communication.                        |
|  0x02  | Collision detected.                            |
|  0x03  | Timeout in communication.                      |
|  0x04  | A buffer is not big enough.                    |
|  0x05  | Internal error in the code. Should not happen. |
|  0x06  | Invalid argument.                              |
|  0x07  | The CRC_A does not match.                      |
|  0xD0  | I2C : Start byte not match                     |
|  0xD1  | I2C : Byte-stuffing Error                      |
|  0xD2  | I2C : Checksum error                           |
|  0xD3  | I2C : Length error                             |
|  0xF0  | Parameter length not match                     |
|  0xFF  | A MIFARE PICC responded with NAK.              |
| Other  | Unknown error                                  |

## Command
### General
| command | code        | des         | can | res  |
| :-----: | ----------- | ----------- | --- | ---- |
|  0x00   | Device_Type | Device type | r   | 0x02 |
|  0x01   | Version     | Version     | r   | 0x00 |
|  0x02   | Reset       | Reset       | w   | **   |

### Functions for manipulating the MFRC522
| command | code                | des               | can | data        |
| :-----: | ------------------- | ----------------- | --- | ----------- |
|  0x10   | PCD_Init            | Init              | x   | [no]        |
|  0x11   | PCD_Reset           | Reset             | x   | [no]        |
|  0x12   | PCD_AntennaOn       | Antenna On        | x   | [no]        |
|  0x13   | PCD_AntennaOff      | Antenna Off       | x   | [no]        |
|  0x14   | PCD_AntennaGain     | AntennaGain       | rw  | gain        |
|  0x15   | PCD_PerformSelfTest | Perform Self Test | r   | func return |

### Power control functions
| command | code              | des             | can | data |
| :-----: | ----------------- | --------------- | --- | ---- |
|  0x20   | PCD_SoftPowerDown | Soft Power Down | x   | [no] |
|  0x21   | PCD_SoftPowerUp   | Soft Power On   | x   | [no] |

### Functions for communicating with PICCs
| command | code      | des  | can | data |
| :-----: | --------- | ---- | --- | ---- |
|  0x30   | PICC_Halt | Halt | x   | <no> |

### Functions for communicating with MIFARE PICCs
| command | code                    | des              | can | data  |
| :-----: | ----------------------- | ---------------- | --- | ----- |
|  0x40   | PCD_Authenticate        | Authenticate     | x   | [no]  |
|  0x41   | PCD_StopCrypto          | StopCrypto       | x   | [no]  |
|  0x42   | MIFARE_Read             | Read             | r   | data  |
|  0x43   | MIFARE_Write            | Write            | w   | data  |
|  0x44   | MIFARE_Ultralight_Write | Ultralight Write | w   | data  |
|  0x45   | MIFARE_Decrement        | Decrement        | w   | delta |
|  0x46   | MIFARE_Increment        | Increment        | w   | delta |
|  0x47   | MIFARE_Restore          | Restore          | x   | [no]  |
|  0x48   | MIFARE_Transfer         | Transfer         | x   | [no]  |
|  0x49   | MIFARE_Value            | get/set value    | rw  | value |
|  0x4A   | PCD_NTAG216_AUTH        | PCD_NTAG216_AUTH | w   | **    |

### Support functions
| command | code                    | des                | can | res |
| :-----: | ----------------------- | ------------------ | --- | --- |
|  0x50   | StatusCodeName          | Status Code Name   | r   | **  |
|  0x51   | PICC_Type               | PICC Type          | r   | **  |
|  0x52   | PICC_TypeName           | PICC Type Name     | r   | **  |
|  0x53   | MIFARE_SetAccessBits    | Set Access Bits    | r   | **  |
|  0x54   | MIFARE_OpenUidBackdoor  | Open Uid Backdoor  | w   | **  |
|  0x55   | MIFARE_SetUid           | Set Uid            | w   | **  |
|  0x56   | MIFARE_UnbrickUidSector | Unbrick Uid Sector | w   | **  |

### Convenience functions
| command | code                  | des                   | can | data        |
| :-----: | --------------------- | --------------------- | --- | ----------- |
|  0x60   | PICC_IsNewCardPresent | Is New Card Present   | r   | func return |
|  0x61   | PICC_ReadCardSerial   | Read Card Serial      | r   | func return |
|  0x62   | DATA_UID              | do with card uid      | rw  | uid         |
|  0x63   | DATA_SAK              | do with card sak      | rw  | sak         |
|  0x64   | DATA_KEY              | do with auth key      | rw  | key         |
|  0x65   | DATA_BLK_ADDR         | do with block address | rw  | blk addr    |