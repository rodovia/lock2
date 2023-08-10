#include "tables.h"

#include "acpica.h"

#include "klibc/string.h"
#include "arch/i386/timer/apic_timer.h"
#include "limine.h"
#include "requests.h"
#include "alloc/physical.h"
#include "terminal.h"
#include "arch/i386/apic.h"
#include "arch/i386/timer/apic_timer.h"

acpi::system_desc_header* 
acpi::GetRootTable()
{
    uint64_t hhdm = rqs::IsEfi() ? rqs::GetHhdm()->offset : 0;
    uint64_t rootaddr = (uint64_t)rqs::GetRsdp()->address - hhdm;
    
    struct rsdp* root = reinterpret_cast<rsdp*>(rootaddr);
    system_desc_header* rsdt = reinterpret_cast<system_desc_header*>(root->RsdtAddress);
    return rsdt;
}

acpi::system_desc_header* 
acpi::FindTable(const char* name, system_desc_header* rsdt)
{
    if (rsdt == nullptr)
    {
        rsdt = GetRootTable();
    }

    if (!strncmp("DSDT", name, 4))
    {
        acpi::fadt* fadt = reinterpret_cast<acpi::fadt*>(FindTable("FACP", rsdt));
        return reinterpret_cast<acpi::system_desc_header*>(fadt->Dsdt);
    }

    rsdt = reinterpret_cast<system_desc_header*>(rsdt);
    uint32_t* ptra = (uint32_t*)PaAdd(rsdt, sizeof(system_desc_header));
    int len = (rsdt->Length - sizeof(system_desc_header)) / sizeof(uint32_t);

    for (int i = 0; i < len; i++)
    {
        system_desc_header* tablept = reinterpret_cast<system_desc_header*>(ptra[i]);
        if (!strncmp(tablept->Signature, name, 4))
        {
            return tablept;
        }
    }

    return nullptr;
}

void acpi::ParseTables()
{
    auto rsdt = GetRootTable();
    madt_header* madt = reinterpret_cast<madt_header*>(FindTable("APIC", rsdt));
    int status;
    
    status = AcpiInitializeSubsystem();
    status = AcpiInitializeTables(nullptr, 0, false);
    status = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
    if (status == AE_NO_MEMORY)
    {
        Error("No memory to fit ACPI subsystem\n");
    }

    AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);

    CApic apic(madt);
    acpi::local::UnmaskAllInterrupts();
    CApicTimer tmr;
    tmr.Configure(kTimerModePeriodic);
}
