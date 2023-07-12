#include "intr.h"
#include "alloc/physical.h"
#include "dllogic/api/dhelp.h"

extern IDHelpDriverManager* __DriverMgr;

static uint64_t GetMsiAddress(uint16_t& data, uint8_t vector, uint32_t processor, 
                            bool edgetrigger, bool deassert)
{
    data = vector | (edgetrigger ? 0 : (1 << 15)) | (deassert ? 0 : (1 << 14));
    return 0xFEE | (processor << 12);
}

static bool HasCapabilityList(IDHelpPciDevice* devc)
{
    IDHelpTerminal* term;
    uint16_t status = devc->ReadConfigurationSpace(0x4) >> 16;
    __DriverMgr->GetTerminal(&term);
    term->WriteFormat("status=0x%p\n", status);
    return false;
}

uint8_t dhelp::RequestForInterrupt(IDHelpPciDevice* devc, int vector)
{
    IDHelpTerminal* t;
    __DriverMgr->GetTerminal(&t);

    HasCapabilityList(devc);
    uint32_t rd;
    uint16_t ctl;
    uint16_t msidat;
    uint64_t msiaddress;
    uint8_t cap = devc->ReadConfigurationSpace(0x34) & 0xFC;
    t->WriteFormat("cap=%p\n", cap);
    while (cap != 0)
    {
        rd = devc->ReadConfigurationSpace(cap);
        if ((rd & 0xFC) == 0x05)
        {
            break;
        }

        cap = (rd >> 8) & 0xFC;
    }

    BochsDebugBreak;
    rd |= (1 << 16); /* Enable MSI */
    devc->WriteConfigurationSpace(cap, rd);
    msiaddress = GetMsiAddress(msidat, vector, 0, false, false);

    devc->WriteConfigurationSpace(cap + 0x4, msiaddress);
    devc->WriteConfigurationSpace(cap + 0x8, msidat);
    return 0xFF;
}

