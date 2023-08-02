/* Code largely based off leap0x7b faruos' paging implementation,
    <https://github.com/leap0x7b/faruos/> */

#include "paging.h"
#include "alloc/physical.h"
#include "limine.h"
#include "requests.h"
#include <vector>

#define KB 1024
#define MB 1024 * 1024

std::vector<virtm::global_page> globalPages;

static uint64_t* GetNextLevel(uint64_t* level, size_t offset, int flags)
{
    if (!(level[offset] & PT_FLAG_PRESENT))
    {
        level[offset] = reinterpret_cast<uint64_t>(pm::AlignedAlloc(4096, 4096));
        memset((uint64_t*)level[offset], 0, 4096);
        level[offset] |= flags;
    }

    level[offset] |= flags;
    return reinterpret_cast<uint64_t*>(level[offset] & ~511);
}

void virtm::SetCr3(paddr_t pd)
{
    asm volatile ("mov %0, %%cr3" :: "r"(pd));
}

static uint64_t* ReadCr3()
{
    uint64_t ret;
    asm volatile ("mov %%cr3, %0" :"=r"(ret));
    return reinterpret_cast<uint64_t*>(ret);
}

uint64_t* virtm::CreatePml4()
{
    uint64_t* pml4 = (uint64_t*)pm::AlignedAlloc(4096, 4096);
    struct limine_memmap_response* mem = rqs::GetMemoryMap();
    struct limine_hhdm_response* hhdm = rqs::GetHhdm();
    struct limine_kernel_address_response* addr = rqs::GetAddresses();

    MapPages(pml4, (uint64_t)pm::GetGlobal(), (uint64_t)pm::GetGlobal(), PT_FLAG_WRITE);
    MapPages(pml4, (uint64_t)pml4, (uint64_t)pml4, PT_FLAG_WRITE);

    for (uint64_t i = 0; i < 100 * KB; i += 4 * KB)
    {
        MapPages(pml4, (uint64_t)pm::GetGlobal() + i, (uint64_t)pm::GetGlobal() + i, PT_FLAG_WRITE);
    }

    for (uint64_t i = 0; i < mem->entry_count; i++)
    {
        struct limine_memmap_entry* ent = mem->entries[i];
        if (ent->type == LIMINE_MEMMAP_USABLE)
        {
            continue;
        }

        if (ent->type == LIMINE_MEMMAP_ACPI_RECLAIMABLE
        || ent->type == LIMINE_MEMMAP_ACPI_NVS)
        {
            for (uint64_t c = 0; c < ent->length; c += 4 * KB)
            {
                MapPages(pml4, ent->base + c, ent->base + c, PT_FLAG_WRITE);
            }

            continue;
        }

        for (uint64_t c = 0; c < ent->length; c += 4 * KB)
        {
            MapPages(pml4, ent->base + c, hhdm->offset + ent->base + c, PT_FLAG_WRITE);
        }
    }

    for (uint64_t i = 0; i < 5 * MB; i += 4 * KB)
    {
        MapPages(pml4, addr->physical_base + i, addr->virtual_base + i, PT_FLAG_WRITE);
    }

    if (globalPages.data() != nullptr)
    {
        for (auto& p : globalPages)
        {
            MapPages(pml4, p.PhysicalAddr, p.VirtualAddr, p.Flags);
        }
    }

    return pml4;
}

void virtm::MapPages(uint64_t *pml4, paddr_t paddr, 
                    vaddr_t vaddr, int flags)
{
    if (pml4 == nullptr)
    {
        pml4 = ReadCr3();
    }

    flags |= PT_FLAG_PRESENT;

    size_t index4 = (size_t)(vaddr & ((vaddr_t)0x1FF << 39)) >> 39;
    size_t index3 = (size_t)(vaddr & ((vaddr_t)0x1FF << 30)) >> 30;
    size_t index2 = (size_t)(vaddr & ((vaddr_t)0x1FF << 21)) >> 21;
    size_t index1 = (size_t)(vaddr & ((vaddr_t)0x1FF << 12)) >> 12;

    uint64_t* pdpe = GetNextLevel(pml4, index4, flags);
    uint64_t* pde = GetNextLevel(pdpe, index3, flags);
    uint64_t* pte = GetNextLevel(pde, index2, flags);

    pte[index1] = paddr | flags;
    asm volatile ("invlpg (%0)" :: "r"(vaddr) : "memory");
}

void virtm::AddGlobalPage(global_page page)
{
    MapPages(nullptr, page.PhysicalAddr, page.VirtualAddr, page.Flags);
    globalPages.push_back(page);
}

void virtm::UnmapPages(uint64_t *pml4, vaddr_t vaddr)
{
    if (pml4 == nullptr)
    {
        pml4 = ReadCr3();
    }

    size_t index4 = (size_t)(vaddr & ((vaddr_t)0x1FF << 39)) >> 39;
    size_t index3 = (size_t)(vaddr & ((vaddr_t)0x1FF << 30)) >> 30;
    size_t index2 = (size_t)(vaddr & ((vaddr_t)0x1FF << 21)) >> 21;
    size_t index1 = (size_t)(vaddr & ((vaddr_t)0x1FF << 12)) >> 12;

    uint64_t* pdpe = reinterpret_cast<uint64_t*>(pml4[index4] & ~511);
    uint64_t* pde = reinterpret_cast<uint64_t*>(pdpe[index3] & ~511);
    uint64_t* pte = reinterpret_cast<uint64_t*>(pde[index2] & ~511);
    pte[index1] = 0;
}

void* virtm::WalkPageTable(uint64_t *pml4, vaddr_t address)
{
    if (pml4 == nullptr)
    {
        pml4 = ReadCr3();
    }

    size_t index4 = (size_t)(address & ((vaddr_t)0x1FF << 39)) >> 39;
    size_t index3 = (size_t)(address & ((vaddr_t)0x1FF << 30)) >> 30;
    size_t index2 = (size_t)(address & ((vaddr_t)0x1FF << 21)) >> 21;
    size_t index1 = (size_t)(address & ((vaddr_t)0x1FF << 12)) >> 12;

    uint64_t* pdpe = reinterpret_cast<uint64_t*>(pml4[index4] & ~511);
    uint64_t* pde = reinterpret_cast<uint64_t*>(pdpe[index3] & ~511);
    uint64_t* pte = reinterpret_cast<uint64_t*>(pde[index2] & ~511);
    return reinterpret_cast<uint64_t*>(pte[index1] & ~511);
}

void virtm::RemapPage(uint64_t *pml4, vaddr_t vaddr, int flags)
{
    if (pml4 == nullptr)
    {
        pml4 = ReadCr3();
    }

    void* phys = virtm::WalkPageTable(pml4, vaddr);
    UnmapPages(pml4, vaddr);
    MapPages(pml4, (paddr_t)phys, vaddr, flags);
}
