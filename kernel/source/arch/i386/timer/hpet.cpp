#include "hpet.h"
#include "acpi/tables.h"
#include "arch/i386/paging/paging.h"
#include "arch/i386/apic.h"
#include "alloc/physical.h"
#include "terminal.h"

#define GENCAP_REG 0
#define GENCFG_REG 0x2
#define GENINT_REG 0x4
#define GENMCV_REG 0x1E

#define TMR_GEN_CALC(V, x) (V + (0x20 * x)) / sizeof(uint64_t)

#define TMRCFG_REG(N) TMR_GEN_CALC(0x100, N)
#define TMRCMP_REG(N) TMR_GEN_CALC(0x108, N)
#define TMRFSB_REG(N) TMR_GEN_CALC(0x110, N)

#define TO_NS(Fs) Fs / 1000000
#define TO_FS(Ns) Ns * 1000000

volatile bool _hpetSleepFlag = true;

static constexpr 
uint8_t FindSuitableIrq(uint32_t irqs)
{
    uint8_t bitpos = 0;
    for (bitpos = 1; bitpos < 32; bitpos++)
    {
        if ((irqs & (1 << bitpos)) == 0)
        {
            continue;
        }

        return bitpos;
    }

    return -1;
}

void acpi::InitializeHpet()
{
    auto hd = reinterpret_cast<hpet_header*>(FindTable("HPET"));
    auto ptr = reinterpret_cast<uint64_t volatile*>(hd->BaseAddress.Address);
    virtm::AddGlobalPage({ 
        .VirtualAddr = (vaddr_t)ptr,
        .PhysicalAddr = (paddr_t)ptr,
        .Flags = PT_FLAG_WRITE
    });
}

void acpi::AssociateHpetInterrupt()
{
    auto hd = reinterpret_cast<hpet_header*>(FindTable("HPET"));
    auto ptr = reinterpret_cast<uint64_t volatile*>(hd->BaseAddress.Address);

    uint8_t finalInt;
    uint64_t cap = ptr[GENCAP_REG]; /* General Capabilities */
    if ((cap & (1 << 15))) /* Legacy IRQ-capable */
    {
        uint64_t tmrcfg = ptr[GENCFG_REG];
        tmrcfg |= 2; /* 0b10. Legacy IRQ enable and overall enable */
        ptr[GENCFG_REG] = tmrcfg;

        finalInt = 2;
    }
    else 
    {
        uint64_t tmrcfg = ptr[TMRCFG_REG(0)];
        finalInt = FindSuitableIrq(tmrcfg >> 32);
        tmrcfg |= (finalInt << 9) | (1 << 2) | (1 << 3); /* IRQ line and enable interrupts */
        ptr[TMRCFG_REG(0)] = tmrcfg;
    }

    uint64_t gencfg = ptr[GENCFG_REG];
    ptr[GENCFG_REG] = gencfg | 1;

    CApic apic;
    auto intr = apic.IoGetRedirectionEntry(finalInt);
    intr.Vector = 33;
    intr.Mask = 0;
    intr.Destination = acpi::local::GetCurrentCpuId();
    apic.IoSetRedirectionEntry(finalInt, intr);
    asm volatile("sti");
}

void acpi::PrepareHpetDelay(time::nanosec_t ns)
{
    _hpetSleepFlag = true;
    auto hd = reinterpret_cast<hpet_header*>(FindTable("HPET"));
    auto ptr = reinterpret_cast<uint64_t volatile*>(hd->BaseAddress.Address);

    /* HALT the timer */
    uint64_t tmr = ptr[GENCFG_REG];
    ptr[GENCFG_REG] = tmr & ~1;
    ptr[TMRCMP_REG(0)] = ptr[GENMCV_REG] + ns;

    /* Reenable the timer */
    ptr[GENCFG_REG] = tmr | 1;
    ptr[TMRCFG_REG(0)] |= (1 << 2);
    
    while (_hpetSleepFlag)
    {
        asm volatile("hlt");
    }
}
