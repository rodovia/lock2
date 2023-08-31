#include "pe.h"
#include "terminal.h"
#include "alloc/physical.h"
#include "arch/i386/paging/paging.h"
#include <string_view>

pe::CPortableExecutable::CPortableExecutable(pe::dos_header* header, int flags)
    : m_Dos(header)
{
    if (header == nullptr)
    {
        Warn("PE: header is nullptr !\n");
        return;
    }

    if (header->Signature != DOS_MAGIC)
    {
        Warn("PE: not a DOS executable\n");
        return;
    }

    auto raw = reinterpret_cast<uint8_t*>(header);
    uint8_t peoff = raw[0x3C];

    auto pe = reinterpret_cast<pe_header*>(&raw[peoff]);
    auto opt = reinterpret_cast<pe_optional_header*>(PaAdd(pe, sizeof(pe_header)));
    auto secs = reinterpret_cast<pe_section*>(PaAdd(opt, pe->SizeOfOptionalHeader));
    if (pe->Signature != PE_MAGIC)
    {
        Warn("PE: not a PE executable (0x%p against 0x%p)\n", pe->Signature, PE_MAGIC);
        return;
    }

    int pflags = (flags & kPeLoadFlagsCurrentAddressSpace) ? 0 : PT_FLAG_USER;
    uint64_t* pml4 = (flags & kPeLoadFlagsPageInsideKernelMode) ? nullptr : virtm::CreatePml4();
    for (uint16_t i = 0; i < pe->NumberOfSections; i++)
    {
        pe_section s = secs[i];
        if (s.Characteristics & kPeSectionWrite)
        {
            pflags |= PT_FLAG_WRITE;
        }

        char* mem = reinterpret_cast<char*>(
            pm::AlignedAlloc(s.VirtualSize, opt->SectionAlignment)
        );
        ZeroMemory(mem, s.SizeOfRawData);
        memcpy(mem, raw + s.PointerToRawData, s.SizeOfRawData);
        
        for (uint64_t i = 0; i < s.VirtualSize; i += 4096)
        {
            virtm::MapPages(pml4, (paddr_t)mem + i, opt->ImageBase + s.VirtualAddress + i, pflags);
            m_Pages.push_back({ (uint64_t)mem, opt->ImageBase + s.VirtualAddress + i, s.VirtualSize });            
        }
    }

    m_Header = pe;
    m_OptHeader = opt;
}

void* pe::CPortableExecutable::GetSymbolAddress(std::string_view name)
{
    if (name.empty() || !(m_Header->Characteristics & kPeCharsDll))
    {
        Warn("Name empty or not a DLL");
        return nullptr;
    }

    uint16_t ordinal = 0;
    pe_section* sec = this->GetSection(".edata");
    auto exprt = reinterpret_cast<pe_export_table*>(m_OptHeader->ImageBase + sec->VirtualAddress);
    auto namep = reinterpret_cast<uint32_t*>(m_OptHeader->ImageBase + exprt->NamePointerRva);
    auto ordt = reinterpret_cast<uint16_t*>(m_OptHeader->ImageBase + exprt->OrdinalTableRva);
    auto addrt = reinterpret_cast<pe_export_address_table*>(
                    m_OptHeader->ImageBase + exprt->ExportAddressTableRva);
                    
    for (uint32_t i = 0; i < exprt->NumberOfNamePointers; i++)
    {
        auto symn = reinterpret_cast<const char*>(m_OptHeader->ImageBase + namep[i]);
        if (!name.compare(symn))
        {
            ordinal = ordt[i];
            uint64_t addr = m_OptHeader->ImageBase + addrt[ordinal].ExportRva;
            return reinterpret_cast<void*>(addr);
        }
    }

    return nullptr;
}

pe::pe_section* 
pe::CPortableExecutable::GetSection(std::string_view section)
{
    pe_section* secs = reinterpret_cast<pe_section*>(
            PaAdd(m_Header, sizeof(pe_header) + m_Header->SizeOfOptionalHeader));
    for (uint32_t i = 0; i < m_Header->NumberOfSections; i++)
    {
        pe_section s = secs[i];
        if (section.compare(s.Name) == 0)
        {
            return &secs[i];
        }
    }

    return nullptr;
}

uint64_t pe::CPortableExecutable::ImageBase() const
{
    return m_OptHeader->ImageBase;
}
