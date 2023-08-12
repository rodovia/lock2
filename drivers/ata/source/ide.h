#pragma once

#include "ata.h"
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

namespace ide
{

struct single_device
{
    uint8_t Reserved; /*< Also used to check if the drive exists (1) or not (0) */
    uint8_t Channel; /*< Primary (0) or secondary (1) */
    uint8_t Drive; /*< Master (0) or slave (1) */
    uint8_t Type; /* ATA (0) or ATAPI (1) */
    uint16_t Signature;
    uint16_t Capabilities;
    uint32_t CommandSets; 
    uint32_t Size; /*< Device size given in sectors*/
    unsigned char Model[41];
};

struct channel
{
    uint16_t IoBase;
    uint16_t CtlBase;
    uint16_t BusMaster;
    uint8_t NoInterrupt;
};

struct ide_t
{
    single_device Devices[4];
    channel Channels[2];

    void IdeWrite(int channel, int reg, unsigned char data);
    uint8_t IdeRead(int channel, int reg);
};

void CreateController(IDHelpPciDevice* device);

}