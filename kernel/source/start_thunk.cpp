#include "acpi/tables.h"
#include "alloc/physical.h"

#include "scheduler/scheduler.h"
#include "terminal.h"
#include "limine.h"
#include "arch/i386/cpu/gdt.h"
#include "arch/i386/cpu/idt.h"
#include "arch/i386/paging/paging.h"
#include "requests.h"
#include "dllogic/load.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static void hcf(void);
static limine_memmap_entry* DetermineUsableEntry();

extern "C" 
void KeStartThunk()
{
    CTerminal::WriteFormatted("Hello, Lock2! %s %s\n", __DATE__, __TIME__);
    InitGdt();

    CIdt idt;
    idt.InitDefaults();
    idt.Encode();

    limine_memmap_entry* ent = DetermineUsableEntry();
    pm::Create(reinterpret_cast<void*>(ent->base), ent->length);
    virtm::SetCr3((paddr_t)virtm::CreatePml4());
    acpi::ParseTables();
    driver::LoadDrivers();

    auto& c = sched::CScheduler::GetInstance();
    c.Enable();

    Info("Finished execution!\n");
    hcf();
}

static limine_memmap_entry* DetermineUsableEntry()
{
    uint64_t max = 0;
    struct limine_memmap_entry* ret = nullptr;
    struct limine_memmap_response* re = rqs::GetMemoryMap();
    for (uint64_t i = 0; i < re->entry_count; i++)
    {
        limine_memmap_entry* c = re->entries[i];
        /*Info("Memory map: base=0x%p, length=%i or %i KB, type=%i\n", c->base, c->length, 
            c->length / 1024, c->type);*/
        if (c->type == LIMINE_MEMMAP_USABLE &&
            c->length >= max)
        {
            max = c->length;
            ret = c;
        }
    }
    
    return ret;
}

static void hcf(void) 
{
    for (;;) {
        __asm__ ("hlt");
    }
}
