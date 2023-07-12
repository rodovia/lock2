#include "acpi/tables.h"
#include "dhelp_internal.h"

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

    virtual int CreateMachineLanguageParser(IDHelpMachineLanguageRegistry** sx) override;
    virtual int GetPci(IDHelpPci** pci) override;
    virtual int GetTerminal(IDHelpTerminal** term) override;
    virtual void FreeInterface(void* ref) override;

    virtual void SetRole(driver_role role) noexcept override;
    virtual void SetNotify(driver_notify routine) noexcept override;

private:
    pe::CPortableExecutable m_Executable;
    driver_role m_Role;
    driver_notify m_DriverNotify;
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
    Warn("Driver_Init=0x%p\n", notf);

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

int CDriverManager::CreateMachineLanguageParser(IDHelpMachineLanguageRegistry** sx)
{
    if (sx == nullptr)
    {
        return kDHelpResultInvalidArgument;
    }

    auto aml = nullptr;
    return kDHelpResultOk;
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

void CDriverManager::FreeInterface(void* ref)
{
    delete (uint8_t*)ref;
}

IDHelpDriverManager* 
driver::CreateManagerFor(pe::CPortableExecutable exec)
{
    return new CDriverManager(exec);
}
