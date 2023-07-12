#include "physical.h"
#include <stddef.h>
#include <stdint.h>

#include <terminal.h>

struct mem_global
{
    struct mem_block* FreeList;
    void* Head;
    uint64_t AllocatedSize;
    uint64_t Length;
};

struct __attribute__((packed)) mem_block
{
    mem_block* Next;
    uint32_t BlockSize;
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

static void TraverseAndRemove(mem_block** head, mem_block* item)
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

    mem_block* tmp = *head;
    mem_block* oldtmp = nullptr;
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
static void* LinklStratg(size_t bytes) /* BUG: 2 free blocks may point to 
                                          themselves, creating an infinite loop */
{
    mem_block* tmp = reinterpret_cast<mem_block*>(global->FreeList);
    while (tmp->Next != nullptr)
    {
        if (tmp->BlockSize >= bytes)
        {
            break;
        }
        Warn("tmp=0x%p, tmp->Next=0x%p\n", tmp, tmp->Next);
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

void* pm::AlignedAlloc(size_t bytes, uint16_t aligned)
{
    if (bytes == 0 || aligned == 0)
    {
        return nullptr;
    }

    global->AllocatedSize += bytes;
    uint64_t head = reinterpret_cast<uint64_t>(global->Head);
    if (head % aligned == 0)
    {
        global->Head = PaAdd(head, bytes);
        return (void*)head;
    }

    head += aligned - head % aligned;
    global->Head = PaAdd(head, bytes);
    return (void*)head;
}

void pm::Free(void* block)
{
    if (block == nullptr)
    {
        Warn("Free: Cannot free null ptr\n");
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