#include "stackframe.h"
#include "elf.h"
#include "terminal.h"
#include <cstdint>

struct stackframe
{
    stackframe* Next;
    void* ReturnAddress;
};

static const char* 
GetSymbolOrAddress(void* ret, uint32_t& offset)
{
    uint8_t* strtab;
    uint64_t symts;
    uint64_t ret64 = (uint64_t)ret;
    dbg::elf_symtable* symtab;
    dbg::GetKernelSymbolTable((void*&)symtab, symts, (void*&)strtab);

    for (uint64_t i = 0; i < symts; i++)
    {
        dbg::elf_symtable t = symtab[i];
        uint64_t end = t.Value + t.Size;
        if (ret64 > t.Value && ret64 < end)
        {
            offset = ret64 - t.Value;
            return reinterpret_cast<char*>(&strtab[t.Name]);
        }
    }

    offset = 0;
    return "Unknown";
}

void dbg::DumpStackFrame(const struct stackframe* rbp)
{
    if (rbp == nullptr)
    {
        asm volatile("mov %%rbp, %0" : "=r"(rbp));
    }

    const char* symb;
    uint32_t offset, i = 0;
    CTerminal::Write("The stack trace is as follows:\n");
    while (rbp->Next != nullptr)
    {
        symb = GetSymbolOrAddress(rbp->ReturnAddress, offset);
        CTerminal::WriteFormatted("Stack #%i: <%s+%i> (0x%p)\n", i, symb, offset, rbp->ReturnAddress);
        rbp = rbp->Next;
        i++;
    }
}
