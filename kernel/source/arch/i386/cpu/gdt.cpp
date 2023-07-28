#include "gdt.h"
#include "terminal.h"
#include "alloc/physical.h"

static CGdt gdt;
static gdt_entries entries;

extern "C" void SegmsReload(uint64_t cs, uint64_t ds);
static struct tss tss;
static char _ring0Stack[4096];
static char _interruptStack[2048];

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
    entries.Code32Bit = gdt_entry { 0xffff, 0, 0x9a, 0xcf }.Encode();
    entries.Data32Bit = gdt_entry { 0xffff, 0, 0x92, 0xcf }.Encode();
    entries.KernelCode64Bit = gdt_entry{ 0xffff, 0, 0x9a, 0xa2 }.Encode();
    entries.KernelData64Bit = gdt_entry{ 0xffff, 0, 0x92, 0xa0 }.Encode();
    entries.UserCode64Bit = gdt_entry { 0xffff, 0, 0xFA, 0x20 }.Encode();
    entries.UserData64Bit = gdt_entry{ 0xffff, 0, 0xF2, 0xa2 }.Encode();

    memset(&tss, 0, sizeof(tss));
    tss.Rsp[0] = (uint64_t)_ring0Stack;
    tss.Ist[0] = (uint64_t)_interruptStack;
}

void CGdt::AddEntry(int index, gdt_entry ent)
{
    if (index > 10)
    {
        Warn("out of bounds attempt to write to m_Entries.\n");
        return;
    }
}

void CGdt::Encode()
{
    struct gdt_pointer pt;
    pt.size = sizeof(entries) - 1;
    pt.potr = reinterpret_cast<uint64_t>(&entries);
    asm volatile ("lgdt %0" :: "m"(pt));
    SegmsReload(3 * 8, 4 * 8);
}

void InitGdt()
{
    gdt.InitDefaults();
    gdt.Encode();
}
