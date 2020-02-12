#ifndef MFRC522_h
#define MFRC522_h

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

#define CMD_Device_Type 0x00
#define CMD_Version 0x01
#define CMD_Reset 0x02

#define CMD_PCD_Init 0x10
#define CMD_PCD_Reset 0x11
#define CMD_PCD_AntennaOn 0x12
#define CMD_PCD_AntennaOff 0x13
#define CMD_PCD_AntennaGain 0x14
#define CMD_PCD_PerformSelfTest 0x15

#define CMD_PCD_SoftPowerDown 0x20
#define CMD_PCD_SoftPowerUp 0x21

#define CMD_PICC_Halt 0x30

#define CMD_PCD_Authenticate 0x40
#define CMD_PCD_StopCrypto 0x41
#define CMD_MIFARE_Read 0x42
#define CMD_MIFARE_Write 0x43
#define CMD_MIFARE_Ultralight_Write 0x44
#define CMD_MIFARE_Decrement 0x45
#define CMD_MIFARE_Increment 0x46
#define CMD_MIFARE_Restore 0x47
#define CMD_MIFARE_Transfer 0x48
#define CMD_MIFARE_Value 0x49
#define CMD_PCD_NTAG216_AUTH 0x4A

#define CMD_StatusCodeName 0x50
#define CMD_PICC_Type 0x51
#define CMD_PICC_TypeName 0x52
#define CMD_MIFARE_SetAccessbits 0x53
#define CMD_MIFARE_OpenUidBackdoor 0x54
#define CMD_MIFARE_SetUid 0x55
#define CMD_MIFARE_UnbrickUidSector 0x56

#define CMD_PICC_IsNewCardPresent 0x60
#define CMD_PICC_ReadCardSerial 0x61
#define CMD_DATA_UID 0x62
#define CMD_DATA_SAK 0x63
#define CMD_DATA_KEY 0x64
#define CMD_DATA_BLK_ADDR 0x65

class MFRC522
{
public:
    // MFRC522 RxGain[2:0] masks, defines the receiver's signal voltage gain factor (on the PCD).
    // Described in 9.3.3.6 / table 98 of the datasheet at http://www.nxp.com/documents/data_sheet/MFRC522.pdf
    enum PCD_RxGain : byte
    {
        RxGain_18dB = 0x00 << 4,   // 000b - 18 dB, minimum
        RxGain_23dB = 0x01 << 4,   // 001b - 23 dB
        RxGain_18dB_2 = 0x02 << 4, // 010b - 18 dB, it seems 010b is a duplicate for 000b
        RxGain_23dB_2 = 0x03 << 4, // 011b - 23 dB, it seems 011b is a duplicate for 001b
        RxGain_33dB = 0x04 << 4,   // 100b - 33 dB, average, and typical default
        RxGain_38dB = 0x05 << 4,   // 101b - 38 dB
        RxGain_43dB = 0x06 << 4,   // 110b - 43 dB
        RxGain_48dB = 0x07 << 4,   // 111b - 48 dB, maximum
        RxGain_min = 0x00 << 4,    // 000b - 18 dB, minimum, convenience for RxGain_18dB
        RxGain_avg = 0x04 << 4,    // 100b - 33 dB, average, convenience for RxGain_33dB
        RxGain_max = 0x07 << 4     // 111b - 48 dB, maximum, convenience for RxGain_48dB
    };

    // Commands sent to the PICC.
    enum PICC_Command : byte
    {
        // The commands used by the PCD to manage communication with several PICCs (ISO 14443-3, Type A, section 6.4)
        PICC_CMD_REQA = 0x26,    // REQuest command, Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
        PICC_CMD_WUPA = 0x52,    // Wake-UP command, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
        PICC_CMD_CT = 0x88,      // Cascade Tag. Not really a command, but used during anti collision.
        PICC_CMD_SEL_CL1 = 0x93, // Anti collision/Select, Cascade Level 1
        PICC_CMD_SEL_CL2 = 0x95, // Anti collision/Select, Cascade Level 2
        PICC_CMD_SEL_CL3 = 0x97, // Anti collision/Select, Cascade Level 3
        PICC_CMD_HLTA = 0x50,    // HaLT command, Type A. Instructs an ACTIVE PICC to go to state HALT.
        PICC_CMD_RATS = 0xE0,    // Request command for Answer To Reset.
        // The commands used for MIFARE Classic (from http://www.mouser.com/ds/2/302/MF1S503x-89574.pdf, Section 9)
        // Use PCD_MFAuthent to authenticate access to a sector, then use these commands to read/write/modify the blocks on the sector.
        // The read/write commands can also be used for MIFARE Ultralight.
        PICC_CMD_MF_AUTH_KEY_A = 0x60, // Perform authentication with Key A
        PICC_CMD_MF_AUTH_KEY_B = 0x61, // Perform authentication with Key B
        PICC_CMD_MF_READ = 0x30,       // Reads one 16 byte block from the authenticated sector of the PICC. Also used for MIFARE Ultralight.
        PICC_CMD_MF_WRITE = 0xA0,      // Writes one 16 byte block to the authenticated sector of the PICC. Called "COMPATIBILITY WRITE" for MIFARE Ultralight.
        PICC_CMD_MF_DECREMENT = 0xC0,  // Decrements the contents of a block and stores the result in the internal data register.
        PICC_CMD_MF_INCREMENT = 0xC1,  // Increments the contents of a block and stores the result in the internal data register.
        PICC_CMD_MF_RESTORE = 0xC2,    // Reads the contents of a block into the internal data register.
        PICC_CMD_MF_TRANSFER = 0xB0,   // Writes the contents of the internal data register to a block.
        // The commands used for MIFARE Ultralight (from http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf, Section 8.6)
        // The PICC_CMD_MF_READ and PICC_CMD_MF_WRITE can also be used for MIFARE Ultralight.
        PICC_CMD_UL_WRITE = 0xA2 // Writes one 4 byte page to the PICC.
    };

