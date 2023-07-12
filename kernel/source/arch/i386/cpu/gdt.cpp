#include "gdt.h"
#include "terminal.h"

static CGdt gdt;

extern "C" void SegmsReload(uint64_t cs, uint64_t ds);

gdt_entry_encoded gdt_entry::Encode() const
{
    gdt_entry_encoded entr;
    entr.AccessByte = this->AccessByte;
    entr.BaseLow16 = this->Base & 0xFFFF;
    entr.BaseMid8 = (this->Base << 16) & 0xFF;
    entr.BaseHigh8 = (this->Base << 24) & 0xFF;
    entr.Flags = this->Flags;
    entr.Limit = this->Limit;
    return entr;
}

void CGdt::InitDefaults()
{
    this->AddEntry(0, {}); /* NULL */
    this->AddEntry(1, { 0xffff, 0, 0x9a, 0xcf }); /* 32-bit code */
    this->AddEntry(2, { 0xffff, 0, 0x92, 0xcf }); /* 32-bit data */
    this->AddEntry(3, { 0xffff, 0, 0x9a, 0xa2 }); /* 64-bit code */
    this->AddEntry(4, { 0xffff, 0, 0x92, 0xa0 }); /* 64-bit data */
    this->AddEntry(5, { 0xffff, 0, 0xF2, 0xa2 }); /* 64-bit user data */
    this->AddEntry(6, { 0xffff, 0, 0xFA, 0x20 }); /* 64-bit user code */
}

void CGdt::AddEntry(int index, gdt_entry ent)
{
    if (index > 10)
    {
        Warn("out of bounds attempt to write to m_Entries.\n");
        return;
    }

    m_Entries[index] = ent.Encode();
}

void CGdt::Encode()
{
    struct gdt_pointer pt;
    pt.size = sizeof(m_Entries) - 1;
    pt.potr = reinterpret_cast<uint64_t>(&m_Entries);
    asm volatile ("lgdt %0" :: "m"(pt));
    SegmsReload(3 * 8, 4 * 8);
}

void InitGdt()
{
    gdt.InitDefaults();
    gdt.Encode();
}
