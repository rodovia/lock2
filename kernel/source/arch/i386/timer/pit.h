#pragma once

#include "arch/i386/port.h"

class CPit
{
public:
    static CPit& GetInstance()
    {
        static CPit c;
        return c;
    }

    void SetReloadValue(uint16_t value);
    uint16_t GetCurrentCount();
private:
    CPit();
};