    // MIFARE constants that does not fit anywhere else
    enum MIFARE_Misc
    {
        MF_ACK = 0xA,   // The MIFARE Classic uses a 4 bit ACK/NAK. Any other value than 0xA is NAK.
        MF_KEY_SIZE = 6 // A Mifare Crypto1 key is 6 bytes.
    };

    // PICC types we can detect. Remember to update PICC_GetTypeName() if you add more.
    // last value set to 0xff, then compiler uses less ram, it seems some optimisations are triggered
    enum PICC_Type : byte
    {
        PICC_TYPE_UNKNOWN,
        PICC_TYPE_ISO_14443_4,        // PICC compliant with ISO/IEC 14443-4
        PICC_TYPE_ISO_18092,          // PICC compliant with ISO/IEC 18092 (NFC)
        PICC_TYPE_MIFARE_MINI,        // MIFARE Classic protocol, 320 bytes
        PICC_TYPE_MIFARE_1K,          // MIFARE Classic protocol, 1KB
        PICC_TYPE_MIFARE_4K,          // MIFARE Classic protocol, 4KB
        PICC_TYPE_MIFARE_UL,          // MIFARE Ultralight or Ultralight C
        PICC_TYPE_MIFARE_PLUS,        // MIFARE Plus
        PICC_TYPE_MIFARE_DESFIRE,     // MIFARE DESFire
        PICC_TYPE_TNP3XXX,            // Only mentioned in NXP AN 10833 MIFARE Type Identification Procedure
        PICC_TYPE_NOT_COMPLETE = 0xff // SAK indicates UID is not complete.
    };

    // MIFARE constants that does not fit anywhere else
    enum MIFARE_Misc
    {
        MF_ACK = 0xA,   // The MIFARE Classic uses a 4 bit ACK/NAK. Any other value than 0xA is NAK.
        MF_KEY_SIZE = 6 // A Mifare Crypto1 key is 6 bytes.
    };

    // PICC types we can detect. Remember to update PICC_GetTypeName() if you add more.
    // last value set to 0xff, then compiler uses less ram, it seems some optimisations are triggered
    enum PICC_Type : byte
    {
        PICC_TYPE_UNKNOWN,
        PICC_TYPE_ISO_14443_4,        // PICC compliant with ISO/IEC 14443-4
        PICC_TYPE_ISO_18092,          // PICC compliant with ISO/IEC 18092 (NFC)
        PICC_TYPE_MIFARE_MINI,        // MIFARE Classic protocol, 320 bytes
        PICC_TYPE_MIFARE_1K,          // MIFARE Classic protocol, 1KB
        PICC_TYPE_MIFARE_4K,          // MIFARE Classic protocol, 4KB
        PICC_TYPE_MIFARE_UL,          // MIFARE Ultralight or Ultralight C
        PICC_TYPE_MIFARE_PLUS,        // MIFARE Plus
        PICC_TYPE_MIFARE_DESFIRE,     // MIFARE DESFire
        PICC_TYPE_TNP3XXX,            // Only mentioned in NXP AN 10833 MIFARE Type Identification Procedure
        PICC_TYPE_NOT_COMPLETE = 0xff // SAK indicates UID is not complete.
    };

    // Return codes from the functions in this class. Remember to update GetStatusCodeName() if you add more.
    // last value set to 0xff, then compiler uses less ram, it seems some optimisations are triggered
    enum StatusCode : byte
    {
        STATUS_OK,                // Success
        STATUS_ERROR,             // Error in communication
        STATUS_COLLISION,         // Collission detected
        STATUS_TIMEOUT,           // Timeout in communication.
        STATUS_NO_ROOM,           // A buffer is not big enough.
        STATUS_INTERNAL_ERROR,    // Internal error in the code. Should not happen ;-)
        STATUS_INVALID,           // Invalid argument.
        STATUS_CRC_WRONG,         // The CRC_A does not match
        STATUS_MIFARE_NACK = 0xff // A MIFARE PICC responded with NAK.
    };

