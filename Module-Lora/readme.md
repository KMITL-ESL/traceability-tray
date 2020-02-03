Ref : [Examples of AT commands on I-CUBE-LRWAN](https://www.st.com/content/ccc/resource/technical/document/application_note/group0/db/8c/4f/5a/95/9e/40/69/DM00346311/files/DM00346311.pdf/jcr:content/translations/en.DM00346311.pdf)

## Frame

### write data
```
i2c write
┌─┬───────┬─┲━┱────────┲━┱────────┲━┱────────┲━┱────────┲━┓
│s│address│w┃A┃  0x2E  ┃A┃  CMD   ┃A┃  size  ┃A┃ Data 0 ┃A┃   ···
└─┴───────┴─┺━┹────────┺━┹────────┺━┹────────┺━┹────────┺━┹───
     ┌────────┲━┱────────┲━┱────────┲━┱─┐
···  │Data n-1┃A┃chk sum ┃A┃  0x2E  ┃A┃P│   ···
   ──┴────────┺━┹────────┺━┹────────┺━┹─┴───

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
     ┌─┬───────┬─┲━┳━━━━━━━━┱─┲━━━━━━━━┱─┲━━━━━━━━┱─┐
···  │s│address│r┃A┃  0x2E  ┃A┃  size  ┃A┃ Data 0 ┃A│   ···
   ──┴─┴───────┴─┺━┻━━━━━━━━┹─┺━━━━━━━━┹─┺━━━━━━━━┹─┴───
     ┏━━━━━━━━┱─┲━━━━━━━━┱─┲━━━━━━━━┱─┬─┐
···  ┃Data n-1┃A┃chk sum ┃A┃  0x2E  ┃Ã│P│
   ──┺━━━━━━━━┹─┺━━━━━━━━┹─┺━━━━━━━━┹─┴─┘
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
Size is length of data to send (before byte-stuffing).

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
| status | des                         |
| :----: | --------------------------- |
|  0x00  | No error                    |
|  0x01  | worg data size              |
|  0x02  | unknown command             |
|  0x03  | no access for this commannd |
|  0x04  | command not reeady          |

## Command
### General
| command | code  | des         | can | res  |
| :-----: | ----- | ----------- | --- | ---- |
|  0x00   | DTYPE | Device type | r   | 0x01 |
|  0x01   | VER   | Version     | r   | 0x00 |
|  0x02   | RESET | Reset       | w   |      |

### Keys, IDs and EUIs management
| command | code    | des                           | can | res |
| :-----: | ------- | ----------------------------- | --- | --- |
|  0x10   | APPEUI  | Application identifier        | rw  |
|  0x11   | APPKEY  | Application key               | rw  |
|  0x12   | APPSKEY | Application session key (ABP) | w   |
|  0x13   | DADDR   | Device address (ABP)          | rw  |
|  0x14   | DEUI    | Device EUI                    | rw  |
|  0x15   | NWKID   | Network ID (ABP)              | rw  |
|  0x16   | NWKSKEY | Network session key (ABP)     | w   |

### Joining and sending data on LoRa® network
| command | code  | des                       | can | res |
| :-----: | ----- | ------------------------- | --- | --- |
|  0x20   | CFM   | Confirm mode              | rw  |
|  0x21   | CFS   | Confirm status            | r   |
|  0x22   | JOIN  | Join LoRa® network        | w   |
|  0x23   | NJM   | LoRa® network join mode   | rw  |
|  0x24   | NJS   | LoRa® network join status | r   |
|  0x25   | RECVP | Last received port        | r   |
|  0x26   | RECVD | Last received data        | r   |
|  0x27   | SENP  | Send port                 | rw  |
|  0x28   | SEND  | Send data                 | w   |

### LoRa® network management
| command | code  | des                                | can | res |
| :-----: | ----- | ---------------------------------- | --- | --- |
|  0x30   | ADR   | Adaptive rate                      | rw  |
|  0x31   | CLASS | LoRa® class                        | rw  |
|  0x32   | DCS   | Duty cycle settings                | rw  |
|  0x33   | DR    | Data rate                          | rw  |
|  0x34   | JN1DL | Join delay on RX window 1          | rw  |
|  0x35   | JN2DL | Join delay on RX window 2          | rw  |
|  0x36   | PNM   | Public network mode                | rw  |
|  0x37   | RX1DL | Delay of the received window 1     | rw  |
|  0x38   | RX2DL | Delay of the received window 2     | rw  |
|  0x39   | RX2DR | Data rate of the received window 2 | rw  |
|  0x3A   | RX2FQ | Frequency of the received window 2 | rw  |
|  0x3B   | TXP   | Transmit power                     | rw  |
