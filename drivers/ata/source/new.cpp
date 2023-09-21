#include "dllogic/api/dhelp.h"

extern IDHelpDriverManager* __DriverMgr;

void* operator new(size_t size)
{
    IDHelpMemoryAllocator* alc;
    __DriverMgr->GetAllocator(&alc);
    return alc->Allocate(size);
}

void* operator new[](size_t size)
{
    IDHelpMemoryAllocator* alc;
    __DriverMgr->GetAllocator(&alc);
    return alc->Allocate(size);
}

void operator delete(void* p)
{
    IDHelpMemoryAllocator* alc;
    __DriverMgr->GetAllocator(&alc);
    alc->Free(p);
}

void operator delete[](void* p)
{
    IDHelpMemoryAllocator* alc;
    __DriverMgr->GetAllocator(&alc);
    alc->Free(p);
}

extern "C" void __cxa_pure_virtual()
{
    
}
