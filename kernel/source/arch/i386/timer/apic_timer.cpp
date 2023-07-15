#include "apic_timer.h"
#include "arch/i386/apic.h"
#include "alloc/physical.h"
#include "hpet.h"
#include "time_units.h"

static void CalibrateTimer()
{
    acpi::InitializeHpet();
    acpi::AssociateHpetInterrupt();
    acpi::PrepareHpetDelay(1s);
}

acpi::CApicTimer::CApicTimer(acpi::CApic* apic)
    : m_Apic(apic)
{
}

void acpi::CApicTimer::Configure(apic_timer_mode mode)
{
    /* Divide by 16 */
    CalibrateTimer();
    m_Apic->WriteLocal(0x3E0, 0x3);
    uint32_t tm = 32 | (mode << 17);
    tm &= ~(1 << 16);
    m_Apic->WriteLocal(kLocalVectorTable_Timer, tm);
    m_Apic->WriteLocal(kLocalVectorTable_Error, 33);
}

void acpi::CApicTimer::SetInitialCount(uint32_t count)
{
    m_Apic->WriteLocal(0x380, count);
    asm volatile("sti"); /* IF is for some reason, clear */
}
