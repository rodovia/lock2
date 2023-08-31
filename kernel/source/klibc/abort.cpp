#include "alloc/physical.h"
#include "arch/i386/debug/stackframe.h"
#include "stdlib.h"
#include "terminal.h"
#include <stdarg.h>

#define ABORT_MSG "An (unexpected) error occurred and the kernel cannot continue.\nReboot your system" \
        " by unplugging it from energy or by holding the power button\n(or by closing your emulator!).\n\n"

#define Msg CTerminal::WriteFormatted

void _abortwrite(const char* msg, ...)
{
    va_list vl;
    va_start(vl, msg);
    _abortwrite(msg, vl);
    va_end(vl);
}

void _abortwrite(const char* msg, va_list ls)
{
    if (msg == nullptr)
    {
        msg = "abort() was called.";
    }

#ifdef _LOCK2_WANT_PRETTY_ABORT
    CTerminal::PrepareForAbort();
#endif

    Msg(ABORT_MSG);
    Msg(msg, ls);
    Msg("\n");
}

void abort(const char* msg)
{
    _abortwrite(msg);
    dbg::DumpStackFrame();
    asm volatile("cli; hlt");

    __builtin_unreachable();
}

void abort(const char* msg, ...)
{
    va_list ls;
    va_start(ls, msg);
    _abortwrite(msg, ls);
    va_end(ls);
    dbg::DumpStackFrame();
    asm volatile("cli; hlt");

    __builtin_unreachable();
}

void abort2(const char* msg)
{
    _abortwrite(msg);
    asm volatile("cli; hlt");
    __builtin_unreachable();
}

void abort2(const char* msg, ...)
{
    va_list ls;
    va_start(ls, msg);
    _abortwrite(msg, ls);
    va_end(ls);
    asm volatile("cli; hlt");

    __builtin_unreachable();
}

