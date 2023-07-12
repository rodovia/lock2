#include "load.h"
#include "alloc/physical.h"
#include "dllogic/api/dhelp.h"
#include "dllogic/ustar.h"
#include "pe.h"
#include "requests.h"
#include "terminal.h"
#include "arch/i386/debug/stackframe.h"
#include "api/dhelp_internal.h"

void driver::LoadDrivers()
{
    char* dll;
    limine_module_response* re = rqs::GetModules();
    limine_file* tar = re->modules[0];

    driver::LoadUstarFile(reinterpret_cast<unsigned char*>(tar->address), "atadrv.dll", dll);
    int flags = pe::kPeLoadFlagsCurrentAddressSpace | pe::kPeLoadFlagsPageInsideKernelMode;
    auto por = pe::CPortableExecutable((pe::dos_header*)dll, flags);

    IDHelpDriverManager* sd = driver::CreateManagerFor(por);
}

