#pragma once

#include <stdint.h>

/* Type definitions and structs taken from Lockdown source code...
   ...which is in turn taken from the Android Runtime source code...
   ...which is in turn taken from the LLVM source code. */

namespace elf
{

typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_TLS 7

#define PT_LOOS 0x60000000
#define PT_HIOS 0x6FFFFFFF
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7FFFFFFF

#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN  3
#define ET_CORE 4
#define ET_LOPROC 0xff00 
#define ET_HIPROC 0xffff 

#define EI_MAG0 0          /* File identification index. */
#define EI_MAG1 1          /* File identification index. */
#define EI_MAG2 2          /* File identification index. */
#define EI_MAG3 3          /* File identification index. */
#define EI_CLASS 4          /* File class. */
#define EI_DATA  5          /* Data encoding. */
#define EI_VERSION 6          /* File version. */
#define EI_OSABI   7          /* OS/ABI identification. */
#define EI_ABIVERSION 8          /* ABI version. */
#define EI_PAD        9          /* Start of padding bytes. */
#define EI_NIDENT     16

static constexpr char elfMagic[] = { 0x7F, 'E', 'L', 'F', '\0' };

typedef struct elf_header64
{
    unsigned char Magic[EI_NIDENT];
    Elf64_Half Type;
    Elf64_Half Machine;
    Elf64_Word Version;
    Elf64_Addr EntryAddress;
    Elf64_Off ProgramHeaderFileOffset;
    Elf64_Off SectionHeaderFileOffset;
    Elf64_Word Flags;
    Elf64_Half HeaderSize;
    Elf64_Half ProgramHeaderEntrySize;
    Elf64_Half ProgramHeaderCount;
    Elf64_Half SectionHeaderEntrySize;
    Elf64_Half SectionHeaderCount;
    Elf64_Half StringSectionIndex;

    constexpr bool ValidateSignature() const
    {
        /* TODO: remove or place these builtins 
                behind a macro or another function */
        return !__builtin_memcmp(Magic, elfMagic, __builtin_strlen(elfMagic));
    }
} elf_header64_t;

typedef struct elf_phdr64
{
    Elf64_Word   Type; 
    Elf64_Word   Flags;
    Elf64_Off    FileOffset;
    Elf64_Addr   VirtualAddress;
    Elf64_Addr   PhysicalAddress;
    Elf64_Xword  FileSize;
    Elf64_Xword  MemorySize;
    Elf64_Xword  Aligned;
} elf_phdr64_t;

typedef struct elf_shdr64 {
    Elf64_Word Name;
    Elf64_Word Type;
    Elf64_Word Flags;
    Elf64_Addr MemoryAddress;
    Elf64_Off FileOffset;
    Elf64_Word FactualSize;
    Elf64_Word Link;
    Elf64_Word Info;
    Elf64_Word AddressAlignment;
    Elf64_Word EntrySize;
} elf_shdr64_t;

}
