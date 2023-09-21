#pragma once

#include "dllogic/api/dhelp.h"
#include "ide.h"
#include <vector>

using driver_error_t = unsigned int;

enum driver_error : driver_error_t
{
    DHELP_OK,
    DHELP_INVALID_ARG
};

class CAtaMassStorageDevice : public IDHelpMassStorageDevice
{
public:
    CAtaMassStorageDevice(ide::single_device* device, ide::ide_t* state);

    int ReadSectors(uint64_t lba, uint32_t size, 
                    uint8_t* buffer) override __system;
    int WriteSectors(uint64_t lba, uint32_t size, 
                            uint8_t *buffer) override __system;
    void GetDeviceName(const char **outName, size_t length) override __system;
    uint64_t GetMaxLba() override __system;
    int GetLastError() override __system;

private:
    ide::single_device* m_Device;
    ide::ide_t* m_State;

    driver_error_t m_Error;
};

class CAtaMassStorage : public IDHelpMassStorage
{
public:
    CAtaMassStorage(ide::ide_t* state);

    IDHelpMassStorageDevice* GetDevice(int index) override __system;
private:
    ide::ide_t* m_State;
    std::vector<CAtaMassStorageDevice*> m_Devices;
};
