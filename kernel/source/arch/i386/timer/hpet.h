#pragma once

#include "acpi/tables.h"

namespace acpi
{

struct __attribute__((packed)) 
hpet_header : public system_desc_header
{
    uint32_t EventBlockTimerId;
    generic_addr_struct BaseAddress;
    uint8_t HpetNumber;
    uint16_t MinimumTicksPeriodic; /*< In spec: "Main Counter Minimum 
                                        Clock_tick in Periodic Mode" */
    uint8_t PageProtectionOem; /*< In spec: "Page Protection And OEM Attribute" */     
};

/* We only use HPET timer for a single purpose:
    To calibrate the Local APIC. So, there is no
    need to create a class for that. */

void InitializeHpet();

void AssociateHpetInterrupt();

}
