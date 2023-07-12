#include "ide.h"
#include "dllogic/api/dhelp.h"
#include <vector>

#define ATA_PRIMARY_IO_PORT 0x1F0
#define ATA_PRIMARY_CTL_PORT 0x3F0
#define ATA_SECONDARY_IO_PORT 0x170
#define ATA_SECONDARY_CTL_PORT 0x376

extern IDHelpDriverManager* __DriverMgr;

template<bool FromMaster> 
constexpr static ide::device_ports GetPorts(IDHelpPciDevice* pci)
{
    if constexpr (FromMaster)
    {
        uint16_t masterio = static_cast<uint16_t>(pci->ReadConfigurationSpace(0x10));
        uint16_t masterctl = static_cast<uint16_t>(pci->ReadConfigurationSpace(0x14));
        return { masterio, masterctl, 0x0 };
    }

    uint16_t slaveio = static_cast<uint16_t>(pci->ReadConfigurationSpace(0x18));
    uint16_t slavectl = static_cast<uint16_t>(pci->ReadConfigurationSpace(0x1C));
    return { slaveio, slavectl, 0x0 };
}

void ide::CreateController(IDHelpPciDevice *devc)
{
    ide::devices devs;
    IDHelpTerminal* term;
    __DriverMgr->GetTerminal(&term);
    uint8_t progif = devc->ReadConfigurationSpace(0x8) & 0xFF00;
    term->WriteFormat("progif=0x%p", progif);
    devs.PrimaryMaster = { ATA_PRIMARY_IO_PORT, ATA_PRIMARY_CTL_PORT, 14 };
    devs.SecondaryMaster = { ATA_SECONDARY_IO_PORT, ATA_SECONDARY_CTL_PORT, 15 };

    if (progif & 1) /* primary channel not in PCI native */
    {
        ide::device_ports p = GetPorts<true>(devc);
        p.HandledIrq = 50;
        devs.PrimaryMaster = p;
    }

    if (progif & (1 << 3)) /* Secondary channel not in PCI native */
    {
        ide::device_ports p = GetPorts<false>(devc);
        p.HandledIrq = 50;
        devs.PrimaryMaster = p;
    }
}
