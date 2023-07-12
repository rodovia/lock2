#pragma once

#include <dllogic/api/dhelp.h>
#include <stdint.h>

namespace dhelp 
{

/* Makes device *devc* trigger CPU interrupt *vector* on
    attention request, using MSI. If *vector* is 0, 
    the interrupt controller will fetch a new free one 
    and return it. */
uint8_t RequestForInterrupt(IDHelpPciDevice* devc, int vector = 0);

}
