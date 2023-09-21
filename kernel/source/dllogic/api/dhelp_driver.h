#pragma once

#include "dhelp_defs.h"
#include <stdint.h>
#include <stddef.h>

/* Interfaces to be implemented by drivers */

class IDHelpMassStorageDevice
{
public:
    virtual ~IDHelpMassStorageDevice() {}

    virtual int ReadSectors(uint64_t lba, uint32_t size,
                            uint8_t* buffer) __system __pure;
    virtual int WriteSectors(uint64_t lba, uint32_t size, 
                            uint8_t* buffer) __system __pure;
    virtual void GetDeviceName(const char **outName, size_t length) __system __pure;
    virtual uint64_t GetMaxLba() __system __pure;
    virtual int GetLastError() __system __pure;
};

class IDHelpMassStorage
{
public:
    virtual ~IDHelpMassStorage() {}

    virtual IDHelpMassStorageDevice* GetDevice(int index) __system __pure;
};
