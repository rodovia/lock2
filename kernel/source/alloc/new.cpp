#include "physical.h"

void* operator new(size_t size)
{
    void* ret = pm::Alloc(size);
    memset(ret, 0, size);
    return ret;
}

void* operator new[](size_t size)
{
    void* ret = pm::Alloc(size);
    return ret;
}

void operator delete(void* p)
{
    pm::Free(p);
}

void operator delete[](void* p)
{
    pm::Free(p);
}
