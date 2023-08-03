#include "physical.h"
#include "arch/i386/debug/stackframe.h"
#include <stddef.h>
#include <stdint.h>

#include <terminal.h>

struct mem_global
{
    struct mem_block* FreeList;
    struct mem_aligned_block* FreeAlignedList;
    void* Head;
    uint64_t AllocatedSize;
    uint64_t Length;
};

struct __attribute__((packed)) mem_block
{
    mem_block* Next;
    uint32_t BlockSize;
};

struct __attribute__((packed)) mem_aligned_block
{
    mem_aligned_block* Next;
    uint16_t Alignment;
    uint32_t BlockSize;

    constexpr mem_aligned_block(uint32_t blockSize, 
                                uint16_t alignment, 
                                mem_aligned_block* next = nullptr)
        : Next(next),
          Alignment(alignment),
          BlockSize(blockSize)
    {}

    constexpr mem_aligned_block()
        : Next(nullptr),
          Alignment(0),
          BlockSize(0)
    {}
};

static mem_global* global;

static void TraverseAppend(mem_block* head, mem_block* item)
{
    if (head == nullptr)
    {
        return;
    }

    mem_block* tmp = head;
    while(tmp->Next != nullptr)
    {
        tmp = tmp->Next;
    }

    tmp->Next = item;
    item->Next = nullptr;
}

template<class _Block>
static void TraverseAndRemove(_Block** head, _Block* item)
{
    if (head == NULL)
    {
        return;
    }

    if (item == *head)
    {
        (*head) = (*head)->Next;
        return;
    }

    _Block* tmp = *head;
    _Block* oldtmp = nullptr;
    while(tmp != nullptr)
    {
        if (tmp == item)
        {
            oldtmp->Next = tmp->Next;
            break;
        }

        oldtmp = tmp;
        tmp = tmp->Next;
    }
}

/* Allocate using the bump strategy */
static void* BumpStrtg(size_t bytes)
{
    void* oldHead = global->Head;
    void* dataHead = PaAdd(global->Head, sizeof(mem_block));
    
    mem_block blo;
    blo.BlockSize = bytes;
    blo.Next = nullptr;

    global->Head = PaAdd(global->Head, bytes + sizeof(mem_block));
    global->AllocatedSize += bytes;
    memcpy(oldHead, &blo, sizeof(mem_block));
    return dataHead;
}

/* "Allocates" (finds a free block of size >= *bytes*) a block.
    Similar to how Lockdown does it. */
static void* LinklStratg(size_t bytes)
{
    mem_block* tmp = reinterpret_cast<mem_block*>(global->FreeList);
    while (tmp->Next != nullptr)
    {
        if (tmp->BlockSize >= bytes)
        {
            break;
        }

        tmp = tmp->Next;
    }

    TraverseAndRemove(&global->FreeList, tmp);
    return PaAdd(tmp, sizeof(mem_block));
}

mem_global* pm::Create(void* begin, size_t size)
{    
    memset(begin, 0, sizeof(mem_global));
    mem_global* gbl = reinterpret_cast<mem_global*>(begin);
    gbl->AllocatedSize = 0;
    gbl->Length = size;
    gbl->Head = PaAdd(begin, sizeof(mem_global));

    global = gbl;
    return gbl;
}

[[gnu::hot]]
void* pm::Alloc(size_t bytes)
{
    if (bytes == 0)
    {
        return nullptr;
    }

    global->AllocatedSize += bytes;    
    if (global->FreeList == NULL)
    {
        return BumpStrtg(bytes);
    }
    
    return LinklStratg(bytes);
}

void* pm::AlignedAlloc(size_t bytes, uint16_t alignment)
{
    if (bytes == 0 || alignment == 0)
    {
        return nullptr;
    }

    if (global->FreeAlignedList != nullptr)
    {
        mem_aligned_block* aligned = global->FreeAlignedList;
        while (aligned->Next != nullptr)
        {
            if (aligned->BlockSize >= bytes && aligned->Alignment == alignment)
            {
                TraverseAndRemove(&global->FreeAlignedList, aligned);
            }

            aligned = aligned->Next;
        }
    }

    mem_aligned_block al(bytes, alignment, nullptr);
    uint64_t head = reinterpret_cast<uint64_t>(global->Head);

    auto headptr = PaAdd(head, bytes);
    int actSize = bytes + sizeof(mem_aligned_block);
    global->AllocatedSize += bytes;
    if (head % alignment == 0)
    {
        memcpy(headptr, &al, sizeof(al));
        global->Head = PaAdd(head, actSize);
        return (void*)head;
    }

    head += alignment - head % alignment;
    global->Head = PaAdd(head, actSize);
    memcpy(headptr, &al, sizeof(al));
    return (void*)head;
}

void pm::Free(void* block)
{
    if (block == nullptr)
    {
        Warn("Free: Cannot free null ptr");
    }

    mem_block* tmp = global->FreeList;
    mem_block* blk = (mem_block*)(reinterpret_cast<uint8_t*>(block) - sizeof(mem_block));
    blk->Next = nullptr;
    global->AllocatedSize -= blk->BlockSize;
    if (tmp == nullptr)
    {
        global->FreeList = blk;
        return;
    }
    
    TraverseAppend(global->FreeList, blk);
}

void pm::AlignedFree(size_t size, void* bl)
{
    if (size == 0 || bl == nullptr)
    {
        return;
    }

    mem_aligned_block* block = reinterpret_cast<mem_aligned_block*>(PaAdd(bl, size));
    mem_aligned_block* tmp = global->FreeAlignedList;
    if (tmp == nullptr)
    {
        global->FreeAlignedList = block;
        return;
    }

    while (tmp->Next != nullptr)
    {
        tmp = tmp->Next;
    }
    tmp->Next = block;
}

bool pm::WasAlloqued(void* ptr)
{
    uint64_t begin = (uint64_t)PaAdd(global, sizeof(mem_global));
    uint64_t ptr64 = (uint64_t)ptr;
    return ptr64 > begin && (uint64_t)global->Head > ptr64;
}

void* pm::GetGlobal()
{
    return global;
}

void* pm::GetBegin()
{
    return PaAdd(global, sizeof(mem_global));
}

void* pm::GetEnd()
{
    return PaAdd(pm::GetBegin(), global->Length);
}

void* memmove(void* to, const void* from, size_t size)
{
    uint8_t* toc = reinterpret_cast<uint8_t*>(to);
    const uint8_t* fromc = reinterpret_cast<const uint8_t*>(from);

    if (to > from && toc - fromc < (long)size)
    {
        for (size_t i = size; i != 0; i--)
        {
            toc[i] = fromc[i];
        }
        return toc;
    }

    return memcpy(to, from, size);
}
