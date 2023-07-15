#include "apic.h"
#include "timer/apic_timer.h"
#include "alloc/physical.h"
#include "arch/i386/paging/paging.h"
#include "acpi/tables.h"
#include "terminal.h"
#include <algorithm>

#define LAPIC_REG(R) R / sizeof(uint32_t)

uint32_t volatile* _localApicAddress = nullptr;
uint32_t volatile* _ioApicAddress = nullptr;

acpi::CApic::CApic(acpi::madt_header* apic)
    : m_IoApicAddress(nullptr),
      m_LapicAddress(reinterpret_cast<uint32_t volatile*>(apic->LocalApic)),
      m_OverrideLocalApic(false)
{
    this->ParseMadtVariableTable(apic);
    virtm::AddGlobalPage({
        .VirtualAddr = reinterpret_cast<paddr_t>(m_LapicAddress),
        .PhysicalAddr = reinterpret_cast<vaddr_t>(m_LapicAddress),
        .Flags = PT_FLAG_PCD | PT_FLAG_WRITE
    });

    virtm::AddGlobalPage({
        .VirtualAddr = reinterpret_cast<paddr_t>(m_IoApicAddress),
        .PhysicalAddr = reinterpret_cast<paddr_t>(m_IoApicAddress),
        .Flags = PT_FLAG_PCD | PT_FLAG_WRITE
    });

    if (_localApicAddress == nullptr)
    {
        _localApicAddress = m_LapicAddress;
    }

    if (_ioApicAddress == nullptr)
    {
        _ioApicAddress = m_IoApicAddress;
    }
}

acpi::CApic::CApic()
    : CApic(reinterpret_cast<madt_header*>(acpi::FindTable("APIC")))
{
}

acpi::CApic::~CApic()
{
    if (m_Timer != nullptr)
    {
        delete m_Timer;
    }
}

void acpi::CApic::ParseMadtVariableTable(acpi::madt_header* madt)
{
    uint8_t* entries = (uint8_t*)PaAdd(madt, sizeof(acpi::madt_header));
    int remaining = madt->Length - sizeof(acpi::madt_header);
    int entrySize = 0;
    do 
    {
        switch (*entries)
        {
        case MADT_IO_APIC:
        {
            auto addr = reinterpret_cast<acpi::madt_entry_io_apic*>(entries);
            m_IoApicAddress = reinterpret_cast<volatile uint32_t*>(addr->IoApicAddress);
            m_GlobalSystemBase = addr->GsiBase;
            break;
        }
        case MADT_ENTRY_LOCAL_APIC_ADDR_OVERRIDE:
        {
            if (m_OverrideLocalApic)
            {
                Warn("Local APIC address override specified more than once.\n");
                break;
            }

            auto addr = reinterpret_cast<acpi::madt_entry_lapic_addr*>(entries);
            m_LapicAddress = reinterpret_cast<volatile uint32_t*>(addr->LocalApicAddress);
            m_OverrideLocalApic = true;
            break;
        }
        case MADT_IO_APIC_INT_SOURCE_OVERRIDE:
        {
            auto addr = reinterpret_cast<acpi::madt_entry_io_apic_int_source_override*>(entries);
            m_IntSrcOverrides.push_back(*addr);
            break;
        }
        default:
            break;
        }

        entrySize = *(entries + 1);
        remaining -= entrySize;
        entries += entrySize;
    } while (remaining > 0);
}


uint8_t acpi::CApic::IoGetMaximumRedirectionEntries()
{
    static uint32_t cache;
    static bool needsRecache = true;

    if (needsRecache)
    {
        cache = this->ReadIo(0x1);
        needsRecache = false;
    }

    return ((cache >> 16) & 0xFF);
}

acpi::io_apic_redir_entry
acpi::CApic::IoGetRedirectionEntry(uint8_t idx)
{
    uint32_t actual = 0x10 + idx * 2;
    uint32_t lower = this->ReadIo(actual);
    uint64_t higher = this->ReadIo(actual + 1);
    return io_apic_redir_entry((higher << 32) | lower);
}

void acpi::CApic::IoSetRedirectionEntry(uint8_t idx, acpi::io_apic_redir_entry entry)
{
    if (idx > this->IoGetMaximumRedirectionEntries())
    {
        Warn("IoSetRedirectionEntry: out of bounds write unsupported (idx %i against max %i)\n");
        return;
    }

    uint8_t actual = 0x10 + idx * 2;
    auto enc = entry.Encode();
    this->WriteIo(actual, enc & 0xFFFFFFFF);
    this->WriteIo(actual + 1, enc >> 32);
}

void acpi::CApic::WriteIo(uint8_t reg, uint32_t value)
{
    m_IoApicAddress[0] = reg & 0xFF;
    m_IoApicAddress[4] = value;
}

uint32_t acpi::CApic::ReadIo(uint8_t reg)
{
    m_IoApicAddress[0] = reg;
    return m_IoApicAddress[4];
}

void acpi::CApic::WriteLocal(uint32_t reg, uint32_t value)
{
    uint32_t volatile* pad = reinterpret_cast<uint32_t volatile*>(PaAdd(m_LapicAddress, reg));
    (*pad) = value;
}

uint32_t acpi::CApic::ReadLocal(uint32_t reg)
{
    uint32_t volatile* pad = reinterpret_cast<uint32_t volatile*>(PaAdd(m_LapicAddress, reg));
    return (*pad);
}

void acpi::CApic::LocalUnmaskAll()
{
    this->WriteLocal(0xF0, 0xFF | (1 << 8));
}

void acpi::CApic::LocalSetLvt(apic_local_vector_table lv, uint8_t vector)
{
    this->WriteLocal(lv, vector);
}

acpi::CApicTimer*
acpi::CApic::LocalGetTimer()
{
    if (m_Timer == nullptr)
    {
        m_Timer = new CApicTimer(this);
    }
    
    return m_Timer;
}

uint64_t acpi::CApic::GetGlobalSystemBase() const
{
    return m_GlobalSystemBase;
}

acpi::madt_entry_io_apic_int_source_override*
acpi::CApic::GetInterruptOverride(int irq)
{
    auto pred = [irq](const madt_entry_io_apic_int_source_override& ovr)
        {
            return ovr.IrqSource == irq;
        };
    auto iter = std::find_if(m_IntSrcOverrides.begin(), m_IntSrcOverrides.end(), pred);
    if (iter == m_IntSrcOverrides.end())
    {
        return nullptr;
    }

    return iter + 0;
}

void acpi::EndOfInterrupt()
{
    if (_localApicAddress == nullptr)
    {
        Error("Cannot send EOI!\n");
        return;
    }
    
    _localApicAddress[LAPIC_REG(0xB0)] = 0;
}

acpi::cpuid_t acpi::GetCurrentCpuId()
{
    if (_localApicAddress == nullptr)
    {
        Error("Cannot get CPU id!\n");
        return -1;
    }

    /* Should I mask the upper 6 bits on Pentium processors? */
    return _localApicAddress[LAPIC_REG(0x20)] >> 24;
}

void acpi::WriteLocal(uint32_t reg, uint32_t value)
{
    _localApicAddress[LAPIC_REG(reg)] = value;
}

uint32_t acpi::ReadLocal(uint32_t reg)
{
    return _localApicAddress[LAPIC_REG(reg)];
}
