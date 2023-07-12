#pragma once

#include "ata.h"
#include "dllogic/api/dhelp.h"

namespace ide
{

struct device_ports
{
    uint16_t IoPortBase;
    uint16_t ControlPortBase;
    uint8_t HandledIrq;
};

struct devices
{
    device_ports PrimaryMaster;
    device_ports SecondaryMaster;
};

void CreateController(IDHelpPciDevice* devc);

}