#pragma once

#include <stdint.h>
#include "acpi/tables.h"

#define IOREDTBL_VECTOR 0xFF
#define IOREDTBL_DELIVERY_MODE  0x700 /* bits 8-10 */
#define IOREDTBL_DESTINATION_MODE (1 << 11) 
#define IOREDTBL_DESTINATION_STATUS (1 << 12)
#define IOREDTBL_PIN_POLARITY       (1 << 13)
#define IOREDTBL_TRIGGER_MODE       (1 << 15)
#define IOREDTBL_MASK               (1 << 16)

namespace acpi
{

enum apic_local_vector_table : unsigned short
{
    kLocalVectorTable_Timer = 0x320,
    kLocalVectorTable_ThermalSensor = 0x330,
    kLocalVectorTable_PerfMonitor   = 0x340,
    kLocalVectorTable_Lint0 = 0x350,
    kLocalVectorTable_Lint1 = 0x360,
    kLocalVectorTable_Error = 0x370
};

using io_apic_redir_entry = uint64_t;

class CApicTimer;

class CApic
{
    friend class CApicTimer;
public:
    CApic(acpi::madt_header* header);
    CApic();
    ~CApic();

    CApicTimer* LocalGetTimer();
    void EndOfInterrupt();
    void LocalGetId();
    void LocalUnmaskAll();
    void LocalSetLvt(apic_local_vector_table lv, uint8_t vector);

    void DumpIrrIsrTmr();

    uint8_t IoGetMaximumRedirectionEntries();
    io_apic_redir_entry IoGetRedirectionEntry(uint8_t idx);
    void IoSetRedirectionEntry(uint8_t idx, io_apic_redir_entry entry);
private:
    void WriteIo(uint8_t reg, uint32_t value);
    uint32_t ReadIo(uint8_t reg);

    void WriteLocal(uint32_t reg, uint32_t value);
    uint32_t ReadLocal(uint32_t reg);
    void ParseMadtVariableTable(acpi::madt_header* header);

private:
    CApicTimer* m_Timer = nullptr;
    volatile uint32_t* m_IoApicAddress;
    volatile uint32_t* m_LapicAddress;
    bool m_OverrideLocalApic;
};

}
