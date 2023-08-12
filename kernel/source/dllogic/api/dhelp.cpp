#include "acpi/tables.h"
#include "alloc/physical.h"
#include "dhelp_internal.h"

#include "arch/i386/apic.h"
#include "dllogic/api/dhelp.h"
#include "dllogic/load.h"
#include "dllogic/pe.h"
#include "pci/pci.h"
#include "terminal.h"

class CDriverManager : public IDHelpDriverManager
{
public:
    CDriverManager(pe::CPortableExecutable exec);
    ~CDriverManager() override {}

    virtual int GetPci(IDHelpPci** pci) override;
    virtual int GetTerminal(IDHelpTerminal** term) override;
    virtual int GetInterruptController(IDHelpInterruptController** ic) override;
    virtual int GetAllocator(IDHelpMemoryAllocator** al) override;
    virtual void FreeInterface(void* ref) override;

    virtual void SetRole(driver_role role) noexcept override;
    virtual void SetNotify(driver_notify routine) noexcept override;

private:
    pe::CPortableExecutable m_Executable;
    driver_role m_Role;
    driver_notify m_DriverNotify;
};

class CMemoryAllocator : public IDHelpMemoryAllocator
{
public:
    static CMemoryAllocator& GetInstance()
    {
        static CMemoryAllocator c;
        return c;
    }

    void* Allocate(size_t bytes) override;
    void Free(void* block) override;
    void* AlignedAlloc(size_t bytes, uint16_t alignment) override;
    void AlignedFree(void* block, size_t bytes) override;

private:
    CMemoryAllocator() = default;
};

CDriverManager::CDriverManager(pe::CPortableExecutable exec)
    : m_Executable(exec),
      m_Role(kDHelpDriverRoleUninit),
      m_DriverNotify(nullptr)
{
    auto notf = reinterpret_cast<driver::driver_init>(m_Executable.GetSymbolAddress("Driver_Init"));
    if (notf == nullptr)
    {
        Warn("Driver has no entry point, unloading!\n");
        return;
    }

    notf(this);
    if (m_Role == kDHelpDriverRoleUninit)
    {
        Warn("Driver did not advertise its role, unloading!\n");
        return;
    }

    if (m_DriverNotify == nullptr)
    {
        Warn("Driver did not set notify routine, unloading!\n");
        return;
    }
}

void CDriverManager::SetNotify(driver_notify routine) noexcept
{
    m_DriverNotify = routine;
}

void CDriverManager::SetRole(driver_role role) noexcept
{
    m_Role = role;
}

int CDriverManager::GetTerminal(IDHelpTerminal** term)
{  
    auto tem = &CTerminal::GetInstance();
    if (term == nullptr)
    {
        return kDHelpResultInvalidArgument;
    }

    (*term) = tem;
    return kDHelpResultOk;
}

int CDriverManager::GetPci(IDHelpPci **pci)
{
    auto pc = &pci::CPci::GetInstance();
    if (pci == nullptr)
    {
        return kDHelpResultInvalidArgument;
    }

    (*pci) = pc;
    return kDHelpResultOk;
}

int CDriverManager::GetAllocator(IDHelpMemoryAllocator **al)
{
    auto mal = &CMemoryAllocator::GetInstance();
    if (al == nullptr)
    {
        return kDHelpResultInvalidArgument;
    }

    (*al) = mal;
    return kDHelpResultOk;
}

void CDriverManager::FreeInterface(void* ref)
{
    delete (uint8_t*)ref;
}

int CDriverManager::GetInterruptController(IDHelpInterruptController **ic)
{
    (*ic) = &acpi::CApicController::GetInstance();
    return kDHelpResultOk;
}

IDHelpDriverManager* 
driver::CreateManagerFor(pe::CPortableExecutable exec)
{
    return new CDriverManager(exec);
}

/* CMemoryAllocator */
void* CMemoryAllocator::AlignedAlloc(size_t bytes, uint16_t alignment)
{
    return pm::AlignedAlloc(bytes, alignment);
}

void CMemoryAllocator::AlignedFree(void *block, size_t bytes)
{
    pm::AlignedFree(bytes, block);
}

void* CMemoryAllocator::Allocate(size_t bytes)
{
    return pm::Alloc(bytes);
}

void CMemoryAllocator::Free(void *block)
{
    pm::Free(block);
}
