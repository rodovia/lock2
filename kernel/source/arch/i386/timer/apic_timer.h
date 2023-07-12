#pragma once

#include <stdint.h>

namespace acpi 
{

class CApic;

enum apic_timer_divide_factor : unsigned short
{
    kTimerDivideBy2 = 0,
    kTimerDivideBy4,
    kTimerDivideBy8,
    kTimerDivideBy16,
    kTimerDivideBy32,
    kTimerDivideBy64,
    kTimerDivideBy128,
    kTimerDivideBy1
};

enum apic_timer_mode : unsigned short
{
    kTimerModeOneShot,
    kTimerModePeriodic,
    kTimerModeTimestampDeadline,
};

class CApicTimer
{
public:
    CApicTimer(CApic* apic);
    void Configure(apic_timer_mode mode);
    void SetInitialCount(uint32_t count);
private:
    CApic* m_Apic;
};

}
