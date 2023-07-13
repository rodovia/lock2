#pragma once

#include <stdint.h>
#include <vector>
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

struct io_apic_redir_entry 
{
    io_apic_redir_entry()
    {
    }

    constexpr io_apic_redir_entry(uint64_t raw)
        : Vector(raw & 0xFF),
          DeliveryMode((raw >> 8) & 7),
          DestinationMode((raw >> 11) & 1),
          DeliveryStatus((raw >> 12) & 1),
          PinPolarity((raw >> 13) & 1),
          RemoteIrr((raw >> 14) & 1),
          TriggerMode((raw >> 15) & 1),
          Mask((raw >> 14) & 1),
          Destination((raw >> 56) & 255)
    {
    }

    constexpr uint64_t Encode()
    {
        return Vector 
                | (DeliveryMode << 8)
                | (DestinationMode << 11)
                | (DeliveryStatus << 12)
                | (PinPolarity << 13)
                | (RemoteIrr << 14)
                | (TriggerMode << 15)
                | (Mask << 16)
                | (Destination << 56);
    }

    uint8_t Vector;
    uint16_t DeliveryMode;
    uint16_t DestinationMode;
    uint16_t DeliveryStatus;
    uint16_t PinPolarity;
    uint16_t RemoteIrr;
    uint16_t TriggerMode;
    uint16_t Mask;
    uint64_t Destination;
};

class CApicTimer;

class CApic
{
    friend class CApicTimer;
public:
    CApic(acpi::madt_header* header);
    CApic();
    ~CApic();

    CApicTimer* LocalGetTimer();
    void LocalGetId();
    void LocalUnmaskAll();
    void LocalSetLvt(apic_local_vector_table lv, uint8_t vector);

    uint8_t IoGetMaximumRedirectionEntries();
    io_apic_redir_entry IoGetRedirectionEntry(uint8_t idx);
    void IoSetRedirectionEntry(uint8_t idx, io_apic_redir_entry entry);
    madt_entry_io_apic_int_source_override* GetInterruptOverride(int irq);
    uint64_t GetGlobalSystemBase() const;
private:
    void WriteIo(uint8_t reg, uint32_t value);
    uint32_t ReadIo(uint8_t reg);

    void WriteLocal(uint32_t reg, uint32_t value);
    uint32_t ReadLocal(uint32_t reg);
    void ParseMadtVariableTable(acpi::madt_header* header);

private:
    std::vector<acpi::madt_entry_io_apic_int_source_override> m_IntSrcOverrides;

    CApicTimer* m_Timer = nullptr;
    volatile uint32_t* m_IoApicAddress;
    volatile uint32_t* m_LapicAddress;
    uint32_t m_GlobalSystemBase;
    bool m_OverrideLocalApic;
};

void EndOfInterrupt();

}