    // A struct used for passing the UID of a PICC.
    typedef struct
    {
        byte size; // Number of bytes in the UID. 4, 7 or 10.
        byte uidByte[10];
        byte sak; // The SAK (Select acknowledge) byte returned from the PICC after successful selection.
    } Uid;

    // A struct used for passing a MIFARE Crypto1 key
    typedef struct
    {
        byte keyByte[MF_KEY_SIZE];
    } MIFARE_Key;

    // Member variables
    Uid uid; // Used by PICC_ReadCardSerial().

    /////////////////////////////////////////////////////////////////////////////////////
    // Functions for manipulating the MFRC522
    /////////////////////////////////////////////////////////////////////////////////////
    void PCD_Init();
    void PCD_Reset();
    void PCD_AntennaOn();
    void PCD_AntennaOff();
    byte PCD_GetAntennaGain();
    void PCD_SetAntennaGain(byte mask);
    bool PCD_PerformSelfTest();

    /////////////////////////////////////////////////////////////////////////////////////
    // Power control functions
    /////////////////////////////////////////////////////////////////////////////////////
    void PCD_SoftPowerDown();
    void PCD_SoftPowerUp();

    /////////////////////////////////////////////////////////////////////////////////////
    // Functions for communicating with PICCs
    /////////////////////////////////////////////////////////////////////////////////////
    StatusCode PICC_HaltA();

    /////////////////////////////////////////////////////////////////////////////////////
    // Functions for communicating with MIFARE PICCs
    /////////////////////////////////////////////////////////////////////////////////////
    StatusCode PCD_Authenticate(byte command, byte blockAddr, MIFARE_Key *key, Uid *uid);
    void PCD_StopCrypto1();
    StatusCode MIFARE_Read(byte blockAddr, byte *buffer, byte *bufferSize);
    StatusCode MIFARE_Write(byte blockAddr, byte *buffer, byte bufferSize);
    StatusCode MIFARE_Ultralight_Write(byte page, byte *buffer, byte bufferSize);
    StatusCode MIFARE_Decrement(byte blockAddr, int32_t delta);
    StatusCode MIFARE_Increment(byte blockAddr, int32_t delta);
    StatusCode MIFARE_Restore(byte blockAddr);
    StatusCode MIFARE_Transfer(byte blockAddr);
    StatusCode MIFARE_GetValue(byte blockAddr, int32_t *value);
    StatusCode MIFARE_SetValue(byte blockAddr, int32_t value);
    StatusCode PCD_NTAG216_AUTH(byte *passWord, byte pACK[]);

    /////////////////////////////////////////////////////////////////////////////////////
    // Support functions
    /////////////////////////////////////////////////////////////////////////////////////
    StatusCode PCD_MIFARE_Transceive(byte *sendData, byte sendLen, bool acceptTimeout = false);
    static const __FlashStringHelper *GetStatusCodeName(StatusCode code);
    static PICC_Type PICC_GetType(byte sak);
    static const __FlashStringHelper *PICC_GetTypeName(PICC_Type type);

    // Support functions for debuging
    void PCD_DumpVersionToSerial();
    void PICC_DumpToSerial(Uid *uid);
    void PICC_DumpDetailsToSerial(Uid *uid);
    void PICC_DumpMifareClassicToSerial(Uid *uid, PICC_Type piccType, MIFARE_Key *key);
    void PICC_DumpMifareClassicSectorToSerial(Uid *uid, MIFARE_Key *key, byte sector);
    void PICC_DumpMifareUltralightToSerial();

    // Advanced functions for MIFARE
    void MIFARE_SetAccessBits(byte *accessBitBuffer, byte g0, byte g1, byte g2, byte g3);
    bool MIFARE_OpenUidBackdoor(bool logErrors);
    bool MIFARE_SetUid(byte *newUid, byte uidSize, bool logErrors);
    bool MIFARE_UnbrickUidSector(bool logErrors);

    /////////////////////////////////////////////////////////////////////////////////////
    // Convenience functions - does not add extra functionality
    /////////////////////////////////////////////////////////////////////////////////////
    virtual bool PICC_IsNewCardPresent();
    virtual bool PICC_ReadCardSerial();

    void GET_UID();
    void GET_SAK();
    void SET_KEY(MIFARE_Key *key);
    void SET_BLK_ADDR(byte blockAddr);

    Uid clientUid;
    MIFARE_Key clientKey;
    byte clientBlockAddr;

    void checkAndChangeUID(Uid *uid);
    void checkAndChangeKey(MIFARE_Key *key);
    void checkAndChangeBlockAddr(byte blockAddr);
};

#endif