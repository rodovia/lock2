#pragma once

#include <stdint.h>
#include "terminal.h"

using uword = unsigned long;

struct interrupt_frame
{
    uword Rip;
    uword Cs;
    uword Rflags;
    uword Rsp;
    uword Ss;
};

struct __attribute__((packed)) 
register_state
{
    uword R15;
    uword R14;
    uword R13;
    uword R12;
    uword R11;
    uword R10;
    uword R9;
    uword Rsi;
    uword Rdi;
    uword Rdx;
    uword Rcx;
    uword Rbx;
    uword Rax;
    uword Rbp;
};

struct full_register_state
{
    interrupt_frame Pointers;
    register_state OtherRegisters;
};

struct interrupt_gate_desc
{
    uint16_t OffsetLw16;
    uint16_t SegmSelector;
    uint8_t IstOffset;
    uint8_t TypeAttrib;
    uint16_t OffsetMd16;
    uint32_t OffsetHi32;
    uint32_t Reserved;
};

using interrupt_serv_routine = void(*)(interrupt_frame*, register_state*);
using interrupt_exception_routine = void(*)(interrupt_frame*, int, register_state*);

class CIdt
{
    static interrupt_gate_desc m_ServiceRots[48];
    static void* m_Routines[48];    /* m_ServiceRots are wrappers to m_Routines */
public:
    void InitDefaults();
    void AddEntry(unsigned char vector, void* isr);
    void AddRoutine(unsigned char vector, void* routine);
    void Encode();

    template<class _Ty> 
    constexpr _Ty* GetRoutine(unsigned char num)
    {
        return reinterpret_cast<_Ty*>(m_Routines[num]);
    }
};
