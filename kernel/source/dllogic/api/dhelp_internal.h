#pragma once

#include "dhelp.h"
#include "dllogic/pe.h"

namespace driver
{

IDHelpDriverManager* CreateManagerFor(pe::CPortableExecutable exec);
void* GetInterfaceFor(IDHelpDriverManager* driver);

}
