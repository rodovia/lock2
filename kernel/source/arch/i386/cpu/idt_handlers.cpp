#include "alloc/physical.h"
#include "arch/i386/debug/stackframe.h"
#include "arch/i386/paging/paging.h"
#include "klibc/stdlib.h"
#include "idt.h"
#include "scheduler/scheduler.h"
#include "terminal.h"

#define HLT do { asm volatile("cli; hlt"); __builtin_unreachable(); } while (0)
#pragma GCC diagnostic ignored "-Wunused-parameter"

static void DumpFrame(interrupt_frame* frame)
{
    CTerminal::WriteFormatted("RIP=0x%p, RSP=0x%p\n", frame->Rip, frame->Rsp);
    CTerminal::WriteFormatted("RFLAGS=0x%p frame=0x%p\n", frame->Rflags, frame);
}

static void Interrupt0(interrupt_frame* frame, register_state* state)
{
    _abortwrite("Division by zero or overflow\n");
    dbg::DumpStackFrame((struct stackframe*)state->Rbp);
    HLT;
}

static void Interrupt1(interrupt_frame* frame)
{
    return;
}

static void Interrupt2(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Non-masked interrupt.\n");
    HLT;
}

static void Interrupt3(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Breakpoint hit.\n");
    HLT;
}

static void Interrupt4(interrupt_frame* frame, register_state* state)
{
    _abortwrite("Overflow check failed\n");
    dbg::DumpStackFrame((struct stackframe*)state->Rbp);
    HLT;
}

static void Interrupt5(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt6(interrupt_frame* frame, register_state* state)
{
    _abortwrite("Invalid instruction\n");
    dbg::DumpStackFrame((struct stackframe*)state->Rbp);
    HLT;
}

static void Interrupt7(interrupt_frame* frame, register_state* state)
{
    _abortwrite("FPU disabled or unavailable (unlikely)\n");
    dbg::DumpStackFrame((struct stackframe*)state->Rbp);
    HLT;
}

static void Interrupt8(interrupt_frame* frame, register_state* state)
{
    _abortwrite("Double fault\n");
    dbg::DumpStackFrame((struct stackframe*)state->Rbp);
    HLT;
}

static void Interrupt9(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt10(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt11(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt12(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt13(interrupt_frame* frame, int error, register_state* state)
{
    _abortwrite("General protection fault; selector=%i\n", error);
    dbg::DumpStackFrame((struct stackframe*)state->Rbp);
    HLT;
}

static void Interrupt14(interrupt_frame* frame, int error, register_state* state)
{
    uint64_t cr2;
    asm volatile("mov %%cr2, %0" :"=r"(cr2));

    if (!(error & 1) && pm::WasAlloqued((void*)cr2))
    {
        virtm::MapPages(nullptr, cr2, cr2, PT_FLAG_WRITE);
        return;
    }
    
    _abortwrite("Page fault.\nAdditional info -> CR2=0x%p, 0x%p\n", cr2, error);
    dbg::DumpStackFrame((struct stackframe*)state->Rbp);
    DumpFrame(frame);
    HLT;
}

static void Interrupt15(interrupt_frame* frame)
{
    _abortwrite("");
    HLT;
}

static void Interrupt16(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt17(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt18(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt19(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt20(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt21(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt22(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt23(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt24(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt25(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt26(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt27(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt28(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt29(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt30(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt31(interrupt_frame* frame)
{
    CTerminal::Severity(SEVERITY_ERROR, "Division Error.\n");
    HLT;
}

static void Interrupt32(interrupt_frame* frame, register_state* state)
{
    full_register_state st = { *frame, *state };
    sched::CScheduler::Think(st);
}

#define _(E) reinterpret_cast<void*>(E)

void* rotTable[] = {
    _(Interrupt0), _(Interrupt1), _(Interrupt2), _(Interrupt3),
    _(Interrupt4), _(Interrupt5), _(Interrupt6), _(Interrupt7),
    _(Interrupt8), _(Interrupt9), _(Interrupt10), _(Interrupt11),
    _(Interrupt12), _(Interrupt13), _(Interrupt14), _(Interrupt15),
    _(Interrupt16), _(Interrupt17), _(Interrupt18), _(Interrupt19),
    _(Interrupt20), _(Interrupt21), _(Interrupt22), _(Interrupt23),
    _(Interrupt24), _(Interrupt25), _(Interrupt26), _(Interrupt27),
    _(Interrupt28), _(Interrupt29), _(Interrupt30), _(Interrupt31),
    _(Interrupt32),
};
