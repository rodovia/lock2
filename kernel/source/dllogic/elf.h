#pragma once

#include "elf_types.h"

namespace elf
{

struct exc_link_flags
{

};

class CExecutableLinkable
{
public:
    CExecutableLinkable(uint8_t* elf, int flags);
};

}
