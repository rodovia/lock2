#include "load.h"
#include "alloc/physical.h"
#include "dllogic/api/dhelp.h"
#include "dllogic/api/dhelp_driver.h"
#include "dllogic/ustar.h"
#include "pe.h"
#include "requests.h"
#include "api/dhelp_internal.h"
#include <algorithm>

void driver::LoadDrivers()
{
    char* dll;
    limine_module_response* re = rqs::GetModules();
    limine_file* tar = re->modules[0];

    driver::LoadUstarFile(reinterpret_cast<unsigned char*>(tar->address), "atadrv.dll", dll);
    int flags = pe::kPeLoadFlagsCurrentAddressSpace | pe::kPeLoadFlagsPageInsideKernelMode;
    auto por = pe::CPortableExecutable((pe::dos_header*)dll, flags);

    IDHelpDriverManager* dsk = driver::CreateManagerFor(por);
    auto& cdl = CDriverLifetime::GetInstance();
    cdl.AddDriver(dsk);
}

void* driver::CDriverLifetime::GetInterfaceForRole(driver_role role)
{
    auto iter = std::find_if(s_Drivers.begin(), s_Drivers.end(), 
                    [&](IDHelpDriverManager* man)
                    {
                        return man->GetRole() == role;
                    });
    return (iter == s_Drivers.end()) ? nullptr : driver::GetInterfaceFor(*iter);
}

void driver::CDriverLifetime::AddDriver(IDHelpDriverManager* mgr)
{
    s_Drivers.push_back(std::move(mgr));
}
