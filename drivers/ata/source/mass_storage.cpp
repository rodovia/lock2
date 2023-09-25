#include "mass_storage.h"
#include "alloc/physical.h"
#include "dllogic/api/dhelp.h"
#include "ide.h"
#include "ata.h"

#define Warn(M, ...) do {  \
    IDHelpTerminal* __term; \
    __DriverMgr->GetTerminal(&__term); \
    __term->WriteFormat(M __VA_OPT__(,) __VA_ARGS__); } \
    while(0)

extern IDHelpDriverManager* __DriverMgr;

CAtaMassStorage::CAtaMassStorage(ide::ide_t* state)
    : m_State(state),
    m_Devices(4)
{
    for (int i = 0; i < 4; i++)
    {
        if (!state->Devices[i].Reserved)
        {
            continue;
        }
        
        m_Devices.push_back(new CAtaMassStorageDevice(&state->Devices[i], m_State));
    }
}

IDHelpMassStorageDevice* CAtaMassStorage::GetDevice(int index)
{
    if (index > m_Devices.size())
    {
        return nullptr;
    }

    return m_Devices.at(index);
}

/* vvv CAtaMassStorageDevice vvv */

CAtaMassStorageDevice::CAtaMassStorageDevice(ide::single_device* device, ide::ide_t* state)
    : m_Device(device),
      m_State(state)
{}

int CAtaMassStorageDevice::ReadSectors(uint64_t lba, uint32_t size, 
                                       uint8_t *buffer)
{
    m_State->CurrentChannel = m_Device->Channel;
    m_State->CurrentDevice = m_Device->Drive;
    if (!m_Device->Type)
    {
        return -1;
    }

    /* ATAPI */ 
    AtapiReadCdRom(m_State, lba, buffer, 1, size);
    auto rd = m_State->IdeRead(m_State->CurrentChannel, ATA_REG_STATUS);
    return 0;
}

int CAtaMassStorageDevice::WriteSectors(uint64_t lba, uint32_t size,
                                        uint8_t* buffer)
{
    return 0;
}

void CAtaMassStorageDevice::GetDeviceName(const char **outName, size_t length)
{
    if (length > 40)
    {
        length = 40;
    }

    (*outName) = m_Device->Model;
}

uint64_t CAtaMassStorageDevice::GetMaxLba()
{
    return m_Device->Size;
}

int CAtaMassStorageDevice::GetLastError()
{
    if (m_Error == (driver_error_t)-1)
    {
    
    }
}
