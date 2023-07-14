#include "hpet.h"
#include "acpi/tables.h"
#include "arch/i386/paging/paging.h"

#define GENCAP_REG 0
#define GENCFG_REG 0x2
#define GENINT_REG 0x4
#define GENMCV_REG 0x1E

#define TMR_GEN_CALC(V, x) (V + 0x20 * x)

#define TMRCFG_REG(N) TMR_GEN_CALC(0x100, N)
#define TMRCMP_REG(N) TMR_GEN_CALC(0x108, N)
#define TMRFSB_REG(N) TMR_GEN_CALC(0x110, N)

void acpi::InitializeHpet()
{
    auto hd = reinterpret_cast<hpet_header*>(FindTable("HPET"));
    auto ptr = reinterpret_cast<uint64_t*>(hd->BaseAddress.Address);
    virtm::AddGlobalPage({ 
        .VirtualAddr = (vaddr_t)ptr,
        .PhysicalAddr = (paddr_t)ptr,
        .Flags = PT_FLAG_WRITE
    });
}

void acpi::AssociateHpetInterrupt()
{
    auto hd = reinterpret_cast<hpet_header*>(FindTable("HPET"));
    auto ptr = reinterpret_cast<uint64_t*>(hd->BaseAddress.Address);

    uint64_t cap = ptr[GENCAP_REG]; /* General Capabilities */
    if ((cap & (1 << 15)) == 1) /* Legacy IRQ-capable */
    {
        uint64_t tmrcfg = ptr[TMRCFG_REG(0)];

    }    
}
