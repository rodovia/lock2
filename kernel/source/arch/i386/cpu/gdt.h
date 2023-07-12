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

class CGdt
{
public:
    void InitDefaults();
    void AddEntry(int index, gdt_entry ent);
    void Encode();
    /* TODO: add a function that returns the first GDT entry
    which matches a specific criteria (can run code, is conformant etc...) */

private:
    gdt_entry_encoded m_Entries[10];

};

void InitGdt();
