#include "idt.h"
#include "arch/i386/cpu/gdt.h"
#include "arch/i386/apic.h"
#include <stdint.h>
#include "terminal.h"

#define UNUSED(E) ((void)E)

extern void* idtTable[];
extern void* rotTable[];

extern "C"
void IdtGenericHandler(interrupt_frame* frame, int code, int error, register_state* state)
{
    CIdt idt;
    
    if (code == 8 /* Double fault */
        || (code >= 10 && code <= 14) /* #TS-#PF */
        || code == 17                 /* #AC */
        || code == 21                 /* #CP */
        || code == 29                 /* #VC */
        || code == 30)                /* #SX */
    {
        interrupt_exception_routine* exc;
        exc = idt.GetRoutine<interrupt_exception_routine>(code);
        (*exc)(frame, error, state);
        return;
    }

    interrupt_serv_routine* ret;
    ret = idt.GetRoutine<interrupt_serv_routine>(code);
    if (ret == nullptr)
    {
        Warn("No entry point for interrupt vector %i!\n", code);
        goto eoi;
    }

    (*ret)(frame, state);

eoi:
    if (code >= 32)
    {
        acpi::EndOfInterrupt();
    }
    return;
}

void CIdt::AddEntry(unsigned char num, void* routine)
{
    interrupt_gate_desc dc;
    dc.IstOffset = 0;
    dc.OffsetLw16 = ((uint64_t)routine & 0xFFFF);
    dc.OffsetMd16 = ((uint64_t)routine >> 16) & 0xFFFF;
    dc.OffsetHi32 = ((uint64_t)routine >> 32);
    dc.SegmSelector = 3 * 8;
    dc.TypeAttrib = 0x8F; /* Trap gate */
    dc.Reserved = 0;

    if (num > 31)
    {
        dc.TypeAttrib = 0x8E; /* Interrupt gate */
    }

    m_ServiceRots[num] = dc;
}

void CIdt::AddRoutine(unsigned char vector, void* routine)
{
    m_Routines[vector] = routine;
}

void CIdt::InitDefaults()
{
    uint64_t i;
    for (i = 0; i < 36; i++)
    {
        this->AddEntry(i, idtTable[i]);
    }

    for (i = 0; i < 36; i++)
    {
        this->AddRoutine(i, &rotTable[i]);
    }
}

void CIdt::Encode()
{
    gdt_pointer potr;
    potr.size = sizeof(m_ServiceRots);
    potr.potr = reinterpret_cast<uint64_t>(&m_ServiceRots);
    asm volatile("lidt %0" :: "m"(potr));
}

interrupt_gate_desc CIdt::m_ServiceRots[48];
void* CIdt::m_Routines[48];
