#pragma once

#include "dllogic/api/dhelp.h"

#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

#define ATA_REG_DMA_COMMAND 0x16
#define ATA_REG_DMA_STATUS  0x18
#define ATA_REG_DMA_PRDT    0x1A

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    1
#define ATA_IDENT_HEADS        3
#define ATA_IDENT_SECTORS      6
#define ATA_IDENT_SERIAL       10
#define ATA_IDENT_MODEL        54 / sizeof(uint16_t)
#define ATA_IDENT_CAPABILITIES 98 / sizeof(uint16_t)
#define ATA_IDENT_FIELDVALID   106 / sizeof(uint16_t)
#define ATA_IDENT_MAX_LBA      120 / sizeof(uint16_t)
#define ATA_IDENT_COMMANDSETS  164 / sizeof(uint16_t)
#define ATA_IDENT_MAX_LBA_EXT  200 / sizeof(uint16_t)

#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_IDENTIFY_PACKET 0xA1

#define ATA_SR_BSY     0x80
#define ATA_SR_DRDY    0x40
#define ATA_SR_DF      0x20
#define ATA_SR_DSC     0x10
#define ATA_SR_DRQ     0x08
#define ATA_SR_CORR    0x04
#define ATA_SR_IDX     0x02
#define ATA_SR_ERR     0x01

#define ATA_PRIM_CHANNEL 0
#define ATA_SECN_CHANNEL 1

#define ATA_MAST_CHANNEL 0
#define ATA_SLAV_CHANNEL 1

namespace ide
{

struct single_device
{
    uint8_t Reserved; /*< Used to check if the drive exists (1) or not (0) */
    uint8_t Channel; /*< Primary (0) or secondary (1) */
    uint8_t Drive; /*< Master (0) or slave (1) */
    uint8_t Type; /* ATA (0) or ATAPI (1) */
    uint16_t Signature;
    uint16_t Capabilities;
    uint32_t CommandSets; 
    uint32_t Size; /*< Device size given in sectors*/
    char Model[41];
};

struct channel
{
    uint16_t IoBase;
    uint16_t CtlBase;
    uint16_t BusMaster;
    uint8_t NoInterrupt;
};

struct __attribute__((packed)) direct_memory
{
    uint32_t Address;
    uint16_t TransferSize; /*< 0 = 64K transfer*/
    uint16_t LastEntry; /* MSB set if last entry of PRDT */
};

struct ide_t
{
    single_device Devices[4];
    channel Channels[2];
    direct_memory* PhysicalRegions;
    int CurrentDevice;
    int CurrentChannel;
    IDHelpMutex* WriteMutex;

    void IdeWrite(int channel, int reg, unsigned char data);
    void IdeReadBuffer(int channel, int reg, 
                        uint32_t* buffer, uint32_t length);
    uint8_t IdeRead(int channel, int reg);
    void SelectDevice(int channel, int slavemaster);
};

ide::ide_t* CreateController(IDHelpPciDevice* device);
void SetupInterrupts(ide::ide_t* state);

}