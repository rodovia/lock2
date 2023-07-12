#include "elf.h"
#include "limine.h"
#include "requests.h"
#include "string.h"
#include "terminal.h"

static dbg::elf_sheader* 
FindSection(dbg::elf_header* h, const char* name)
{
    uint8_t* raw = reinterpret_cast<uint8_t*>(h);
    auto secs = reinterpret_cast<dbg::elf_sheader*>(raw + h->SectionHeaderOffset);
    auto shstr = raw + secs[h->SectionHeaderStringIndex].Offset;
    
    for (uint64_t i = 0; i < h->SectionHeaderCount; i++)
    {
        dbg::elf_sheader sec = secs[i];
        auto nm = reinterpret_cast<char*>(&shstr[sec.Name]);
        if (!strncmp(name, nm, strlen(nm)))
        {
            return &secs[i];
        }
    }
    return nullptr;
}

void dbg::GetKernelSymbolTable(void*& symtab, uint64_t& symtabSize, void*& strtab)
{
    struct limine_file* r = rqs::GetKernelFile()->kernel_file;
    uint8_t* raw = reinterpret_cast<uint8_t*>(r->address);
    elf_header* h = reinterpret_cast<elf_header*>(r->address);
    elf_sheader* sec = FindSection(h, ".symtab");

    symtab = raw + sec->Offset;
    symtabSize = sec->Size / sec->EntrySize;
    strtab = raw + FindSection(h, ".strtab")->Offset;
}
