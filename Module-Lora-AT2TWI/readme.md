Ref : [E78-900M22S User Manual](http://www.ebyte.com/en/pdf-down.aspx?id=1561)

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
| status | des                        |
| :----: | -------------------------- |
|  0x00  | No error                   |
|  0x01  | wrong data size            |
|  0x02  | unknown command            |
|  0x03  | no access for this command |
|  0x04  | command not ready          |

## Command
### General
| command | code  | des         | can | res  |
| :-----: | ----- | ----------- | --- | ---- |
|  0x00   | DTYPE | Device type | r   | 0x01 |
|  0x01   | VER   | Version     | r   | 0x00 |
|  0x02   | RESET | Reset       | w   |      |

### Module General
| command | code | des                                   | can | res |
| :-----: | ---- | ------------------------------------- | --- | --- |
|  0x10   | CGMI | Read the manufacturer                 | r   |
|  0x11   | CGMM | Read module Identification            | r   |
|  0x12   | CGMR | Read version identifier               | r   |
|  0x13   | CGSN | Read product serial number identifier | r   |
|  0x14   | CGBR | Set the baud rate of the UART         | rw  |

### 
| command | code      | des                   | can | res |
| :-----: | --------- | --------------------- | --- | --- |
|  0x20   | CJOINMODE | Join mode (OTAA, ABP) | rw  |
|  0x21   | CDEVEUI   | DevEUI (OTAA)         | rw  |
|  0x22   | CAPPEUI   | AppEUI (OTAA)         | rw  |
|  0x23   | CAPPKEY   | AppKey (OTAA)         | rw  |
|  0x24   | CDEVADDR  | DevAddr (ABP)         | rw  |
|  0x25   | CAPPSKEY  | AppsKey (ABP)         | rw  |
|  0x26   | CNWKSKEY  | NnksKey (ABP)         | rw  |

### 
| command | code       | des                                 | can | res |
| :-----: | ---------- | ----------------------------------- | --- | --- |
|  0x30   | CFREQBANDM | Frequency mark                      | rw  |
|  0x31   | CULDLMODE  | Ul/Dl mode (same or diff freq)      | rw  |
|  0x32   | CWORKMODE  | Working mode                        | rw  |
|  0x33   | CCLASS     | Class type (Class A/C)              | rw  |
|  0x34   | CBL        | Battery level                       | r   |
|  0x35   | CSTATUS    | node status                         | rw  |
|  0x36   | CJOIN      | Init OTAA access to the network     | rw  |
|  0x37   | DTRX       | Send and receive data frames        | rw  |
|  0x38   | DRX        | latest received data from Rx buffer | rw  |

### 
| command | code       | des                                   | can | res |
| :-----: | ---------- | ------------------------------------- | --- | --- |
|  0x40   | CCONFIRM   | Type of read message (confirm or not) | rw  |
|  0x41   | CAPPPORT   | APP layer port                        | rw  |
|  0x42   | CDATARATE  | Data rate                             | rw  |
|  0x43   | CRSSI      | RSSI                                  | r   |
|  0x44   | CNBTRIALS  | NbTrans                               | rw  |
|  0x45   | CRM        | report mode                           | rw  |
|  0x46   | CTXP       | Transmit power                        | rw  |
|  0x47   | CLINKCHECK | Link check                            | rw  |
|  0x48   | CADR       | ADR                                   | rw  |
|  0x49   | CRXP       | receive window parameter              | rw  |
|  0x4A   | CRX1DELAY  | Delay to read TX and RX1              | rw  |

### 
| command | code        | des                           | can | res |
| :-----: | ----------- | ----------------------------- | --- | --- |
|  0x50   | CSAVE       | Save configuration            | w   |
|  0x51   | CRESTOREMAC | Restore default configuration | w   |
|  0x52   | IREBOOT     | System reboot                 | w   |
|  0x53   | CLPM        | System low power settings     | w   |
|  0x54   | ECHO        | echo configuration            | w   |
