#pragma once

#include "dllogic/api/dhelp.h"
#include <stdint.h>

namespace driver 
{

using driver_notify = uint64_t(*)(uint32_t, uint64_t, uint64_t, uint64_t);
using driver_init = void(*)(IDHelpDriverManager*);

void LoadDrivers();

}
