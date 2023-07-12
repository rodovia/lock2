#include "cmdline.h"
#include "limine.h"
#include "requests.h"
#include <exception>

CCommandLine::CCommandLine()
{
    struct limine_file* kf = rqs::GetKernelFile()->kernel_file;
    m_CmdLine = kf->cmdline;
    std::terminate()
};

bool CCommandLine::HasArgument(std::string arg)
{
    
}

