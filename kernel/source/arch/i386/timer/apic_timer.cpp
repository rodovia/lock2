#include "apic_timer.h"
#include "arch/i386/apic.h"
#include "hpet.h"
#include "time_units.h"

static void CalibrateTimer()
{
    acpi::InitializeHpet();
    acpi::AssociateHpetInterrupt();

    acpi::WriteLocal(0x3E0, 0x3); /* Divide by 16 */
    acpi::WriteLocal(0x380, 0xFFFFFFFF); /* Set initial count */
    acpi::PrepareHpetDelay(10ms);
    uint32_t cnt = 0xFFFFFFFF - acpi::ReadLocal(0x390);
    Warn("cnt=0x%p", cnt);
    acpi::WriteLocal(0x380, cnt);
}

acpi::CApicTimer::CApicTimer(acpi::CApic* apic)
    : m_Apic(apic)
{
}

void acpi::CApicTimer::Configure(apic_timer_mode mode)
{
    uint32_t tm = 32 | (mode << 17);
    tm |= (1 << 16);

    acpi::WriteLocal(kLocalVectorTable_Timer, tm);
    CalibrateTimer();
    tm &= ~(1 << 16);
    acpi::WriteLocal(kLocalVectorTable_Timer, tm);

    acpi::WriteLocal(kLocalVectorTable_Error, 33);
    
    /* Divide by 16 */

}

void acpi::CApicTimer::SetInitialCount(uint32_t count)
{
    acpi::WriteLocal(0x380, count);
    asm volatile("sti"); /* IF is for some reason, clear */
}
