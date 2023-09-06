#include "acpi/tables.h"
#include "alloc/physical.h"
#include "dhelp_internal.h"

#include "arch/i386/apic.h"
#include "arch/i386/timer/hpet.h"
#include "dllogic/api/dhelp.h"
#include "dllogic/load.h"
#include "dllogic/pe.h"
#include "pci/pci.h"
#include "terminal.h"

namespace 
{

class CThreadManager : public IDHelpThreadScheduler
{
public:
    static CThreadManager& GetInstance()
    {
        static CThreadManager m;
        return m;
    }

    void HaltExecution(int ms) override
    {
        acpi::PrepareHpetDelay(ms * 1000);
    }

private:
    CThreadManager() = default;
};

class CMemoryAllocator : public IDHelpMemoryAllocator
{
public:
    static CMemoryAllocator& GetInstance()
    {
        static CMemoryAllocator c;
        return c;
    }

    void* Allocate(size_t bytes) override
    {
        return pm::Alloc(bytes);
    }

    void Free(void* block) override
    {
        pm::Free(block);
    }

    void* AlignedAlloc(size_t bytes, uint16_t alignment) override
    {
        return pm::AlignedAlloc(bytes, alignment);
    }

    void AlignedFree(void* block, size_t bytes) override
    {
        pm::AlignedFree(bytes, block);
    }

private:
    CMemoryAllocator() = default;
};

class CDriverManager : public IDHelpDriverManager
{
public:
    CDriverManager(pe::CPortableExecutable exec)
        : m_Executable(exec),
          m_Role(kDHelpDriverRoleUninit),
          m_Interface(nullptr)
    {
        auto main = reinterpret_cast<driver::driver_init>(m_Executable.GetSymbolAddress("Driver_Init"));
        if (main == nullptr)
        {
            Warn("Driver has no entry point, unloading!\n");
            return;
        }

        main(this);
        if (m_Role == kDHelpDriverRoleUninit)
        {
            Warn("Driver did not advertise its role, unloading!\n");
            return;
        }
    }

    ~CDriverManager() override {}

    virtual int GetPci(IDHelpPci** pci) override
    {
        auto pc = &pci::CPci::GetInstance();
        if (pci == nullptr)
        {
            return kDHelpResultInvalidArgument;
        }

        (*pci) = pc;
        return kDHelpResultOk;
    }

    virtual int GetTerminal(IDHelpTerminal** term) override
    {  
        auto tem = &CTerminal::GetInstance();
        if (term == nullptr)
        {
            return kDHelpResultInvalidArgument;
        }

        (*term) = tem;
        return kDHelpResultOk;
    }

    virtual int GetInterruptController(IDHelpInterruptController** ic) override
    {
        (*ic) = &acpi::CApicController::GetInstance();
        return kDHelpResultOk;
    }

    virtual int GetAllocator(IDHelpMemoryAllocator** al) override
    {
        auto mal = &CMemoryAllocator::GetInstance();
        if (al == nullptr)
        {
            return kDHelpResultInvalidArgument;
        }

        (*al) = mal;
        return kDHelpResultOk;
    }

    virtual int GetThreadManager(IDHelpThreadScheduler** th) override
    {
        auto mal = &CThreadManager::GetInstance();
        if (mal == nullptr)
        {
            return kDHelpResultInvalidArgument;
        }

        (*th) = mal;
        return kDHelpResultOk;
    }

    virtual void FreeInterface(void* ref) override
    {
        delete (uint8_t*)ref;
    }

    virtual void SetRole(driver_role role) noexcept override
    {
        m_Role = role;
    }

    virtual void SetInterface(void *interf) override
    {
        m_Interface = interf;
    }

    void* GetInterface() const
    {
        return m_Interface;
    }

private:
    pe::CPortableExecutable m_Executable;
    driver_role m_Role;
    void* m_Interface;
};

}

IDHelpDriverManager* 
driver::CreateManagerFor(pe::CPortableExecutable exec)
{
    return new CDriverManager(exec);
}

/* CMemoryAllocator */

