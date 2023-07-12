#pragma once 

#include <stdint.h>

namespace dbg
{

using Elf64_Addr = uint64_t;
using Elf64_Half = uint16_t;
using Elf64_SHalf = int16_t;
using Elf64_Off = uint64_t;
using Elf64_Sword = int32_t;
using Elf64_Word = uint32_t;
using Elf64_Xword = uint64_t;
using Elf64_Sxword = int64_t;

struct elf_header
{
    unsigned char e_ident[15];
    Elf64_Half Type;
    Elf64_Half machine;
    Elf64_Word Version;
    Elf64_Addr Entry;
    Elf64_Off ProgramHeaderOffset;
    Elf64_Off SectionHeaderOffset;
    Elf64_Word Flags;
    Elf64_Half HeaderSize;
    Elf64_Half ProgramHeaderEntrySize;
    Elf64_Half ProgramHeaderCount;
    Elf64_Half SectionHeaderEntrySize;
    Elf64_Half SectionHeaderCount;
    Elf64_Half SectionHeaderStringIndex;
};

struct elf_sheader
{
    Elf64_Word Name;
    Elf64_Word Type;
    Elf64_Xword Flags;
    Elf64_Addr Addr;
    Elf64_Off Offset;
    Elf64_Xword Size;
    Elf64_Word Link;	
    Elf64_Word Info;
    Elf64_Xword AddressAlign;
    Elf64_Xword EntrySize;
};

struct elf_symtable
{
	uint32_t Name;
	unsigned char Info;
	unsigned char Other;
	uint16_t Shndx;
	uint64_t Value;
	uint64_t Size;
};

void GetKernelSymbolTable(void*& symtab, uint64_t& symtabSize, void*& strtab);

}
