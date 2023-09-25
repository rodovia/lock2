#pragma once

#include <type_traits>
#include <stdint.h>
#include "arch/i386/port.h"
#include "ide.h"

enum ata_io_register : unsigned short 
{
    kAtaRegIoData,
    kAtaRegIoErrorFeatures,
    kAtaRegIoSectorCountRegister,
    kAtaRegIoSectorNumber,
    kAtaRegIoSectorCylinderLow,
    kAtaRegIoSectorCylinderHigh,
    kAtaRegIoDriveSelectHead,
    kAtaRegIoStatusCommand,
};

enum ata_ctl_register : unsigned short
{
    kAtaRegCtlAlternateStatusDevCtl,
    kAtaRegCtlDriverAddress,
};

enum ata_check_result
{
    kAtaChkNoDrive,
    kAtaChkAtapiDrive,
    kAtaChkAtaDrive
};

void AtapiReadCdRom(ide::ide_t* state,
                    uint32_t lba,
                    uint8_t* buffer,
                    uint32_t sectors,
                    uint32_t maxByteCount);
