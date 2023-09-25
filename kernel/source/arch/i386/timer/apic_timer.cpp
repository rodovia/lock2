#include "apic_timer.h"
#include "arch/i386/apic.h"
#include "hpet.h"
#include "time_units.h"

static void CalibrateTimer()
{
    acpi::InitializeHpet();
    acpi::AssociateHpetInterrupt();

    acpi::local::WriteLocal(0x3E0, 0x3); /* Divide by 16 */
    acpi::local::WriteLocal(0x380, 0xFFFFFFFF); /* Set initial count */
    acpi::PrepareHpetDelay(10ms);
    uint32_t cnt = 0xFFFFFFFF - acpi::local::ReadLocal(0x390);
    acpi::local::WriteLocal(0x380, cnt);
}

void acpi::CApicTimer::Configure(apic_timer_mode mode)
{
    uint32_t tm = 32 | (mode << 17);
    tm |= (1 << 16);

    acpi::local::WriteLocal(kLocalVectorTable_Timer, tm);
    CalibrateTimer();
    tm &= ~(1 << 16);

    acpi::local::WriteLocal(kLocalVectorTable_Timer, tm);
    acpi::local::WriteLocal(kLocalVectorTable_Error, 34);
    acpi::local::WriteLocal(0xF0, 47 | (1 << 8));
}

void acpi::CApicTimer::SetInitialCount(uint32_t count)
{
    acpi::local::WriteLocal(0x380, count);
    asm volatile("sti"); /* IF is for some reason, clear */
}
