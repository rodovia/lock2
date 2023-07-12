#pragma once

#include <stdint.h>

#define PT_FLAG_PRESENT  ((uint64_t)1 << 0)
#define PT_FLAG_WRITE    ((uint64_t)1 << 1)
#define PT_FLAG_USER     ((uint64_t)1 << 2)
#define PT_FLAG_PCD      ((uint64_t)1 << 4)
#define PT_FLAG_LARGE    ((uint64_t)1 << 7)
#define PT_FLAG_GUARD    ((uint64_t)1 << 9)
#define PT_FLAG_NX       ((uint64_t)1 << 63)

using paddr_t = uint64_t;
using vaddr_t = uint64_t;

namespace virtm
{

struct global_page
{
    uint64_t VirtualAddr;
    uint64_t PhysicalAddr;
    int Flags;
};

uint64_t* CreatePml4();
void MapPages(uint64_t* pml4, paddr_t paddr, 
            vaddr_t vaddr, int flags);
void UnmapPages(uint64_t* pml4,
                vaddr_t vaddr);
void RemapPage(uint64_t* pml4, vaddr_t vaddr, int flags);
void* WalkPageTable(uint64_t* pml4, vaddr_t address);
void SetCr3(paddr_t pd);
void AddGlobalPage(global_page page);

}
