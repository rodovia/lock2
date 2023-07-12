#pragma once

#include <string>

class CCommandLine
{
public:
    static CCommandLine& Instance()
    {
        static CCommandLine c;
        return c;
    }

    CCommandLine();
    static bool HasArgument(std::string arg);

private:
    std::string m_CmdLine;
};
