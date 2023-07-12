#pragma once

#include <stdint.h>

__attribute__((sysv_abi)) extern "C" uint64_t Driver_Notify(uint32_t message,
                            uint64_t lparam, uint64_t wparam,
                            uint64_t flags);

namespace atapi
{

void ReadSectored();

}
