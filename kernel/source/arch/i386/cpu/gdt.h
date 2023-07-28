#pragma once

#include <stdint.h>

struct __attribute__((packed))
gdt_pointer
{
    uint16_t size;
    uint64_t potr;  
};

struct  __attribute__((packed)) 
gdt_entry_encoded
{
    uint16_t Limit;
    uint16_t BaseLow16;
    uint8_t BaseMid8;
    uint8_t AccessByte;
    uint8_t Flags;
    uint8_t BaseHigh8;
};

struct __attribute__((packed)) 
gdt_entry
{
    uint32_t Base;
    uint16_t Limit;
    uint8_t AccessByte;
    uint8_t Flags;

    gdt_entry_encoded Encode() const;
};

struct  __attribute__((packed)) 
gdt_tss_entry_encoded : public gdt_entry_encoded
{
    uint32_t BaseHigher32;
    uint32_t Reserved;
};

struct gdt_entries
{
    gdt_entry_encoded Null;
    gdt_entry_encoded Code32Bit;
    gdt_entry_encoded Data32Bit;
    gdt_entry_encoded KernelCode64Bit;
    gdt_entry_encoded KernelData64Bit;
    gdt_entry_encoded UserCode64Bit;
    gdt_entry_encoded UserData64Bit;
    gdt_tss_entry_encoded Tss;
};

struct __attribute__((packed))
tss
{
    uint32_t Reserved0;
    uint64_t Rsp[3];
    uint64_t Reserved1;
    uint64_t Ist[7];
    uint64_t Reserved2;
    uint16_t Reserved3;
    uint16_t Iopb;
};

class CGdt
{
public:
    void InitDefaults();
    void AddEntry(int index, gdt_entry ent);
    void Encode();
    /* TODO: add a function that returns the first GDT entry
    which matches a specific criteria (can run code, is conformant etc...) */

};

void InitGdt();
