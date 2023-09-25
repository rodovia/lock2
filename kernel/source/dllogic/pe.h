#pragma once

#include "alloc/physical.h"
#include "arch/i386/paging/paging.h"
#include "terminal.h"
#include <stdint.h>
#include <vector>
#include <string>

#define DOS_MAGIC 0x5A4D /* MZ */
#define PE_MAGIC 0x00004550 /* PE\0\0 (two null bytes) */

namespace pe
{

struct allocated_page
{
    uint64_t PhysicalAddress;
    uint64_t VirtualAddress;
    size_t Size;
};

struct dos_header
{
    uint16_t Signature;
};

struct pe_header
{
    uint32_t Signature;
    uint16_t Machine;
    uint16_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
};

struct pe_optional_header
{
    uint16_t Magic;
    uint8_t MajorLinkerVersion;
    uint8_t MinorLinkerVersion;
    uint32_t SizeOfCode;
    uint32_t SizeOfInitializedData;
    uint32_t SizeOfUninitializedData;
    uint32_t AddressOfEntryPoint;
    uint32_t BaseOfCode;
    uint64_t ImageBase;
    uint32_t SectionAlignment;
    uint32_t FileAlignment;
    uint16_t MajorOperatingSystemVersion;
    uint16_t MinorOperatingSystemVersion;
    uint16_t MajorImageVersion;
    uint16_t MinorImageVersion;
    uint16_t MajorSubsystemVersion;
    uint16_t MinorSubsystemVersion;
    uint32_t Reserved;
    uint32_t SizeOfImage;
    uint32_t SizeOfHeaders;
    uint32_t CheckSum;
    uint16_t Subsystem;
    uint16_t DllCharacteristics;
    uint64_t SizeOfStackReserve;
    uint64_t SizeOfStackCommit;
    uint64_t SizeOfHeapReserve;
    uint64_t SizeOfHeapCommit;
    uint32_t Reserved2; /* Loader Flags */
    uint32_t NumberOfRvaAndSizes;
};

struct pe_section
{
    char Name[8];
    uint32_t VirtualSize;
    uint32_t VirtualAddress;
    uint32_t SizeOfRawData;
    uint32_t PointerToRawData;
    uint32_t PointerToRelocations;
    uint32_t PointerToLineNumbers;
    uint16_t NumberOfRelocations;
    uint16_t NumberOfLineNumbers;
    uint32_t Characteristics;
};

struct pe_export_table
{
    uint32_t ExportFlags;
    uint32_t TimeDateStamp;
    uint16_t MajorVersion;
    uint16_t MinorVersion;
    uint32_t NameRva;
    uint32_t OrdinalBase;
    uint32_t AddressTableEntries;
    uint32_t NumberOfNamePointers;
    uint32_t ExportAddressTableRva;
    uint32_t NamePointerRva;
    uint32_t OrdinalTableRva;
};

struct pe_export_address_table
{
    uint32_t ExportRva;
    uint32_t ForwarderRva;
};

struct pe_relocation_table : public pe_section
{
    uint32_t PageRva;
    uint32_t BlockSize;
};

enum pe_section_flags : unsigned int
{
    kPeSectionRead = 0x40000000,
    kPeSectionWrite = 0x80000000,
};

enum pe_image_characteristics : unsigned short
{
    kPeCharsExecutableImage = 0x0002,
    kPeCharsDll = 0x2000
};

enum pe_load_flags
{
    kPeLoadFlagsCurrentAddressSpace = 0x1, /* Do not create a new PML4 to this executable.
                                              Only valid if executable is a DLL */
    kPeLoadFlagsPageInsideKernelMode,      /* Do not insert the executable inside user pages */
};

class CPortableExecutable
{
public:
    CPortableExecutable(dos_header* dos, int flags);

    void* AddressSpace() const;
    void* EntryPoint() const;
    uint64_t ImageBase() const;
    void* GetSymbolAddress(std::string_view name);
    pe_section* GetSection(std::string_view section);
private:
    void FixRelocations();

    bool m_Loaded;
    void* m_AddressSpace;
    void* m_EntryPoint;
    dos_header* m_Dos;
    pe_header* m_Header;
    pe_optional_header* m_OptHeader;
    std::vector<allocated_page> m_Pages;
};

}
