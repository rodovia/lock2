#pragma once

#include <type_traits>
#include <stdint.h>
#include "arch/i386/port.h"

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

enum ata_drive
{
    kAtaDriveMaster,
    kAtaDriveSlave
};

enum ata_check_result
{
    kAtaChkNoDrive,
    kAtaChkAtapiDrive,
    kAtaChkAtaDrive
};

