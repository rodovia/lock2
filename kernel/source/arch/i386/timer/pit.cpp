#include "pit.h"
#include "acpi/tables.h"
#include "arch/i386/apic.h"
#include "arch/i386/port.h"

#include <algorithm>

CPit::CPit()
{
    acpi::CApic apic;
    auto ovr = apic.GetInterruptOverride(0);
    auto irq = ovr->GlobalSystemInterrupt - apic.GetGlobalSystemBase();
    acpi::io_apic_redir_entry ent = apic.IoGetRedirectionEntry(irq);

    ent.Vector = 33;
    ent.Mask = 0;
    ent.TriggerMode = 1; /* IDK */
    apic.IoSetRedirectionEntry(irq, ent);
}

void CPit::SetReloadValue(uint16_t value)
{
    port8 data = 0x40;
    port8 command = 0x43;

    /* Send low and high at once */
    (*command) = (3 << 4) | 1;
    (*data) = value & 0xFF;
    (*data) = (value & 0xFF00) >> 8;
}

uint16_t CPit::GetCurrentCount()
{
    port8 data = 0x40;
    port8 command = 0x43;
    uint16_t count;

    (*command) = (3 << 4);
    count = *data;
    count |= (*data) << 8;
    return count;
}
