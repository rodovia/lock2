#pragma once

#include "dllogic/api/dhelp.h"
#include <stdint.h>
#include <string_view>
#include <vector>

namespace driver 
{

using driver_init = void(*)(IDHelpDriverManager*);

class CDriverLifetime
{
public:
    static CDriverLifetime& GetInstance()
    {
        static CDriverLifetime cd;
        return cd;
    }

    void AddDriver(IDHelpDriverManager* mgr);
    static void* GetInterfaceForRole(driver_role role);

private:
    static std::vector<IDHelpDriverManager*> s_Drivers;
};

void LoadDrivers();

}
